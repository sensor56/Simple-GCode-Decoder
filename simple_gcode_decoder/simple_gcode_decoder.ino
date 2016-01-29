// --- Programme Arduino --- 

// Auteur du Programme : X. HINAULT - Tous droits réservés - Licence GPL v3
// Programme écrit le : 02/2015 - MAJ 06/2015
// www.mon-club-elec.fr 

// initié d'après : https://www.marginallyclever.com/blog/2013/08/how-to-build-an-2-axis-arduino-cnc-gcode-interpreter/
// https://github.com/MarginallyClever/GcodeCNCDemo/blob/master/GcodeCNCDemo2AxisV2/GcodeCNCDemo2AxisV2.ino

// ------- Licence du code de ce programme ----- 
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License,
//  or any later version.
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

// ////////////////////  PRESENTATION DU PROGRAMME //////////////////// 

// -------- Que fait ce programme ? ---------

/* 
 * Ce programme est un décodeur de GCode simplifié pour machine 3 axes de base
 * /
 
// -------- Circuit à réaliser --------- 

/*
 * broches STEP des étages moteurs sur les broches STEP
 * broches DIR des étages moteurs sur les broches DIR
 * /

// /////////////////////////////// 1. Entête déclarative /////////////////////// 
// A ce niveau sont déclarées les librairies incluses, les constantes, les variables, les objets utiles...

// --- Déclaration des constantes ---

// --- Inclusion des librairies ---

// --- Déclaration des constantes utiles ---

// --- Déclaration des constantes des broches E/S numériques ---


// --- Déclaration des constantes des broches analogiques ---


// --- Déclaration des variables globales ---

/*
 int octetReception=0; // variable de stockage des valeurs reçues sur le port Série
 long nombreReception=0; // variable de stockage du nombre  reçu sur le port Série
 long nombreReception0=0; // variable de stockage du dernier nombre  reçu sur le port Série
 String chaineReception=""; // déclare un objet String vide pour reception chaine
*/

#define VERSION (0.3)

//--- config générale ---
#define BAUD (115200) // debit de communication
#define MAX_BUF (64) // taille max du buffer de reception serie

#define VERBOSE 1 // pour messages  

//--- vitesse de deplacement --- // en (mm/sec)
#define MAX_FEEDRATE (100.0) // vitesse maximale 
#define MIN_FEEDRATE (0.1) // vitesse minimale

//-- params moteurs
#define X 0
#define Y 1
#define Z 2

#define STEPS_PER_TURN (200) // pas par tour du moteur
#define MICROSTEPS_MODE (16) // mode micropas utilisé
#define MM_PER_TURN 1.25 // mm par tour - fonction pas de vis - M6 : 1mm | M8 : 1.25mm
#define PULSE_US 5 // largeur du pulse moteur en µs

//-- params mouvements
#define X_STEPS_PER_MM (STEPS_PER_TURN*MICROSTEPS_MODE/MM_PER_TURN)
#define Y_STEPS_PER_MM (STEPS_PER_TURN*MICROSTEPS_MODE/MM_PER_TURN)
#define Z_STEPS_PER_MM (STEPS_PER_TURN*MICROSTEPS_MODE/MM_PER_TURN)


// limites mouvements en mm
#define X_MAX_POS 290
#define X_MIN_POS 0
#define Y_MAX_POS 210
#define Y_MIN_POS 0
#define Z_MAX_POS 50
#define Z_MIN_POS 0

// chaque moteur est controlé par 
// > une broche de pulse : front montant déclenche pas suivant
// > une broche de dir : fixe le sens du moteur

//--- identifiant X,Y,Z pour indices tableaux
#define x (0)
#define y (1)
#define z (2)

//-- broches de pulse moteur

/*
const int X_STEP=2;
const int Y_STEP=3;
const int Z_STEP=4;
*/

int pin_STEP[3]={2,3,4}; // les broches de step des moteurs X,Y,Z

  
//-- broches de sens moteur

/*
const int X_DIR=5;
const int Y_DIR=6;
const int Z_DIR=7;
*/

int pin_DIR[3]={5,6,7}; // les broches de direction des moteurs X,Y,Z

//-- broche activation moteur 

const int pin_ENABLE=8; // broche activation moteur

//-- broches fin de course 

/*
const int X_END_STOP=8;
const int Y_END_STOP=9;
const int Z_END_STOP=10;
*/

int pin_ENDSTOP[3]={9,10,11}; // les broches de endstop des moteurs X,Y,Z
const int APPUI=LOW; // endstop actif sur niveau bas

//-- reception serie
char buffer[MAX_BUF];  // tableau de reception des caracteres
int sizeIn;  // nombre de caracteres presents dans le buffer

//--- position 
// variables de position courante
float px=0.0; 
float py=0.0;
float pz=0.0;

