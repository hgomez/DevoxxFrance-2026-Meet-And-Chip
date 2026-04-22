#include <bluefruit.h>

#define ADV_TIMEOUT   1 // change adv every 1 seconds

typedef volatile uint32_t REG32;
#define pREG32 (REG32 *)
#define MAC_ADDRESS_LOW   (*(pREG32 (0x100000a4)))
#define MAX_DEVICES 64
#define MAX_TOPICS 32
#define TOPIC_SZ 8

/**
 * Define some functions here to make C++ compiler happy
 */
void adv_stop_callback(void);
void startAdv(void);
void startScan(void);
void scan_callback(ble_gap_evt_adv_report_t* report);

/**
 * Select the Topics you want to share as positive or negative
 * by removing the comment.
 */
 const char topics[][TOPIC_SZ] = {
  //"+JAVA",
  "-JAVA",
  //"+PYTHON",
  //"-PYTHON",
  //"+JSCRIP",
  //"-JSCRIP",
  //"+CPLUS",
  //"-CPLUS",
  //"+RUBY",
  //"-RUBY",
  //"+SWIFT",
  //"-SWIFT",
  //"+KOTLIN",
  //"-KOTLIN",
  //"+GO",
  //"-GO",
  //"+RUST",
  //"-RUST",
  //"+HTML",
  //"-HTML",
  //"+CSS",
  //"-CSS",
  //"+SQL",
  //"-SQL",
  //"+NODE",
  //"-NODE",
  //"+ANGULR",
  //"-ANGULR",
  //"+REACT",
  //"-REACT",
  //"+VUE",
  //"-VUE",
  //"+DART",
  //"-DART",
  //"+FLUTTR",
  //"-FLUTTR",
  //"+COBOL",
  //"-COBOL",
  //"+C",
  //"-C",
  "+PHP",
  //"-PHP",
  //"+PERL",
  "-PERL",
  //"+CSHARP",
  //"-CSHARP",
  //"+UNITY",
  //"-UNITY",
  "+ARDUIN",
  //"-ARDUIN",
 };

#define NUM_TOPICS (sizeof(topics)/TOPIC_SZ)

_Static_assert(NUM_TOPICS <= MAX_TOPICS,"Too many topics selected");
uint8_t topic_i; // topic index
char myName[5]; // my short name
uint32_t watchdog;

// RSSI is emission power
#define PEER_FAR_AWAY -126
#define MAX_IN_RANGE_PEER -119
#define NEAR_PEER -55

// How many milliseconds before removing a peer not seen
#define PEER_RETENTION_MS 10000

/**
 * The system will maintain the advertising from the others and matching with signal strength.
 * The objective is to apply the matching calculation when the device are really close.
 * Memory allocation is static, we will store only MAX_DEVICES devices.
 */
 typedef struct {
    boolean initialized;
    uint32_t lastUpdate;
    char peerId[5];
    int8_t lastRssi;
    uint8_t matchCount;
    uint8_t detractorCount;
    char topics[MAX_TOPICS][TOPIC_SZ];
 } peer_t;

 peer_t peers[MAX_DEVICES];


