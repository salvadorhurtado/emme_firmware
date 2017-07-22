//ANILLO NEOPIXEL

#include <Wire.h>
#include <Adafruit_NeoPixel.h>

#define PIN            10
#define NUMPIXELS      12

Adafruit_NeoPixel anillo_rueda = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//JSON
#include <ArduinoJson.h>


String inputStringPrisma = "";

StaticJsonBuffer<200> jsonBufferEnviar;
JsonObject& jsonEnviar = jsonBufferEnviar.createObject();

//WINC1500
#include <SPI.h>
#include <Adafruit_WINC1500.h>
#include <Adafruit_WINC1500Udp.h>


// Define the WINC1500 board connections below.
// If you're following the Adafruit WINC1500 board
// guide you don't need to modify these:
#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2     // or, tie EN to VCC and comment this out
// The SPI pins of the WINC1500 (SCK, MOSI, MISO) should be
// connected to the hardware SPI port of the Arduino.
// On an Uno or compatible these are SCK = #13, MISO = #12, MOSI = #11.
// On an Arduino Zero use the 6-pin ICSP header, see:
//   https://www.arduino.cc/en/Reference/SPI

// Setup the WINC1500 connection with the pins above and the default hardware SPI.
Adafruit_WINC1500 WiFi(WINC_CS, WINC_IRQ, WINC_RST);

IPAddress ip; //guardo la ip asignada por el router wifi

char ssid[] = "xxxx";     //  your network SSID (name)
char pass[] = "xxxxxxxx";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;


char packetBuffer[255]; //buffer to hold incoming packet


// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
Adafruit_WINC1500Client client;

Adafruit_WINC1500UDP Udp;


const int IN1 = 5;
const int IN2 = 6;
const int IN3 = 13;

const int ENABLE = 11;

const int pwmSin[] = {128, 132, 136, 140, 143, 147, 151, 155, 159, 162, 166, 170, 174, 178, 181, 185, 189, 192, 196, 200, 203, 207, 211, 214, 218, 221, 225, 228, 232, 235, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 248, 249, 250, 250, 251, 252, 252, 253, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 253, 252, 252, 251, 250, 250, 249, 248, 248, 247, 246, 245, 244, 243, 242, 241, 240, 239, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 248, 249, 250, 250, 251, 252, 252, 253, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 253, 252, 252, 251, 250, 250, 249, 248, 248, 247, 246, 245, 244, 243, 242, 241, 240, 239, 238, 235, 232, 228, 225, 221, 218, 214, 211, 207, 203, 200, 196, 192, 189, 185, 181, 178, 174, 170, 166, 162, 159, 155, 151, 147, 143, 140, 136, 132, 128, 124, 120, 116, 113, 109, 105, 101, 97, 94, 90, 86, 82, 78, 75, 71, 67, 64, 60, 56, 53, 49, 45, 42, 38, 35, 31, 28, 24, 21, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 8, 7, 6, 6, 5, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 8, 7, 6, 6, 5, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 21, 24, 28, 31, 35, 38, 42, 45, 49, 53, 56, 60, 64, 67, 71, 75, 78, 82, 86, 90, 94, 97, 101, 105, 109, 113, 116, 120, 124};

int currentStepA;
int currentStepB;
int currentStepC;
int sineArraySize;
int increment = 0;
boolean sentidoGiroMotor = true; // direction true=forward, false=backward
int espera_entre_pulsos;
int espera_entre_pulsos_solicitada;
char inChar;

//Timers
uint32_t timer_aceleracion;

//Posicion Prisma
#define M_PARAR 0
#define M_AVANZAR 1
#define M_RETROCEDER 2
#define M_APAGADO_ROBOT 3

int estado_motor;
int comando_solicitado;

//Nombre remitente
#define N_DESCONOCIDO 0
#define N_PRISMA 2222
#define N_RUEDA_IZQUIERDA 3333
#define N_RUEDA_DERECHA 4444