float pxn, pyn, pzn; // variable nouvelle position courante

//-- vitesse
float fr=1.0; // memo feedrate en mm/sec - utilisée pour G01
float frmax=6.0; // feedrate utilisé pour G00
float currentfr=0; // le feedrate courant actif - different de fr pour initialisation possible 

//long step_delay_ms;
long step_delay_us=0; // delai entre 2 pas 

//-- parametrages fonctionnement
int mode=90; // mode absolu/relatif - 90 = absolu 92 = relatif - par défaut absolu
long line_number=0;

int lastCmd=-1; // Variable mémorise dernier commande G appelée

//-- variables de backslash --

int bs_Up[3]={0,0,0}; // valeur de correction du backslash dans le sens mouvement positif
int bs_Down[3]={0,0,0}; // valeur de correction du backslash dans le sens mouvement négatif

// 0,0,0 par défaut pour permettre découpe, etc... utiliser vitesse minimale F1 
// pour perçage utiliser 1200, 600, 600 avec M99 X1200 Y600 Z600 F1 et M99 X1200 Y600 Z600 F-1 

int dir[3]={0,0,0}; // direction courante du mouvement : -1 si sens nég, +1 si sens +

int last_Dir[3]={0,0,0}; // mémorisation dernière direction du mouvement : -1 si sens nég, +1 si sens +
// 0 au départ = pas d'info sur dernier mouvement - un home X, Y est conseillé


// --- Déclaration des objets utiles pour les fonctionnalités utilisées ---


// ////////////////////////// 2. FONCTION SETUP = Code d'initialisation ////////////////////////// 
// La fonction setup() est exécutée en premier et 1 seule fois, au démarrage du programme

void setup()   { // debut de la fonction setup()

// --- ici instructions à exécuter 1 seule fois au démarrage du programme --- 

// ------- Initialisation fonctionnalités utilisées -------  

	Serial.begin(115200); // initialise connexion série à 115200 bauds

	delay(1000); 

	help(); // messages d'instruction

	ready(); // fonction qui réinitialise contenu buffer

	feedrate(1); // initialisation feedrate 
		
// IMPORTANT : régler le terminal côté PC avec la même valeur de transmission 


// ------- Broches en sorties numériques -------  

// broches de pulse moteur
pinMode(pin_STEP[X], OUTPUT); // met broche en sortie
pinMode(pin_STEP[Y], OUTPUT); // met broche en sortie
pinMode(pin_STEP[Z], OUTPUT); // met broche en sortie 

digitalWrite(pin_STEP[X], LOW); // met broche niveau BAS
digitalWrite(pin_STEP[Y], LOW); // met broche niveau BAS
digitalWrite(pin_STEP[Z], LOW); // met broche niveau BAS


// broches de dir moteur
pinMode(pin_DIR[X], OUTPUT); // met broche en sortie
pinMode(pin_DIR[Y], OUTPUT); // met broche en sortie
pinMode(pin_DIR[Z], OUTPUT); // met broche en sortie 

digitalWrite(pin_DIR[X], LOW); // met broche niveau BAS
digitalWrite(pin_DIR[Y], LOW); // met broche niveau BAS
digitalWrite(pin_DIR[Z], LOW); // met broche niveau BAS

// broches de endstop
pinMode(pin_ENDSTOP[X], INPUT_PULLUP); // met broche en entrée avec rappel au + actif
pinMode(pin_ENDSTOP[Y], INPUT_PULLUP); // met broche en entrée avec rappel au + actif
pinMode(pin_ENDSTOP[Z], INPUT_PULLUP); // met broche en entrée avec rappel au + actif

/*
digitalWrite(pin_ENDSTOP[X], HIGH); // met broche avec rappel au + actif
digitalWrite(pin_ENDSTOP[Y], HIGH); // met broche avec rappel au + actif
digitalWrite(pin_ENDSTOP[Z], HIGH); // met broche avec rappel au + actif
*/

//---- broche activation moteur 
pinMode(pin_ENABLE, OUTPUT); // broche en sortie 
digitalWrite(pin_ENABLE, LOW); // activation par defaut moteur - ENABLE ACTIF SUR NIVEAU BAS ++

// ------- Broches en entrées numériques -------  

// ------- Activation si besoin du rappel au + (pullup) des broches en entrées numériques -------  

// ------- Initialisation des variables utilisées -------  

} // fin de la fonction setup()
// ********************************************************************************

////////////////////////////////// 3. FONCTION LOOP = Boucle sans fin = coeur du programme //////////////////
// la fonction loop() s'exécute sans fin en boucle aussi longtemps que l'Arduino est sous tension

