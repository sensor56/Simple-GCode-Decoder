# Simple-GCode-Decoder
Un decodeur de G-Code léger pour machine 3 axes compatible Arduino Uno. Ecrit par X. HINAULT (www.mon-club-elec.fr). Sous licence GPL v3. 

## Description 
Le Simple G-Code Decoder est un décodeur de G-Code minimaliste, de 10Ko environ pouvant donc tenir dans une carte Arduino, pour machine 3 axes, permettant de contrôler des moteurs pas-à-pas en mode micropas via des drivers A4988. 

L'objectif de ce décodeur est de fournir un décodeur de G-Code simple pour réaliser facilement de petites machines 1, 2 ou 3 axes contrôlables par le port série à partir de G-Code. 

## Compatibilité matérielle 

Le Simple GCode Decoder est compatible avec le CNC-Shield : http://blog.protoneer.co.nz/arduino-cnc-shield/

## Comparatif avec d'autre décodeurs de G-Code :

- Marlin (https://github.com/MarlinFirmware/Marlin) : décodeur de G-Code très complet utilisé sur les imprimantes 3D de type "reprap", mais beaucoup trop lourd (110Ko) pour de petits projets et en tout cas inutilisable avec une carte Arduino Uno. Un must pour l'impression 3D, trop lourd pour une petite machine 1,2 ou 3 axes.
 

- GRBL (https://github.com/grbl/grbl) : un décodeur de G-Code pour machine 3 axes complet, de ~30Ko, déjà trop lourd pour une machine simple, mais qui pourra être une bonne alternative au Simple GCode Decoder sur un projet qui grossirait... Le brochage utilisé pour le Simple G-Code Decoder est le même que celui utilisé par GRBL pour une transposition facilitée de l'un vers l'autre. 

## Instructions de G-Code implémentées

Les instructions de G-Code implémentées et reconnues par le Simple G-Code Decoder sont les suivantes : 