#define PIN_RUEDA 12//Estara conectado a tierra si soy la izquierda


int miNombre;
void pintaAnilloAmarillo()
{
	for(int i=0; i<NUMPIXELS ; i++)
	{
		anillo_rueda.setPixelColor(i, anillo_rueda.Color(255,215,0)); // Moderately bright green color.
	}
	anillo_rueda.setBrightness(200);
	anillo_rueda.show();
}
void InicializarAnillo()
{
	anillo_rueda.begin(); // This initializes the NeoPixel library.
	anillo_rueda.setBrightness(75);
}
void setup() {
#ifdef WINC_EN
  pinMode(WINC_EN, OUTPUT);
  digitalWrite(WINC_EN, HIGH);
#endif
	
  Serial.begin(115200);

  Serial.println("PRISMA ACTIVANDOSE");
  
  InicializarAnillo();
  pintaAnilloAmarillo();
  InicializaWifi();
  
  pinMode(PIN_RUEDA, INPUT_PULLUP);
  
  if (digitalRead(PIN_RUEDA) == LOW)//Conectado a tierra indica rueda izquierda
  {
	  miNombre = N_RUEDA_IZQUIERDA;
	  Serial.println("RUEDA IZQUIERDA");
  }
  else
  {
	 miNombre = N_RUEDA_DERECHA;
	 Serial.println("RUEDA DERECHA");
  }
  
	 //estado_motor = M_PARAR;
	 comando_solicitado = M_APAGADO_ROBOT;
	 estado_motor = M_APAGADO_ROBOT;
	 PintaAnillo();
	 
  	Udp.begin(miNombre);
	delay(3000);
	Serial.println(" UDP ACTIVADO");
	
	IncializarPaqueteJSON();
	
	espera_entre_pulsos=6000;
	espera_entre_pulsos_solicitada = espera_entre_pulsos;
	InicializarMotor();
	
}
void InicializarMotor()
{
	
	pinMode(IN1, OUTPUT);
	pinMode(IN2, OUTPUT);
	pinMode(IN3, OUTPUT);
	
	pinMode(ENABLE, OUTPUT);
	digitalWrite(ENABLE, LOW);
	


	sineArraySize = sizeof(pwmSin)/sizeof(int); // Find lookup table size
	int phaseShift = sineArraySize / 3;         // Find phase shift and initial A, B C phase values
	currentStepA = 0;
	currentStepB = currentStepA + phaseShift;
	currentStepC = currentStepB + phaseShift;

	sineArraySize--; // Convert from array Size to last PWM array number
	
}
void MoverMotor()
{	
	 analogWrite(IN1, pwmSin[currentStepA]);
	 analogWrite(IN2, pwmSin[currentStepB]);
	 analogWrite(IN3, pwmSin[currentStepC]);
	 
	 if (sentidoGiroMotor==true) increment = 1;
	 else increment = -1;

	 currentStepA = currentStepA + increment;
	 currentStepB = currentStepB + increment;
	 currentStepC = currentStepC + increment;

	 //Check for lookup table overflow and return to opposite end if necessary
	 if(currentStepA > sineArraySize)  currentStepA = 0;
	 if(currentStepA < 0)  currentStepA = sineArraySize;
	 
	 if(currentStepB > sineArraySize)  currentStepB = 0;
	 if(currentStepB < 0)  currentStepB = sineArraySize;
	 
	 if(currentStepC > sineArraySize)  currentStepC = 0;
	 if(currentStepC < 0) currentStepC = sineArraySize;
	 
	 /// Control speed by this delay	 
	 delayMicroseconds(espera_entre_pulsos);
}
void IncializarPaqueteJSON()
{
	jsonEnviar["nombre"] = N_DESCONOCIDO;
	jsonEnviar["comando_solicitado"] = M_APAGADO_ROBOT;
	jsonEnviar["esperaEntrePulsos"] = 0;
}
void DecodificaJSON_ComandoMotor(String stringJson)
{
	char t_char_array[200];
	stringJson.toCharArray(t_char_array, 200);
	StaticJsonBuffer<200> bufferRecepcion;

	JsonObject& jsonRecibido = bufferRecepcion.parseObject(t_char_array);

	//const char* nombre = jsonRecibido["nombre"];
	
	//ATENCION !!! SI LA VARIBLE YA ESTA DECLARADA DA ERROR !!! ???
	//int t_remitente = jsonRecibido["nombre"];
	espera_entre_pulsos_solicitada = jsonRecibido["esperaEntrePulsos"];
	comando_solicitado = jsonRecibido["comando_solicitado"];
	Serial.print("COMANDO motor RECIBIDO: ");
	Serial.print(comando_solicitado);
	Serial.print(" ,Espera entre pulsos solicitada: ");
	Serial.println(espera_entre_pulsos_solicitada);
	Serial.println();
	
		
// 	Serial.print("JSON PRISMA RECIBIDO: ");
// 	jsonRecibido.printTo(Serial);
// 	Serial.println();
	
	
	
}
void Acelera()
{
	if(espera_entre_pulsos != espera_entre_pulsos_solicitada)
	{
// 		Serial.print("E: ");
// 		Serial.print(espera_entre_pulsos_solicitada);
// 		Serial.print(" , ");
// 		Serial.println(espera_entre_pulsos);
		
		//Acelero o decelero a la nueva velocidad establecida por espera entre pulsos
		//Con el comando parar no se ve la deceleracion ya que en loop lo para directo
		if(espera_entre_pulsos_solicitada >= espera_entre_pulsos)
		{
			espera_entre_pulsos = espera_entre_pulsos + 20;
		}
		else
		{
			espera_entre_pulsos = espera_entre_pulsos - 20;
		}
		
		
	}
	
}
void GestionaSolicitudes()
{
	if((millis()-timer_aceleracion) > 2)
	{
		timer_aceleracion = millis();
		Acelera();
	}
	
	
	if(comando_solicitado != estado_motor)
	{
		estado_motor = comando_solicitado;
		PintaAnillo();
		switch (estado_motor)
		{
			case M_PARAR:
				digitalWrite(ENABLE, LOW);
				espera_entre_pulsos = espera_entre_pulsos_solicitada;
			break;
			case M_AVANZAR:
			 sentidoGiroMotor = true;
			 digitalWrite(ENABLE, HIGH);
			break;
			case M_RETROCEDER:
			 sentidoGiroMotor = false;
			 digitalWrite(ENABLE, HIGH);
			break;
			
		
			default:
			Serial.println("NO TENDRIA QUE ENTRAR EN DEFAULT SWITCH ESTADO MOTORES");
			break;
		
		}
	}
	
}
void DecodificaJSON(String stringJson)
{
	
	char t_char_array[200];
	stringJson.toCharArray(t_char_array, 200);
	StaticJsonBuffer<200> bufferRecepcion;

	JsonObject& jsonRecibido = bufferRecepcion.parseObject(t_char_array);

	int t_remitente = jsonRecibido["nombre"];
	
	if(t_remitente == N_PRISMA)
		DecodificaJSON_ComandoMotor(stringJson);
	
	
}
void GestionaUDP()
{
	  // if there's data available, read a packet
	  int packetSize = Udp.parsePacket();
	  if (packetSize)
	  {
 		  Serial.print("Tamaño de paquete: ");
 		  Serial.println(packetSize);
 		  Serial.print("From ");
		  IPAddress remoteIp = Udp.remoteIP();
 		  Serial.print(remoteIp);
 		  Serial.print(", puerto ");
		  int puertoRemitente = Udp.remotePort();
		  Serial.println(puertoRemitente);

		  // read the packet into packetBufffer
		  int len = Udp.read(packetBuffer, 255);
		  if (len > 0) packetBuffer[len] = 0;
// 		  Serial.print("Contenido paquete recibido:");
// 		  Serial.println(packetBuffer);
		  
		  String contenido = packetBuffer;
		  
		  
		  //cambia el estado del led
		  if(puertoRemitente == N_PRISMA)
		  {
			  DecodificaJSON(contenido);
			  char  contenidoJson[255];
			  
			  //JSON
			  jsonEnviar["nombre"] = miNombre;
			  jsonEnviar["comando_solicitado"] = comando_solicitado;
			  jsonEnviar["esperaEntrePulsos"] = espera_entre_pulsos;
			  
			  jsonEnviar.printTo(contenidoJson, 255);
			  
			  //Enviar comando al REMITENTE
			  Udp.beginPacket(remoteIp, Udp.remotePort());
			  Udp.write(contenidoJson);
			  Udp.endPacket();
			  
		  }
		 		  
	  }
}
void PintaAnillo()
{
	Serial.print("anillo pintado, estado motor: ");
	Serial.println(estado_motor);
	switch (estado_motor)
	{
		case M_PARAR:
		for(int i=0; i<NUMPIXELS ; i++)
		{
			anillo_rueda.setPixelColor(i, anillo_rueda.Color(255,255,255));
		}
		anillo_rueda.setBrightness(75);
		anillo_rueda.show();
		break;
		case M_AVANZAR:
		for(int i=0; i<NUMPIXELS ; i++)
		{
			anillo_rueda.setPixelColor(i, anillo_rueda.Color(30, 200, 50));
		}
		anillo_rueda.setBrightness(175);
		anillo_rueda.show();
		break;
		case M_RETROCEDER:
		for(int i=0; i<NUMPIXELS ; i++)
		{
			anillo_rueda.setPixelColor(i, anillo_rueda.Color(255,69,0));
		}
		anillo_rueda.setBrightness(225);

		anillo_rueda.show();
		break;
		case M_APAGADO_ROBOT:
		for(int i=0; i<NUMPIXELS ; i++)
		{
			anillo_rueda.setPixelColor(i, anillo_rueda.Color(0,0,0));
		}
		//anillo_rueda.setBrightness(225);
		anillo_rueda.show();
		break;
		
		
		default:
		Serial.println("NO TENDRIA QUE ENTRAR EN DEFAULT SWITCH ESTADO motores Pinta Anillo");
		break;
		
	}
}
void loop() {
	
  //CHEQUEO MI CONEXION A LA WIFI
  if (WiFi.status() == WL_CONNECTED) 
  {
	   GestionaUDP();
	   GestionaSolicitudes();
	   if(estado_motor != M_PARAR
	      && estado_motor != M_APAGADO_ROBOT)//Impide decelerar en estado parar
	   {
		  MoverMotor();
	   }   

  }
  else
  {
	  Serial.println("RECONECTANDO A LA WIFI");
	  pintaAnilloAmarillo();
	  InicializaWifi();
	  estado_motor = M_APAGADO_ROBOT;
	  PintaAnillo();
  }
  
  
}
void InicializaWifi()
{
	
	// attempt to connect to Wifi network:
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print("Attempting to connect to SSID: ");
		Serial.println(ssid);
		// Connect to WPA/WPA2 network. Change this line if using open or WEP network:
		status = WiFi.begin(ssid, pass);

		// wait 10 seconds for connection:
		uint8_t timeout = 10;
		while (timeout && (WiFi.status() != WL_CONNECTED)) {
			timeout--;
			delay(1000);
		}
	}
	
	//Modo automatico de ahorro de energia, reduce hasta un 75% segun medidas de Adafruit
	WiFi.setSleepMode(M2M_PS_H_AUTOMATIC, 1); // go into power save mode when possible!

	Serial.println("Connected to wifi");
	printWifiStatus();
	
	
}
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