void loop(){ // debut de la fonction loop()


// --- ici instructions à exécuter par le programme principal --- 



//---- code type réception chaine sur le port série ---
/*
while (Serial.available()>0) { // tant qu'un octet en réception 
	octetReception=Serial.read(); // Lit le 1er octet reçu et le met dans la variable

	if (octetReception==10) { // si Octet reçu est le saut de ligne 
		Serial.println (chaineReception); // affiche la chaine recue
		chaineReception=""; //RAZ le String de réception
		break; // sort de la boucle while
	}
	else { // si le caractère reçu n'est pas un saut de ligne
		chaineReception=chaineReception+char(octetReception); // ajoute le caratère au String
		//Serial.print(char(octetReception)); // affiche la chaine recue
	}
*/

  // Gcode de la forme : G1 Z5 F5000 ; 

	// reception serie
	while(Serial.available() > 0) {  // tant qu'un caractere dans la file d'attente serie
		
		char c=Serial.read();  // lecture du caractere suivant
		//Serial.print(c);  // affichage 
		
		if(sizeIn<MAX_BUF) buffer[sizeIn++]=c;  // on le memorise dans le buffer tant que place
		
		//if(buffer[sizeIn-1]==';') break;  // on sort si dernier caractere recu=;
		if (c==';') break; // equivalent car c est le dernier caractere recu - 
		// on analyse après...

		if (Serial.available()==0) delay(5); // laisse temps éventuel nouveau caractere arriver

	} // fin while  octet réception

	// si arrivee instruction GCode valide
	//if(sizeIn>0 && buffer[sizeIn-1]==';') { // si caractere dans le buffer et dernier est un ;
	if(sizeIn>0 && buffer[sizeIn-1]=='\n') { // si caractere dans le buffer et dernier est un \n
	
		// lorsqu'un message GCode valide a été recu
		buffer[sizeIn]=0;  // ajoute un 0 à la fin du buffer pour traitement en chaine de caractere
		
		//Serial.print(F("\r\n"));  // affiche message
		// if debug : Serial.println(buffer);  // affiche message - traité comme une chaine 
		  
		analyseChaine();  // analyse la commande
		
		ready(); // reinitialise buffer
		
	}// fin else

	// --- commande personnalisée --- => a mettre dans analyse chaine.. 
	/*else if (sizeIn>0 && buffer[0]=='$') { // sizeIn>0 et ready() necessaires 

		Serial.println("status()");
		status();
		ready(); // reinitialise buffer

	} // else if
	*/
	else if(sizeIn>0 ) { // sinon on vide le buffer reception
		
		 Serial.println(); // saut ligne 
		 ready(); // reinitialise buffer
		 
	} // fin else if

	/* // -- test des endstops --
	  // lecture des endstops
	  if (digitalRead(pin_ENDSTOP[X])==APPUI) { 
		Serial.println("Appui ENDSTOP X");
		delay(100); // anti-rebond  
	  } // fin si appui
  
	  if (digitalRead(pin_ENDSTOP[Y])==APPUI) { 
		Serial.println("Appui ENDSTOP Y");
		delay(100); // anti-rebond  
	  } // fin si appui
  
	  if (digitalRead(pin_ENDSTOP[Z])==APPUI) { 
		Serial.println("Appui ENDSTOP Z");
		delay(100); // anti-rebond  
	  } // fin si appui
	
	*/
	
} // fin de la fonction loop() - le programme recommence au début de la fonction loop sans fin
// ********************************************************************************


// ////////////////////////// FONCTIONS DE GESTION DES INTERRUPTIONS //////////////////// 


// ////////////////////////// AUTRES FONCTIONS DU PROGRAMME //////////////////// 

// message d'aide
void help() {
	Serial.print(F("--- Simple Gcode --- v"));
	Serial.println(VERSION);
	Serial.println(F("Par X.HINAULT - www.mon-club-elec.fr - GPLv3"));

	Serial.println(F("$ Statut interne "));
	Serial.println(F("Commandes:"));
	Serial.println(F("G00 [X(steps)] [Y(steps)] [F(feedrate)]; - linear move"));
	Serial.println(F("G01 [X(steps)] [Y(steps)] [F(feedrate)]; - linear move"));
	Serial.println(F("G04 P[seconds]; - delay"));
	Serial.println(F("G90; - absolute mode"));
	Serial.println(F("G91; - relative mode"));
	Serial.println(F("G92 [X(steps)] [Y(steps)]; - change logical position"));
	Serial.println(F("M18; - disable motors"));
	Serial.println(F("M100; - this help message"));
	Serial.println(F("M114; - report position and feedrate"));
  
  	Serial.println(F("M99; [X(steps)] [Y(steps)] [Z(steps)] [F(-1/1)]; - change backlash correction"));
}

