# Simple-GCode-Decoder
Un decodeur de G-Code léger pour machine 3 axes compatible Arduino Uno. Ecrit par X. HINAULT (www.mon-club-elec.fr). Sous licence GPL v3. 

## Description 
Le Simple G-Code Decoder est un décodeur de G-Code minimaliste, de 10Ko environ pouvant donc tenir dans une carte Arduino, pour machine 3 axes, permettant de contrôler des moteurs pas-à-pas en mode micropas via des drivers A4988. 

L'objectif de ce décodeur est de fournir un décodeur de G-Code simple pour réaliser facilement de petites machines 1, 2 ou 3 axes contrôlables par le port série à partir de G-Code. 

## Compatibilité matérielle 

Le Simple GCode Decoder est compatible avec le CNC-Shield : http://blog.protoneer.co.nz/arduino-cnc-shield/

## Caractéristiques essentielles 

- Léger (seulement 10Ko environ !), programmable dans une simple carte Arduino
- supporte un jeu d'instructions de G-Code permettant une utilisation immédiate de G-Code généré avec PyCamm
- Entièrement paramétrable : mode micro-pas utilisé, nombre pas moteur, mm/pas, etc...
- Facile à utilisé avec des étages pololu A4988
- Brochage compatible GRBL et CNC-SHIELD
- Mémorisation du mode de coordonnées courant, relatif ou absolu

## Comparatif avec d'autre décodeurs de G-Code :

- Marlin (https://github.com/MarlinFirmware/Marlin) : décodeur de G-Code très complet utilisé sur les imprimantes 3D de type "reprap", mais beaucoup trop lourd (110Ko) pour de petits projets et en tout cas inutilisable avec une carte Arduino Uno. Un must pour l'impression 3D, trop lourd pour une petite machine 1,2 ou 3 axes.
 

- GRBL (https://github.com/grbl/grbl) : un décodeur de G-Code pour machine 3 axes complet, de ~30Ko, déjà trop lourd pour une machine simple, mais qui pourra être une bonne alternative au Simple GCode Decoder sur un projet qui grossirait... Le brochage utilisé pour le Simple G-Code Decoder est le même que celui utilisé par GRBL pour une transposition facilitée de l'un vers l'autre. 

## Instructions de G-Code implémentées

Les instructions de G-Code implémentées et reconnues par le Simple G-Code Decoder sont les suivantes : 

- G00 X888 Y888 Z888 F888 : mouvement rapide vers les coordonnées X,Y,Z, à la vitesse F en mm/sec
- G01 : mouvement linéaire vers les coordonnées X,Y,Z, à la vitesse F en mm/sec
- G04 P888 : pause en seconde
- G28 X Y Z : home du ou des axes précisés
- G90 : mode coordonnées absolues
- G91 : mode coordonnées relatives
- G92 : Fixe la position courante interne

- M99 : Fixe la correction logicielle du backlash à appliquer
- M100 : Aide
- M114 : Localisation courante

D'autres instructions seront ajoutées en fonction des besoins, mais ce jeu d'instruction suffit pour une utilisation d'une machine multi-outil 3 axes. Il est également très facile d'ajouter ses propres instructions personnaliées. 


## Utilisation 

Simplement programmer le programme dans une carte Arduino UNO. Envoyer ensuite des instructions de GCode valide sur le port série : elles seront reconnues et exécutées. 

## Exemple de mise en oeuvre : 

Le Simple GCode Decoder a été développé et est utilisé sur l'Open Maker Machine en situation réelle : voir http://www.mon-club-elec.fr/openmakermachine/index.html

## Interface graphique disponible

En complément du Simple GCode Decoder, il est possible d'utiliser une interface PyQt dédiée : l'interface [Simple GCode GUI](https://github.com/sensor56/Simple-GCode-GUI) du même auteur. 

## Correction du backlash

La commande M99 permet d'ajouter une correction logicielle du backlash (jeu) au changement de direction. La forme de la commande est : M99 Xnombrepas Ynombrepas Fsens. 

Le nombre de pas correspond au nombres de pas à réaliser à vide lors d'un changement de direction. 
Le sens prend la valeur 1 pour un changement de sens positif et -1 pour un changement de sens négatif. 

Exemple : 
- M99 X1200 Y600 Z600 F1   
- M99 X1200 Y600 Z600 F-1   