void setup() {
  // initialize digital pin D3-D8 and the built-in LED as an output.
  pinMode(D3,OUTPUT);
  pinMode(D4,OUTPUT);
  pinMode(D5,OUTPUT);
  pinMode(D6,OUTPUT);
  pinMode(D7,OUTPUT);
  pinMode(D8,OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // Init serial communication for debugging
  Serial.begin(115200);
  long start = millis();
  while ( !Serial && (millis() - start < 2000)) delay(10);

  // start bluetooth
  Bluefruit.begin();
  Bluefruit.setTxPower(8);

  // Set name, setName just associate the buffer with the right size
  // the buffer update is not appreciated by the lib so we will just
  // update the content later. The size will be fixed to 15 chars
  topic_i = 0;
  watchdog = millis();
  sprintf(myName,"%04X",(MAC_ADDRESS_LOW) & 0xFFFF);
  startAdv();
  startScan();
}

// ===========================================================
// Advertizing
// ===========================================================

/**
 * Start Advertising
 */
void startAdv(void)
{
  static char ble_name[15];

  // Setup device name
  sprintf(ble_name,"M&G%s%-7.7s",myName, topics[topic_i]);
  Serial.printf("Send message %s\n",ble_name);

  // Clean
  Bluefruit.Advertising.stop();
  Bluefruit.Advertising.clearData();
  Bluefruit.ScanResponse.clearData();
  Bluefruit.setName(ble_name);

  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.setType(BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED);
  Bluefruit.Advertising.addTxPower();
  //Bluefruit.Advertising.addName();
  Bluefruit.ScanResponse.addName();

  Bluefruit.Advertising.setStopCallback(adv_stop_callback);
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(96, 244);    // in units of 0.625 ms (every 60ms, not spam)
  Bluefruit.Advertising.setFastTimeout(ADV_TIMEOUT);      // number of seconds in fast mode
  Bluefruit.Advertising.start(ADV_TIMEOUT);               // Stop advertising entirely after ADV_TIMEOUT seconds
}

/**
 * Callback invoked when advertising is stopped by timeout
 */
void adv_stop_callback(void)
{
  // got to next topic
 // Bluefruit.Advertising.stop();
  watchdog = millis();
  topic_i = (topic_i + 1) % NUM_TOPICS;
  startAdv();
}

// ===========================================================
// Scanning
// ===========================================================

/**
 * Start scan
 */
void startScan(void) {
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.filterRssi(-80);
  Bluefruit.Scanner.setInterval(160, 80);       // in units of 0.625 ms 50ms every 100ms
  Bluefruit.Scanner.useActiveScan(true);        // Request scan response data
  Bluefruit.Scanner.start(0);                  // 0 = Don't stop scanning after n seconds
}

/***
 * Remove Peers not seen for a period of time
 */
void cleanupOldPeers(uint32_t expiration) {
  peer_t *peers_pt = &peers[0];
  uint32_t now = millis();

  for (int i = 0; i < MAX_DEVICES; i++) {

    // Initialized peer ?
    if (peers_pt->initialized) {

      // LastUpdate is too old ?
      if ((now - peers_pt->lastUpdate) > expiration) {

        Serial.printf("cleanupOldPeers Id %s lastUpdate %d\n", peers_pt->peerId, peers_pt->lastUpdate);

        // peer is unitialized
        peers_pt->initialized = false;

        // Clean structure
        peers_pt->peerId[0] = 0;
        peers_pt->lastUpdate = 0;
        peers_pt->lastRssi = 0;
        peers_pt->detractorCount = 0;
        peers_pt->matchCount = 0;

        // Reset all topics chars
        for (int t = 0; t < MAX_TOPICS; t++)
          peers_pt->topics[t][0] = 0;
 
       }
    }
    
    peers_pt++;
  }
}


/**
 * Update the topic list for a given peer
 */
void updateTopic(peer_t *peers_pt,char * topic) {
  int i = 0;

  if (peers_pt == nullptr)
    return;

  while ( i < MAX_TOPICS && strlen(peers_pt->topics[i])>0 && strcmp(topic,peers_pt->topics[i]) != 0 ) 
    i++;

  if ( i < MAX_TOPICS && strlen(peers_pt->topics[i])==0 ) {
    // Not found, add and update computation
    bcopy(topic,peers_pt->topics[i],TOPIC_SZ);
    // search if we have it (+/-)
    for ( int k = 0 ; k < NUM_TOPICS ; k++ ) {
      if ( strncmp(&topics[k][1],&topic[1],strlen(topics[k])-1) == 0 ) {

        Serial.printf("New topic %s for Peer Id %.5s\n", topic, peers_pt->peerId);

        // match
        if ( (topics[k][0] == '+' && topic[0] == '+') || (topics[k][0] == '-' && topic[0] == '-') ) {
          peers_pt->matchCount++;      // same opinion
        } else {
          peers_pt->detractorCount++;  // diverging opinion
        }
      }
    }
  } // else, we already know it, nothing more to be done
}

/**
 * Dump Peers
 */
void dump_peers() {
  peer_t *peers_pt = &peers[0];

  for (int i = 0; i < MAX_DEVICES; i++) {

    if (peers_pt->initialized)
      Serial.printf("Peer Id %.5s lastUpdate %d lastRssi %d detractorCount %d matchCount %d\n", peers_pt->peerId, peers_pt->lastUpdate, peers_pt->lastRssi, peers_pt->detractorCount, peers_pt->matchCount);
    
    peers_pt++;
  }
}


/**
  * Scan callback
  */
void scan_callback(ble_gap_evt_adv_report_t* report)
{
  // Serial.printf("scan_callback\n");

  // Get the name & filter one M&G
  const char name[32] = {0};
  peer_t *peers_pt;
  peer_t *freepeers_pt = nullptr;
  bool    addpeer = true;
 
  if (Bluefruit.Scanner.parseReportByType(
    report,
    BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
    (uint8_t *)name, sizeof(name)
  )) {
    if ( strncmp(name,"M&G", 3) == 0 && strlen(name) >= 14) {
      
      char adr[5]; 
      char topic[8];
      
      bcopy(&name[3],adr,4);adr[4]=0;
      bcopy(&name[7],topic,TOPIC_SZ-1);
      topic[TOPIC_SZ-1]=0;

      // We found a name, not my name
      if ( strcmp(adr,myName) != 0 ) {

        peers_pt = &peers[0];
        for (int i =0; i <= MAX_DEVICES; i++, peers_pt++) {

          // Uninitialized slot and looking for a slot ?
          if ((! peers_pt->initialized) && (freepeers_pt == nullptr))
            freepeers_pt = peers_pt;

          // Peer exist and same name
          if ( peers_pt->initialized && strcmp(peers_pt->peerId,adr) == 0 ) {

            // More than 0.5s since last time seen ?
            if ( (millis()-peers_pt->lastUpdate) > 500 ) {
              // Update last Updat time and RSSI

              peers_pt->lastUpdate = millis();
              peers_pt->lastRssi = report->rssi;
              Serial.printf("Peer Id %.5s with Rssi %d\n", peers_pt->peerId, peers_pt->lastRssi);

              // Update Topic, Match/Detractor
              updateTopic(peers_pt,topic);
            }

            // dump_peers();
            addpeer = false; 
            break;
          }
        }

        if ((addpeer == true) && (freepeers_pt != nullptr)) {
          // new device with an available slot
          freepeers_pt->initialized = true;
          strcpy(freepeers_pt->peerId, adr);
          freepeers_pt->lastUpdate = millis();
          freepeers_pt->lastRssi = report->rssi;
          freepeers_pt->matchCount = 0;
          freepeers_pt->detractorCount = 0;

          Serial.printf("New peer Id %.5s with Rssi %d\n", peers_pt->peerId, peers_pt->lastRssi);

          // clean topics space
          bzero(freepeers_pt->topics,MAX_TOPICS*TOPIC_SZ);
          // Update Topic, Match/Detractor
          updateTopic(peers_pt,topic);

          // dump_peers();
        }
      }
     // Serial.printf("Rx %s %s with rssi %d\r\n",adr,top,report->rssi);
    } //else if ( name[0] == 'M' ) Serial.printf("from (%s)\r\n",name);
  }

  // Cleanup Old Peers
  cleanupOldPeers(PEER_RETENTION_MS);

  Bluefruit.Scanner.resume();
}

void loop() {

  static int cnt = 0;

  peer_t * peer_pt;

  // Manage bootloader switch for over_the_cable fw update (type ! on serial console)
  while ( Serial && Serial.available() ) if (Serial.read()=='!') enterSerialDfu();

  // Make sure a device shutting down close to user will not be kept as close
  // clear rssi for old peers ( not updated since 10 seconds )
  for ( int i = 0; i < MAX_DEVICES; i++ ) {
    if ( peers[i].initialized && (millis() - peers[i].lastUpdate) > 10000 ) {
      peers[i].lastRssi = PEER_FAR_AWAY;
    }
  }

  // Detect close devices with -55dBm or more
  int closeDevices = 0;
  int activeDevices = 0;

  int matchers = 0;
  int detractors = 0;
  
  int bestRssi = MAX_IN_RANGE_PEER;
  int bestId = MAX_DEVICES + 1;

  peer_pt = &peers[0];

  for ( int i = 0; i < MAX_DEVICES; i++, peer_pt++ ) {

    // Found an initialized peer?
    if (peer_pt->initialized) { 
      
      // it's an active Peer if not too far
      if (peer_pt->lastRssi >= PEER_FAR_AWAY)
        activeDevices++;
    
      if (peer_pt->matchCount > peer_pt->detractorCount)
        matchers++;
      else if (peer_pt->matchCount < peer_pt->detractorCount)
        detractors++;

      // Close Peer if rssi is low enough
      if (peer_pt->lastRssi >= NEAR_PEER) {
        closeDevices++;
        
        // New best rssi (closest peer) ?
        if (bestRssi < peer_pt->lastRssi) { 
          bestId = i; 
          bestRssi = peer_pt->lastRssi; 
        }
      }
    }
  }

  //         LEDS 
  // -- RED --   -- GREEN -- 
  // D8 D7 D6     D5 D4 D3
  // Light searching when no close devices
  analogWrite(D3,0);
  analogWrite(D4,0);
  analogWrite(D5,0);
  analogWrite(D6,0);
  analogWrite(D7,0);
  analogWrite(D8,0);

  // If we don't find any active devices 
  if (activeDevices == 0) {

    Serial.printf("No active devices\n");

    switch(cnt) {
      case 0 : analogWrite(D7,20);analogWrite(D8,30);analogWrite(D3,40);break;
      case 1 : analogWrite(D8,10);analogWrite(D3,40);analogWrite(D4,40);break;
      case 2 : analogWrite(D3,20);analogWrite(D4,30);analogWrite(D5,40);break;
      case 3 : analogWrite(D4,10);analogWrite(D5,40);analogWrite(D6,40);break;
      case 4 : analogWrite(D5,20);analogWrite(D6,30);analogWrite(D7,40);break;
      case 5 : analogWrite(D6,10);analogWrite(D7,40);analogWrite(D8,40);break;
      default: break;
    }

    cnt = ( cnt + 1 ) % 6;
    delay(300);
  } 
  else {

    int power_led = 40;

    // If we don't find any active devices, show affinity around (all close peers) 
    if (closeDevices == 0) {

      Serial.printf("No close devices, compute affinity around\n");

      int matchpct = (100 * matchers) / activeDevices;
      int detractorpct = (100 * detractors) / activeDevices;
      
      Serial.printf("matchpct: %d detractorpct: %d\n", matchpct, detractorpct);

      // Close Devices - cycle is 10/100

      for (int l = 0; l < 2; l++) {
        if (matchpct >= 20) analogWrite(D3, (power_led * 33) / (matchpct % 33));
        if (matchpct >= 35) analogWrite(D4, (power_led * 66) / (matchpct % 66));
        if (matchpct >= 68) analogWrite(D5, (power_led * 99) / (matchpct % 99));
        if (detractorpct >= 20) analogWrite(D6, (power_led * 33) / (detractorpct % 33));
        if (detractorpct >= 35) analogWrite(D7, (power_led * 66) / (detractorpct % 66));
        if (detractorpct >= 68) analogWrite(D8, (power_led * 99) / (detractorpct % 99));
        delay(100);
        analogWrite(D3,0);
        analogWrite(D4,0);
        analogWrite(D5,0);
        analogWrite(D6,0);
        analogWrite(D7,0);
        analogWrite(D8,0);
        delay(1000);
      } 
    }
    else {

      Serial.printf("closest peer %d\n",bestId);

      // Affinity for closest peer - cycle is 50/50
      if ( peers[bestId].matchCount >= 1 ) analogWrite(D3,power_led);
      if ( peers[bestId].matchCount >= 2 ) analogWrite(D4,power_led);
      if ( peers[bestId].matchCount >= 3 ) analogWrite(D5,power_led);
      if ( peers[bestId].detractorCount >= 1 ) analogWrite(D6,power_led);
      if ( peers[bestId].detractorCount >= 2 ) analogWrite(D7,power_led);
      if ( peers[bestId].detractorCount >= 3 ) analogWrite(D8,power_led);
      delay(500);
      analogWrite(D3,0);
      analogWrite(D4,0);
      analogWrite(D5,0);
      analogWrite(D6,0);
      analogWrite(D7,0);
      analogWrite(D8,0);
      delay(500);
    }

  // Sometime the advertizing is killed... resume it
    if ( (millis() - watchdog) > 5000 )
      adv_stop_callback();
  }
}