//---- message etat ----
void status() {
 
 output("X steps/mm : ",X_STEPS_PER_MM); 
 output("Y steps/mm : ",Y_STEPS_PER_MM); 
 output("Z steps/mm : ",Z_STEPS_PER_MM); 

 output("Feedrate mm/s : ",currentfr); 
 output("step_delay_us : ", step_delay_us);
 
 output("pos X :", px);
 output("pos Y :", py);
 output("pos Z :", pz);

 output("bsUp x = ", bs_Up[x]); // backslash
 output("bsUp y = ", bs_Up[y]); // backslash
 output("bsUp z = ", bs_Up[z]); // backslash
 
 output("bsDown x = ", bs_Down[x]); // backslash
 output("bsDown y = ", bs_Down[y]); // backslash
 output("bsDown z = ", bs_Down[z]); // backslash

 Serial.println((mode-90)?"REL":"ABS");// affiche mode ABS ou REL - mode 90 : ABS et 92 : REL
}

//--- RAZ buffer reception 
void ready() {
  sizeIn=0;  // RAZ nombre caracteres en reception 
  //Serial.flush(); // vide file d'attente 
  Serial.print(F(">>>"));  // signal ready to receive input
}

void analyseChaine() {
  
  // Gcode de la forme : G1 Z5 F5000 ; 
  
  //float cmd=parsenumber('G',-1); // la fonction renvoie valeur passée = -1 si pas de changement
  //Serial.println (cmd);
  
  //------- code exit = ; initial -------
  if (buffer[0]==';'){
	  sendOK() ; 
	  return; // sort de la fonction
  } // fin if ; 
  
  //------- decodage instruction G ------------
  else if (buffer[0]==' ') analyseEspace(); // si espace en premier = on utilise la dernière commande 

  //------- decodage instruction G ------------
  else if (buffer[0]=='G') analyseG();
  
  //---------- decodage instructions M --------------- 
  else if (buffer[0]=='M') analyseM();

  //--- si commande $ pour status interne ---
  else if (buffer[0]=='$') {
  		Serial.println("status()");
		status();
		ready(); // reinitialise buffer
		sendOK(); 
  }
  else sendOK(); 
  
} // fin analyse chaine


//--------- fonction d'analyse des chaines du buffer serie

// detecte les lettres suivies de valeur numerique dans la chaine
// Gcode de la forme : G1 Z5 F5000 ; 
// la fonction parsenumber reçoit une valeur val qui restera inchangée si pas de valeur trouvée
float parsenumber(char code,float val) {
  
  char *ptr=buffer; // recupere pointeur buffer reception
  
  while(ptr && *ptr && ptr<buffer+sizeIn) {
	
	if(*ptr==code) {
	return atof(ptr+1); // conversion en float de ce qui suit la lettre
	 } // fin if
	 
	ptr=strchr(ptr,' ')+1; // recherche de caractere dans une chaine - +1 pour décaler...  devient 0 si pas ? ici ' ' = fin de chaine
  
  } // fin while
  
return val;

} // fin parsenumber

//----- analyse espace -----
void analyseEspace() { // si que X, Y Z précédé de espace, on se cale sur la dernière commande reçue
	
	if (lastCmd==01) G01(), sendOK(); // si dernière commande est G01 - en prem's car le plus frequent
	else if (lastCmd==00) G00(), sendOK(); // si dernière commande est G00
	else 	sendOK();
	
} // fin analyse espace
 
//----- analyse chaine G -----
void analyseG() {
	
	int cmd = parsenumber('G',-1);
	switch(cmd) {

	//---- code 0x ----

	// G00 // Déplacement rapide - idem G01 en rapide
	case 0: 
		G00();
		lastCmd=00;
		sendOK();
		break;
	  
	// G01 : // mouvement linéaire vitesse standard
	case 1: // move linear
		G01();
		lastCmd=01; 
		sendOK();
		break;

	// G04 : // pause 
	case 4: 
	  Serial.println(F("G04"));
	  //pause(parsenumber('S',0) + parsenumber('P',0)*1000.0f);

	  float delai;
	  delai=parsenumber('P',0);       
	  Serial.print("Pause "), Serial.print(delai), Serial.println(" sec.");   
	  delay(delai*1000.0f);  
	  
	  sendOK();
		   
	  break; // 

	//--- codes 1x --
	case 17: // ..
	  Serial.println(F("G17"));
	  sendOK();
	  break;

	//--- codes 2x --

	case 20: // ..
	  Serial.println(F("G20"));
	  sendOK();
	  break;

	case 28: // home axes
		G28(); 
		sendOK();
		break;

	//--- codes 5x --

	case 54: // ..
	  Serial.println(F("G54"));
	  sendOK();
	  break;

	//--- codes 9x --

	// G90 : mode coordonnées absolues
	case 90: 
	  Serial.println(F("G90"));
	  mode=90; 
	  sendOK();
	  break; 

	// G91 : mode coordonnées relatives
	case 91: 
	  Serial.println(F("G91"));
	  mode=91; 
	  sendOK();
	  break; // relative mode

	case 92: // fixe la position courante interne - pas de mouvement
		G92();
		//position( parsenumber('X',0),
		//parsenumber('Y',0) );
		sendOK();
		break;

	 case 94: // ..
	  Serial.println(F("G94"));
	  sendOK();
	  break;

	default: 
		sendOK();
	  break;
	
	} // fin switch
	
} // fin analyse G

