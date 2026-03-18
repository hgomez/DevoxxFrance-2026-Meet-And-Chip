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
  "+JAVA",
  //"-JAVA",
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
  //"+PHP",
  "-PHP",
  "+PERL",
  //"-PERL",
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
  Serial.printf("Name : %s (%d)\r\n",ble_name,strlen(ble_name));

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

/**
 * Update the topic list for a given peer
 */
void updateTopic(int peerIdx,char * topic) {
  int i = 0;
  while ( i < MAX_TOPICS && strlen(peers[peerIdx].topics[i])>0 && strcmp(topic,peers[peerIdx].topics[i]) != 0 ) i++;
  if ( i < MAX_TOPICS && strlen(peers[peerIdx].topics[i])==0 ) {
    // Not found, add and update computation
    bcopy(topic,peers[peerIdx].topics[i],TOPIC_SZ);
    // search if we have it (+/-)
    for ( int k = 0 ; k < NUM_TOPICS ; k++ ) {
      if ( strncmp(&topics[k][1],&topic[1],strlen(topics[k])-1) == 0 ) {
        // match
        if ( (topics[k][0] == '+' && topic[0] == '+') || (topics[k][0] == '-' && topic[0] == '-') ) {
          peers[peerIdx].matchCount++;      // same opinion
        } else {
          peers[peerIdx].detractorCount++;  // diverging opinion
        }
      }
    }
  } // else, we already know it, nothing more to be done
}

/**
  * Scan callback
  */
void scan_callback(ble_gap_evt_adv_report_t* report)
{
  // Get the name & filter one M&G
  const char name[32] = {0};
  if (Bluefruit.Scanner.parseReportByType(
    report,
    BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
    (uint8_t *)name, sizeof(name)
  )) {
    if ( strncmp(name,"M&G", 3) == 0 && strlen(name) >= 14) {
      char adr[5]; char top[8];
      bcopy(&name[3],adr,4);adr[4]=0;
      bcopy(&name[7],top,TOPIC_SZ-1);top[TOPIC_SZ-1]=0;
      if ( strcmp(adr,myName) != 0 ) {
        int i = 0, k = MAX_DEVICES;
        while (i < MAX_DEVICES) {
          if ( peers[i].initialized && strcmp(peers[i].peerId,adr) == 0 ) {
            if ( (millis()-peers[i].lastUpdate) > 500 ) {
              // check for update
              peers[i].lastUpdate = millis();
              peers[i].lastRssi = report->rssi;
              updateTopic(i,top);
            }
            break;
          }
          if ( !peers[i].initialized ) {k = i; i = MAX_DEVICES;} else i++;
        }
        if ( i == MAX_DEVICES && k < MAX_DEVICES ) {
          // new device with an available slot
          peers[k].initialized = true;
          strcpy(peers[k].peerId, adr);
          peers[k].lastUpdate = millis();
          peers[k].lastRssi = report->rssi;
          peers[k].matchCount = 0;
          peers[k].detractorCount = 0;
          bzero(peers[k].topics,MAX_TOPICS*TOPIC_SZ);
          updateTopic(k,top);
        }
      }
      //Serial.printf("Rx %s %s with rssi %d\r\n",adr,top,report->rssi);
    } //else if ( name[0] == 'M' ) Serial.printf("from (%s)\r\n",name);
  }
  Bluefruit.Scanner.resume();
}


void loop() {

  // Manage bootloader switch for over_the_cable fw update (type ! on serial console)
  while ( Serial && Serial.available() ) if (Serial.read()=='!') enterSerialDfu();

  // Make sure a device shutting down close to user will not be kept as close
  // clear rssi for old peers ( not updated since 10 seconds )
  for ( int i = 0; i < MAX_DEVICES; i++ ) {
    if ( peers[i].initialized && (millis() - peers[i].lastUpdate) > 10000 ) {
      peers[i].lastRssi = -120;
    }
  }

  // Detect close devices with -55dBm or more
  int closeDevices = 0;
  int bestRssi=-120, bestId=MAX_DEVICES+1;
  for ( int i = 0; i < MAX_DEVICES; i++ ) {
    if ( peers[i].initialized && peers[i].lastRssi >= -55 ) {
      closeDevices++;
      if ( bestRssi < peers[i].lastRssi ) { bestId = i ; bestRssi = peers[i].lastRssi; }
    }
  }

  // Light searching when no close devices
  analogWrite(D3,0);
  analogWrite(D4,0);
  analogWrite(D5,0);
  analogWrite(D6,0);
  analogWrite(D7,0);
  analogWrite(D8,0);
  if ( closeDevices == 0 ) {
    static int cnt = 0;
    switch(cnt) {
      case 0 : analogWrite(D7,20);analogWrite(D8,30);analogWrite(D3,120);break;
      case 1 : analogWrite(D8,10);analogWrite(D3,40);analogWrite(D4,100);break;
      case 2 : analogWrite(D3,20);analogWrite(D4,30);analogWrite(D5,120);break;
      case 3 : analogWrite(D4,10);analogWrite(D5,40);analogWrite(D6,100);break;
      case 4 : analogWrite(D5,20);analogWrite(D6,30);analogWrite(D7,120);break;
      case 5 : analogWrite(D6,10);analogWrite(D7,40);analogWrite(D8,100);break;
      default: break;
    }
    //digitalWrite(LED_BUILTIN,(cnt&1)?HIGH:LOW);
    cnt = ( cnt + 1 ) % 6;
    delay(300);
  } else {
    // find best device
    if ( peers[bestId].matchCount >= 1 ) analogWrite(D3,120);
    if ( peers[bestId].matchCount >= 2 ) analogWrite(D5,120);
    if ( peers[bestId].matchCount >= 3 ) analogWrite(D7,120);
    if ( peers[bestId].detractorCount >= 1 ) analogWrite(D4,120);
    if ( peers[bestId].detractorCount >= 2 ) analogWrite(D6,120);
    if ( peers[bestId].detractorCount >= 3 ) analogWrite(D8,120);
    delay(1000);
  }

  // Sometime the advertizing is killed... resume it
  if ( (millis() - watchdog) > 5000 ) {
     adv_stop_callback();
  }

  // Debug ----
  /*
  static uint32_t lastPrint = 0;
  if ( millis() - lastPrint > 3000 ) {
    lastPrint = millis();
    // Every 3 seconds print the peers info
    Serial.printf("Peers:\r\n");
    for ( int i = 0; i < MAX_DEVICES; i++ ) {
      if ( peers[i].initialized ) {
        Serial.printf("[%s] Rssi:%d Last:%dms Matches:%d Detractors:%d ",
          peers[i].peerId,
          peers[i].lastRssi,
          millis() - peers[i].lastUpdate,
          peers[i].matchCount,
          peers[i].detractorCount
        );
        int k = 0 ; while ( peers[i].topics[k][0] != 0 && k < MAX_TOPICS ) {
          Serial.printf(" (%s)",peers[i].topics[k]);
          k++;
        }
        Serial.printf("\r\n");
      }
    }
  }
  */
}
