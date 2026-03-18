# Devoxx France 2026, Atelier Meet & Chip

Ce dépôt contient le programme utilisé lors de l'atelier Meet & Chip de Devoxx France 2026, créé par Paul Pinault et Henri Gomez. Notre objectif est d'initier les développeurs à l'univers de la microélectronique et des objets connectés grâce à un petit gadget : un badge utilisant Bluetooth pour identifier les personnes partageant les mêmes centres d'intérêt et établir un contact. Au-delà de ce gadget, c'est l'occasion d'explorer le processus de création, d'aborder les microcontrôleurs, le Bluetooth et d'autres réseaux IoT, la conception de cartes, la fabrication, les mesures… et d'écrire un peu de code C basse consommation.

## Configuration de l'environnement

### Environnement matériel

Pour cet atelier, nous avons choisi d'utiliser un microcontrôleur Nordic nRF52840, distribué sous forme de module par Seeed Studio via sa gamme de produits XIAO. Seeed Studio nous a accompagnés tout au long de ce projet, permettant à chaque participant de repartir avec un badge. Nous avons choisi le nRF52 pour ses capacités BLE et sa
très faible consommation d'énergie, permettant une longue autonomie grâce à une simple pile bouton CR2032, largement disponible. Outre son soutien dans ce projet,

Seeed propose une gamme de produits bien documentés et bénéficiant d'un support technique complet, facilitant leur prise en main, notamment dans l'environnement Arduino.

#### Installation de VSCode et PlatformIO

Ce projet a été réalisé avec PlatformIO et VSCode.

1) Installez VSCode.

2) Installez l'extension PlatformIO.

3) Connectez le câble USB-C à la carte et à votre ordinateur.

#### Testez votre configuration

PlatformIO devrait reconnaître automatiquement votre carte. Lancez le programme et visualisez le résultat et vous verrez un message apparaitre dans la console série attaché à la carte

## Etape 1

Allez on s'amuse un peu avec notre badge qui a des leds. Récupérez le tag "etape-1" pour voir un exemple de ce qu'on peut faire avec les leds.