//------- analyse M --------
void analyseM() {
	
	int cmd = parsenumber('M',-1);
  switch(cmd) {
	case 18: // arret moteurs
	  Serial.println(F("M18"));
	  //release();
	  sendOK();
	  break;

	case 99: // fixe le backslash (hysteresis) en micropas - F indique up ou down avec F+1/-1  
		Serial.println(F("M99"));
		
		if (parsenumber('F',0)==1) { // si F1 = backslash up
			
			bs_Up[x]=parsenumber('X',bs_Up[x]); // fixe le nouveau backslash ou reste inchangé si absent dans chaine
			//output("bsUp x = ", bs_Up[x]); // debug

			bs_Up[y]=parsenumber('Y',bs_Up[y]); // fixe le nouveau backslash ou reste inchangé si absent dans chaine
			//output("bsUp y = ", bs_Up[y]); // debug

			bs_Up[z]=parsenumber('Z',bs_Up[z]); // fixe le nouveau backslash ou reste inchangé si absent dans chaine
			//output("bsUp z = ", bs_Up[z]); // debug

		} // fin if F1

		else if (parsenumber('F',0)==-1){ // si F-1 = backslash down

			bs_Down[x]=parsenumber('X',bs_Down[x]); // fixe le nouveau backslash ou reste inchangé si absent dans chaine
			//output("bsDown x =", bs_Down[x]); // debug

			bs_Down[y]=parsenumber('Y',bs_Down[y]); // fixe le nouveau backslash ou reste inchangé si absent dans chaine
			//output("bsDown y =", bs_Down[y]); // debug

			bs_Down[z]=parsenumber('Z',bs_Down[z]); // fixe le nouveau backslash ou reste inchangé si absent dans chaine
			//output("bsDown z =", bs_Down[z]); // debug

		}// fin if F-1
		
		else {

			Serial.println("Preciser F valide (1 ou -1)");

		}// fin else
		
		sendOK();
		break;
	  
	case 100: 
	  Serial.println(F("M100"));
	  help(); 
	  sendOK();
	  break;
	
	case 110: 
	  Serial.println(F("M110"));
	  //line_number = parsenumber('N',line_number); 
	  sendOK();
	  break;
	
	case 114: 
	  Serial.println(F("M114"));
	  where(); 
	  sendOK();
	  break;
	  
	default: 
		sendOK();
	  break;
   
} // fin switch
   
}// fin analyse M

//----- G00 ---- 
void G00(){
	Serial.println(F("G00"));
	feedrate(frmax);// on utilise le feedrate max
	parseFXYZ(); // on fait comme G01 mais avec feedrate maxi

} // fin G00

//---- GO1 --- 
void G01(){
	Serial.println(F("G01"));
	feedrate(fr);// on utilise le feedrate simple
	parseFXYZ(); 

} // fin G01

//---- G28 --- 
void G28(){
	Serial.println(F("G28"));
	feedrate(frmax);// on utilise le feedrate rapide
	
	int  test=0.0; // variable pour test 
	// -- axe X
	test=parsenumber('X',-1); // nouvelle position X - on passe -1 en parametre - sera idem si pas present - note X seul idem X0
	output("X :", test);
	
	if (test!=-1) { // si X : on va chercher le endstop X
		
		searchEndstop(X); // fonction de recherche endstop
		px=0.0; // RAZ position X
		
	} // fin if 

	// -- axe Y
	test=parsenumber('Y',-1); // nouvelle position Y - on passe -1 en parametre - sera idem si pas present - note Y seul idem Y0
	output("Y :", test);
	
	if (test!=-1) { // si Y : on va chercher le endstop Y
		
		searchEndstop(Y); // fonction de recherche endstop
		py=0.0; // RAZ position Y
		
	} // fin if 

	// -- axe Z
	test=parsenumber('Z',-1); // nouvelle position Z - on passe -1 en parametre - sera idem si pas present - note Z seul idem Z0
	output("Z :", test);
	
	if (test!=-1) { // si X : on va chercher le endstop X
		
		searchEndstop(Z); // fonction de recherche endstop
		pz=0.0; // RAZ position X
		
	} // fin if 


} // fin G28

