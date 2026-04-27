## 🔌 Guide de Configuration USB (Devoxx 2026)

Ce guide explique comment faire reconnaître votre carte Seeed XIAO nRF52840 par votre ordinateur.

### 🛠️ 1. Drivers selon votre système

#### 🐧 Linux (Ubuntu, Debian, Fedora...)
Le XIAO nRF52840 est reconnu nativement, mais vous devez autoriser l'accès au port USB sans être root.
Installer les règles udev :
```
curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/master/scripts/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
sudo service udev restart
```

S'ajouter au groupe dialout :
```
sudo usermod -a -G dialout $USER
```

(Pensez à redémarrer votre session utilisateur).

#### 🍏 macOS
Aucun driver n'est normalement nécessaire pour le XIAO nRF52840 sur macOS (il utilise le driver CDC interne).
Si la carte n'apparaît pas, vérifiez dans Réglages Système > Confidentialité et sécurité s'il y a un message bloquant un accessoire USB.

#### 🪟 Windows (10 et 11)
Windows installe généralement le driver automatiquement.
Si la carte apparaît comme "Périphérique inconnu" dans le Gestionnaire de périphériques, installez les drivers Seeed officiels ici.

### 🚀 2. Le cas spécifique du XIAO nRF52840

Contrairement aux Arduino classiques, cette carte n'a pas de puce USB-Série dédiée. L'USB est géré par le microcontrôleur lui-même.

#### 🆘 En cas de blocage (Le mode "UF2 Bootloader")
Si votre code plante (boucle infinie, crash mémoire) ou si PlatformIO n'arrive plus à uploader :
Le geste magique : Appuyez deux fois très rapidement sur le bouton RESET (le petit bouton blanc à côté du port USB).
Le signal : Une LED rouge/orange va s'allumer en mode "respiration" (fading).
L'effet : Un nouveau lecteur nommé XIAO-SENSE apparaît sur votre bureau comme une clé USB.
L'action : PlatformIO pourra maintenant uploader votre code car le processeur est dans un mode "sain".
### ⚠️ 3. Check-list de dépannage (Talk-Ready)
Le câble : Utilisez-vous un câble DATA ? (Testez avec votre téléphone : s'il ne peut pas transférer de photos, le câble ne fonctionnera pas pour coder).
Le port : Évitez les hubs USB non alimentés. Branchez-vous directement sur l'ordinateur.
Le terminal : Tapez pio device list dans VS Code. Si vous ne voyez rien, c'est un problème de câble ou de driver.