//---- G92 --- modifie valeurs absolues à 0 - quasi idem G28 sans recherche des endstops 
void G92(){
	Serial.println(F("G92"));
	
	int  test=0.0; // variable pour test 
	// -- axe X
	test=parsenumber('X',-1); // nouvelle position X - on passe -1 en parametre - sera idem si pas present - note X seul idem X0
	//output("X :", test);
	
	if (test!=-1) px=0.0; // si X : RAZ position X 

	// -- axe Y
	test=parsenumber('Y',-1); // nouvelle position Y - on passe -1 en parametre - sera idem si pas present - note Y seul idem Y0
	//output("Y :", test);
	
	if (test!=-1) py=0.0; // si Y : RAZ position Y 

	// -- axe Z
	test=parsenumber('Z',-1); // nouvelle position Z - on passe -1 en parametre - sera idem si pas present - note Z seul idem Z0
	//output("Z :", test);
	
	if (test!=-1) pz=0.0; // si Z : RAZ position Z 


} // fin G92
//--- fonction de recherche d'un endstop
void searchEndstop(int axisIn) {
	
		int search=true; 
		
		while(search) {
			onestep(axisIn,-1); // 1 pas en arriere
			tick(); // delay de feedrate
			
			if (digitalRead(pin_ENDSTOP[axisIn])==APPUI) { // on teste le endstop
				Serial.println("Appui ENDSTOP"); // debug
				delay(100); // anti-rebond  
				search=false; // pour sortir du while
			} // fin si appui

		} // fin while

		last_Dir[axisIn]=-1; // MAJ lastDir[axisIn]


} // fin searchEndstop

//---- parse FXYZ ----
void parseFXYZ(){
		
		//feedrate(parsenumber('F',fr)); // analyse valeur FXXX présent aussi dans l'instruction - fixe la vitesse 
		float newfr=parsenumber('F',-1.0); // extraction du feedrate - currentfr est passé en paramètre à parsenumber = valeur inchangée si pas de valeur renvoyée

		if (newfr!=-1.0) {
			feedrate(newfr); // prend en compte le nouveau feedrate -- 
			fr=newfr; // mémorise nouveau fr
		} // fin if 

		//output("feedrate : ", currentfr); 

		//line( parsenumber('X',(mode_abs?px:0)) + (mode_abs?0:px),parsenumber('Y',(mode_abs?py:0)) + (mode_abs?0:py) );
		// condition ? valtrue : valfalse : si condition est vraie, valeur true sinon valeur false
		// la ligne (mode_abs?px:0)) + (mode_abs?0:px) permet d'utiliser coord absolue ou relative = bien vu !

		//-- mouvement absolus -- 
		if (mode==90) { // mode absolu
			pxn=parsenumber('X',px); // nouvelle position X - on passe px en parametre pour idem si pas present
			//output("X :", pxn);

			pyn=parsenumber('Y',py); // nouvelle position Y - on passe py en parametre pour idem si pas present
			//output("Y :", pyn); 

			line(pxn, pyn); // mouvement linéaire en nouvelle position 

			pzn=parsenumber('Z',pz); // nouvelle position X - on passe px en parametre pour idem si pas present
			//output("Z :", pzn); 

			outil(pzn); // mouvement outil

			// memorisation nouvelles valeurs absolues (dans tous les cas) 
			px=pxn; 
			py=pyn; 
			pz=pzn; 

		} // fin if mode absolu
		
		//-- mouvements relatifs -- 
		else if (mode==91){
			
			pxn=parsenumber('X',0); // nouvelle position X - on passe 0 en parametre pour idem si pas present
			//output("X :", pxn);

			pyn=parsenumber('Y',0); // nouvelle position Y - on passe 0 en parametre pour idem si pas present
			//output("Y :", pyn); 

			line(px+pxn, py+pyn); // mouvement linéaire en nouvelle position relativement à la précédente

			pzn=parsenumber('Z',0); // nouvelle position X - on passe 0 en parametre pour idem si pas present
			//output("Z :", pzn); 

			outil(pz+pzn); // mouvement outil en nouvelle position relativement à la précédente

			// memorisation nouvelles valeurs absolues (dans tous les cas) 
			px=px+pxn; // position courante + nouveau déplacement
			py=py+pyn; // position courante + nouveau déplacement
			pz=pz+pzn; // position courante + nouveau déplacement

			//output("X :", px);
			//output("Y :", py); 
			//output("Z :", pz); 

		} // fin si mode relatif

} // fin parseFXYZ

//============ fonctions d'affichage utiles ======================

// affiche message + valeur sur 1 ligne
void output(char *code,float val) {
  Serial.print(code);
  Serial.println(val);
} // fin output


// send ok
void sendOK() {
	
	Serial.println(F("<ok>"));
	
} // sendOK

//============= fonctions de position ================

// affiche position courante 
void where() {
  output("X",px); // le x actuel 
  output("Y",py); // y actuel
  output("Z",pz); // y actuel
  output("F",fr); // le feedrate actuel 
  Serial.println((mode-90)?"REL":"ABS");// affiche mode ABS ou REL - mode 90 : ABS et 92 : REL
} // fin where



//============= fonctions de mouvements moteurs ===================

/**
* Set the feedrate (speed motors will move)
* @input nfr the new speed in steps/second
*/

// fonction qui modifie le feedrate
void feedrate(float nfr) {
  if(currentfr==nfr) return; // same as last time? quit now.
  
  // le feedrate s'exprime en mm par seconde 
  // ici on calcul le delay entre 2 pas en µs qui vaut donc : 
  // step_delay_us = 1 000 000 / total_steps_per_mm = 1 000 000 / fr * axe_steps_per_mm

  if (nfr!=0){ // pour éviter division par zero si nfr=0
	step_delay_us=1000000/(nfr*X_STEPS_PER_MM); // calcul delai entre 2 pas
	//output("step_delay_us : ", step_delay_us);
  } // fin if nfr!=0

  /*
  if(nfr>MAX_FEEDRATE || nfr<MIN_FEEDRATE) { // limitation feedrate 
	Serial.print(F("Le feedrate doit etre superieur a "));
	Serial.print(MIN_FEEDRATE);
	Serial.print(F("steps/s et inferieur a "));
	Serial.print(MAX_FEEDRATE);
	Serial.println(F("steps/s."));
	return;
  } // fin if
  */
  
  // calcul du feedrate - ici en pas par minute 
  /*long us_per_min = 60 * 1000 * 1000;
  long frmin = us_per_min / nfr;
  step_delay_ms = frmin / 1000;
  step_delay_us = frmin % 1000;
  */
  
  currentfr=nfr;

} // fin feedrate

// mouvement en nouvelle position X,Y
void line(float newx,float newy) {

	// calcul delta x et y
	long dx=(newx-px)*X_STEPS_PER_MM;
	long dy=(newy-py)*Y_STEPS_PER_MM;

	// fixe sens moteur adequat
	//int dirx=dx>0?1:-1;
	//int diry=dy>0?-1:1; // because the motors are mounted in opposite directions
	
	//int dirx=dx>0?0:1; // 0 et 1 plutôt que -1 et 1 = utilisation directe valeur LOW/HIGH - mais pas intuitif
	//int diry=dy>0?0:1; // 0 et 1 plutôt que -1 et 1

	//int dirx=dx>0?1:-1; // -1 et 1 plutôt que 0 et 1
	//int diry=dy>0?1:-1; // -1 et 1 plutôt que 0 et 1

	dir[x]=dx>0?1:-1; // -1 et 1 plutôt que 0 et 1
	dir[y]=dy>0?1:-1; // -1 et 1 plutôt que 0 et 1

	output("dirx", dir[x]); // debug
	output("diry", dir[y]); // debug

	//dy>0?0:1 : syntaxe de condition inline

	// ---- prise en compte du changement de direction et correction du backslash avant le mouvement --- 

	// -- axe X
	backslash(x,dx); 


	// -- axe Y
	backslash(y,dy); 

	// -- axe Z
	// pas ici - voir outil.. 
	

	// valeur absolue de delta x, y 
	dx=abs(dx);
	dy=abs(dy);
	
	// variables utiles
	long i;
	long over=0;
	
	// debug
	#ifdef VERBOSE
	//Serial.println(F("Start XY>"));
	#endif
	
	// boucle de mouvement 
	if(dx>dy) {
		
		for(i=0;i<dx;++i) {
			onestep(X,dir[x]); // moteur X
			over+=dy;
			
			if(over>=dx) {
				over-=dx;
				onestep(Y,dir[y]); // moteur Y
			} // fin if

		tick(); // pause entre 2 pas

		/*
		// détection des endstop de sécurité - revoir pour détection tous les npas...
		if (digitalRead(pin_ENDSTOP[x])==APPUI) { // on teste le endstop x - si appui, on sort de la fonction line de mouvement

			Serial.println("Appui endstop X : arret des mouvements"); 
			return; // on sort de la fonction line 

		} // fin if endstop x
		
		if (digitalRead(pin_ENDSTOP[y])==APPUI) { // on teste le endstop y - si appui, on sort de la fonction line de mouvement
			
			Serial.println("Appui endstop Y : arret des mouvements"); 
			return;  // on sort de la fonction line
			
		} // fin if endstop y
		*/
		
		} // fin for
		
	} // fin if 
	
	else { // si dy<dx
		
		for(i=0;i<dy;++i) {
			onestep(Y,dir[y]);
			over+=dx;
			
			if(over>=dy) {
				over-=dy;
				onestep(X,dir[x]);
			} // fin if
	
			tick();
			
		} // fin for
		
	} // fin else
	
} // fin line 

void outil(float newz) {

	long dz=(newz-pz)*Z_STEPS_PER_MM;
	
	//int dirz=dz>0?0:1; // 0 et 1 plutôt que -1 et 1

	//int dirz=dz>0?1:-1; // -1 et 1 plutôt que 0 et 1

	dir[z]=dz>0?1:-1; // -1 et 1 plutôt que 0 et 1
	output("dirz", dir[z]); // debug

	// valeur absolue de delta z 
	dz=abs(dz);
	
	// variables utiles
	long i;
	
	// debug
	#ifdef VERBOSE
	//Serial.println(F("Start Z>"));
	#endif
	
	// boucle de mouvement 		
		for(i=0;i<dz;++i) {
			onestep(Z,dir[z]); // moteur Z		
		   tick();
		} // fin for
	
} // fin outil 


// fonction 1 pas moteur 
void onestep(int motor,int dirIn) {
  
  // motor =0 pour X, 1 pour Y et 2 pour Z

  // fixe sens moteur
  int direction=0; // variable locale etat broche direction
  if (dirIn==-1) direction=LOW; else direction=HIGH; // conversion -1 / 1 en LOW/HIGH
  digitalWrite(pin_DIR[motor], direction); // sens du pas - direction vaut 0 ou 1

  // genere pulse moteur 
  digitalWrite(pin_STEP[motor], HIGH); // pulse de pas
  delayMicroseconds(PULSE_US);
  digitalWrite(pin_STEP[motor],LOW); // fin pulse
  
  //delayMicroseconds(step_delay_us-PULSE_US); // - pulse_us pour pas exactement tous les step_delay_us
  // mis dans tick pour avoir tous les moteurs positionnés ensemble. Pause commune pour l'ensemble...
  
} // fin onestep 


void tick() { // pause entre deux pulse moteur 

	delayMicroseconds(step_delay_us-PULSE_US); // - pulse_us pour pas exactement tous les step_delay_us
	// en fait, on ne tient compte que d'un pulse moteur... mais il peut y en avoir plusieurs
	
} // fin tick.

//void pause(long ms) {
  //delay(ms/1000);
  //delayMicroseconds(ms%1000); // delayMicroseconds doesn't work for values > ~16k.
//}

void position(float npx,float npy) {
  // fixe la position courante ... 
  px=npx;  
  py=npy;
} // fin position

// fonction de correction logicielle du jeu d'écrou (backslash)
void backslash(int axisIn, int dIn) { // dIn = delta courant sur l'axe

	//output("axe:",axisIn); 

	if (last_Dir[axisIn]==0) { // si valeur initiale = si pas de home...
		//Serial.println(F("Home axe non fait")); // debug
		
		last_Dir[axisIn]=dir[axisIn]; // MAJ lastDir[x] - de cette façon mouvement suivant aura correction backslash
		
	} // fin last-Dir==0

	else if (dIn==0) { // si pas de mouvement sur X
		//Serial.println(F("Pas de mouvement sur axe")); // debug
		// pas de MAJ de lastDir qui reste inchangé depuis dernier mouvement
	}
	else if (last_Dir[axisIn]<dir[axisIn]) { // si inversion sens X du négatif (précédent) vers positif (actuel)
		
		//Serial.println(F("Inversion sens negatif vers positif : compensation du Backslash Up")); // debug
		//Serial.println(bs_Up[axisIn]); // debug
		
		for (int i=0; i<bs_Up[axisIn]; i++) { // boucle de n pas de compensation du backslash up
			onestep(axisIn,dir[axisIn]); // avance d'un pas sens voulu
			tick(); // pause entre 2 pas
		} // fin for
		// note : les n pas sont effectués d'un coup
		
		last_Dir[axisIn]=dir[axisIn]; // MAJ lastDir[x]
		
		
	} // fin else if sens - vers +

	else if (last_Dir[axisIn]>dir[axisIn]) { // si inversion sens X du positif (précédent) vers negatif (actuel)
		
		// Serial.println(F("Inversion sens X positif vers negatif : compensation du Backslash Down")); // debug
		// Serial.println(bs_Down[axisIn]); // debug

		for (int i=0; i<bs_Down[axisIn]; i++) { // boucle de n pas de compensation du backslash down
			onestep(axisIn,dir[axisIn]); // avance d'un pas sens voulu - négatif ici 
			tick(); // pause entre 2 pas
		} // fin for
		// note : les n pas sont effectués d'un coup

		
		last_Dir[axisIn]=dir[axisIn]; // MAJ lastDir[x]
	
	} // fin else if sens + vers -
	
} // fin backslash

// ////////////////////////// Fin du programme //////////////////// 


// ////////////////////////// Mémo instructions //////////////////// 
// ////////////////////////// Fin Mémo instructions //////////////////// 


