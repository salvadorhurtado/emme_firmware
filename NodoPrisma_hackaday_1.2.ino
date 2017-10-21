
//ANILLO NEOPIXEL

#include <Wire.h>
#include <Adafruit_NeoPixel.h>

#define PIN            5
#define NUMPIXELS      7

Adafruit_NeoPixel anillo_control = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define PIN_ANILLO_ESTADO            6

Adafruit_NeoPixel anillo_estado = Adafruit_NeoPixel(NUMPIXELS, PIN_ANILLO_ESTADO, NEO_GRB + NEO_KHZ800);

//ACELEROMETRO MMA8451
#include <Adafruit_MMA8451.h>
Adafruit_MMA8451 mma = Adafruit_MMA8451();

//WINC1500
#include <SPI.h>
#include <Adafruit_WINC1500.h>
#include <Adafruit_WINC1500Udp.h>

//WIFI

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

char ssid[] = "emmewifi";//"enred";     //  your network SSID (name)
char pass[] = "muyjuntos";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

unsigned int localPort = 2222;      // local port to listen on

char packetBuffer[255]; //buffer to hold incoming packet

Adafruit_WINC1500UDP Udp;


// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
Adafruit_WINC1500Client client;



//Libreria JSON
#include <ArduinoJson.h>

StaticJsonBuffer<200> jsonBufferEnviar;
JsonObject& jsonEnviar = jsonBufferEnviar.createObject();

//Timers
uint32_t timer_udp_enviado;

//SOLICITUD MOTORES
#define M_PARAR 0
#define M_AVANZAR 1
#define M_RETROCEDER 2
#define M_APAGADO_ROBOT 3
#define M_AVANZAR_RAPIDO 4



//Posicion Prisma
#define P_NEUTRO 0
#define P_DERECHA 1
#define P_IZQUIERDA 2
#define P_DELANTE 3
#define P_DETRAS 4
#define P_DELANTE_RAPIDO 5
#define P_DETRAS_RAPIDO 6


int posicion_prisma;
int posicion_prisma_anterior;

int estado_motor_izquierdo;
int robot_encendido = 0;

//Nombre remitente
#define N_DESCONOCIDO 0
#define N_PRISMA 2222
#define N_RUEDA_IZQUIERDA 3333
#define N_RUEDA_DERECHA 4444
#define N_PRISMA_INTERNO_ESTADO 6666


int miNombre;

#define PIXEL_ESTADO_GENERAL 0
#define PIXEL_TEMPERATURA 1
#define PIXEL_ACELERACION 4
#define PIXEL_DIA_NOCHE 5

#define ESTADO_OK 0
#define ESTADO_ALARMA_ACELERACION 1
#define ESTADO_TEMPERATURA_BAJA 1
#define ESTADO_TEMPERATURA_ALTA 2
#define ESTADO_NOCHE 1

#define NUMERO_ENCENDIDOS 3
#define TIEMPO_AZUL_ENCENDIDO 1000
#define TIEMPO_MARGEN_ENCENDIDO 300
#define ESPERA_EN_ARCOIRIS 250
#define ESPERA_ANTES_APAGAR_ARCOIRIS 2000

// #define NUMERO_ENCENDIDOS 5
// #define TIEMPO_AZUL_ENCENDIDO 100
// #define TIEMPO_MARGEN_ENCENDIDO 500
// #define ESPERA_EN_ARCOIRIS 100
// #define ESPERA_ANTES_APAGAR_ARCOIRIS 100

void inicializarJSON()
{
	jsonEnviar["nombre"] = N_DESCONOCIDO;
	jsonEnviar["comando_solicitado"] = M_PARAR;
	jsonEnviar["esperaEntrePulsos"] = 0;
}
void InicializarAcelerometro()
{
	 if (! mma.begin()) {
		 Serial.println("Couldnt start");
		 while (1);
	 }
	 Serial.println("MMA8451 found!");
	 
	 mma.setRange(MMA8451_RANGE_2_G);
	 
	 Serial.print("Range = "); Serial.print(2 << mma.getRange());
	 Serial.println("G");
}
void InicializarAnillos()
{
	anillo_control.begin(); // This initializes the NeoPixel library.
	anillo_control.setBrightness(200);   
	apagaAnilloControl();                                                                                                                                                
	
	anillo_estado.begin(); // This initializes the NeoPixel library.
	anillo_estado.setBrightness(200);
	apagaAnilloEstado();
// 	pintaEstado(PIXEL_ESTADO_GENERAL, 30, 30, 200);//color azul
// 	pintaEstado(PIXEL_TEMPERATURA,255,69,50);//color naranja
// 	pintaEstado(PIXEL_ACELERACION,30,200,30);//color verde
// 	pintaEstado(PIXEL_DIA_NOCHE,30,200,30);//color verde
// 	
	//anillo_estado.show(); 
}
void pintaEstado(int nombre, int R, int G, int B)
{
	anillo_estado.setPixelColor(nombre, anillo_control.Color(R,G,B)); 
}
void setup() {
	
	#ifdef WINC_EN
	pinMode(WINC_EN, OUTPUT);
	digitalWrite(WINC_EN, HIGH);
	#endif
	
   Serial.begin(9600);
   InicializarAcelerometro();
   InicializarAnillos();
   pintaAnillosRojo();
   InicializaWifi(); 
   
   //apagaAnilloControl();
   //apagaAnilloEstado();
   inicializarJSON();
  
   //Udp.begin(localPort);
   Udp.begin(N_PRISMA);
   delay(1000);//OJO ANTES ERA 3000
   Serial.println("UDP ARRANCADO");
   
   pintaBlanco();
   
   posicion_prisma = P_NEUTRO;
   posicion_prisma_anterior = posicion_prisma;
   
   estado_motor_izquierdo = M_PARAR;
   
   miNombre = N_PRISMA;
  
}
void EnviaPaqueteRueda(int t_nombreRueda, int t_comando_solicitado, int t_esperaEntrePulsos)
{

	  char  contenidoJson[255];
	  
	  //JSON
	  jsonEnviar["nombre"] = N_PRISMA;
	  jsonEnviar["comando_solicitado"] = t_comando_solicitado;
	  jsonEnviar["esperaEntrePulsos"] = t_esperaEntrePulsos;
	  
	  jsonEnviar.printTo(contenidoJson, 255);
	  
	  Serial.print("COMANDO ENVIADO: ");
	  Serial.print(t_comando_solicitado);
	  Serial.print(", Al Motor: ");
	  Serial.println(t_nombreRueda);
	  Serial.println();
	  	 
	 
	IPAddress ip(192, 168, 0, 255);//...., 0, 255) "cero para tplink, 1 para huawei 4g
	Udp.beginPacket(ip, t_nombreRueda);
	Udp.write(contenidoJson);
	Udp.endPacket();
	
	
}
void GestionaUDP()
{
	// if there's data available, read a packet
	int packetSize = Udp.parsePacket();
	if (packetSize)
	{
// 		Serial.print("Received packet of size ");
// 		Serial.println(packetSize);
// 		Serial.print("From ");
		IPAddress remoteIp = Udp.remoteIP();
		Serial.print(remoteIp);
		Serial.print(", port ");
		 int puertoRemitente = Udp.remotePort();
		 Serial.println(puertoRemitente);


		// read the packet into packetBufffer
		int len = Udp.read(packetBuffer, 255);
		if (len > 0) packetBuffer[len] = 0;
// 		Serial.print("Contenido paquete recibido:");
// 		Serial.println(packetBuffer);


		String contenido = packetBuffer;
		
		
		
		if(puertoRemitente == N_PRISMA_INTERNO_ESTADO)
		{
			Serial.println("ESTADO del prisma interno Recibido");
			DecodificaJSON_Estado(contenido);	
		}
 
		 if(puertoRemitente == N_RUEDA_IZQUIERDA
			|| puertoRemitente == N_RUEDA_DERECHA)
		 {
			 //DecodificaJSON(contenido);
			 DecodificaJSON_ComandoMotor(contenido);
		 }
	}
		
}
void DecodificaJSON_ComandoMotor(String stringJson)
{
	char t_char_array[200];
	stringJson.toCharArray(t_char_array, 200);
	StaticJsonBuffer<200> bufferRecepcion;

	JsonObject& jsonRecibido = bufferRecepcion.parseObject(t_char_array);

	//const char* nombre = jsonRecibido["nombre"];
	
	//ATENCION !!! SI LA VARIBLE YA ESTA DECLARADA DA ERROR !!! ???
	int t_comando_solicitado = jsonRecibido["comando_solicitado"];
	Serial.print("COMANDO MOTOR CONFIRMADO: ");
	Serial.println(t_comando_solicitado);
	Serial.println();
	
	
}
void DecodificaJSON_Estado(String stringJson)
{
	char t_char_array[200];
	stringJson.toCharArray(t_char_array, 200);
	StaticJsonBuffer<200> bufferRecepcion;

	JsonObject& jsonRecibido = bufferRecepcion.parseObject(t_char_array);

	int remitente = jsonRecibido["nombre"];
	int estadoTemperatura = jsonRecibido["estadoTemperatura"];
	int estadoAceleracion = jsonRecibido["estadoAceleracion"];
	int estadoDiaNoche = jsonRecibido["estadoDiaNoche"];
	
	if(estadoTemperatura == ESTADO_OK)
	{
		pintaEstado(PIXEL_TEMPERATURA, 0,200, 50);//color verde
	}
	else if (estadoTemperatura == ESTADO_TEMPERATURA_BAJA)
	{
		pintaEstado(PIXEL_TEMPERATURA, 0,30,200);//color azul
	}
	else
	{
		pintaEstado(PIXEL_TEMPERATURA, 200, 30, 30);//color rojo
	}
	
	if(estadoAceleracion == ESTADO_OK)
	{
		pintaEstado(PIXEL_ACELERACION, 30,200, 90);//color verde
	}
	else
	{
		pintaEstado(PIXEL_ACELERACION, 255,69,0);//color rojo
	}
	
	if(estadoAceleracion == ESTADO_OK)
	{
		pintaEstado(PIXEL_DIA_NOCHE, 255, 255, 0);//color amarillo "dia"
	}
	else
	{
		pintaEstado(PIXEL_DIA_NOCHE, 25, 25 , 112);//color azul oscuro "noche"
	}
	
	if(estadoTemperatura > 0
	   || estadoAceleracion > 0
	   || estadoDiaNoche >0)
	   {
		   pintaEstado(PIXEL_ESTADO_GENERAL, 255,69,0); //naranja
	   }
	else
	{
		pintaEstado(PIXEL_ESTADO_GENERAL, 30,200,90); //verde
	}
	
	
	
	anillo_estado.show(); 
	
	Serial.print("RECIBIDO ESTADO, estado temperatura: ");
	Serial.print(estadoTemperatura);
	Serial.print(" ,estado Aceleracion: ");
	Serial.print(estadoAceleracion);
	Serial.print(" ,estado D�a Noche: ");
	Serial.print(estadoDiaNoche);
	
	Serial.println();
	
	
}
void pintaAnillosRojo()
{
	for(int i=0; i<NUMPIXELS ; i++)
	{
		anillo_control.setPixelColor(i, anillo_control.Color(255,0,0)); // Moderately bright green color.
	}
	anillo_control.setBrightness(200);
	anillo_control.show();
	
	for(int i=0; i<NUMPIXELS ; i++)
	{
		anillo_estado.setPixelColor(i, anillo_estado.Color(255,0,0)); // Moderately bright green color.
	}
	anillo_estado.setBrightness(200);
	anillo_estado.show();
}
void apagaAnilloControl()
{
	for(int i=0; i<NUMPIXELS ; i++)
	{
		anillo_control.setPixelColor(i, anillo_control.Color(0,0,0)); 
		}
	anillo_control.show();
}
void pintaAnillosAzul()
{
	for(int i=0; i<NUMPIXELS ; i++)
	{
		anillo_control.setPixelColor(i, anillo_control.Color(30,144,255));
	}
	anillo_control.setBrightness(200);
	anillo_control.show();
	
	for(int i=0; i<NUMPIXELS ; i++)
	{
		anillo_estado.setPixelColor(i, anillo_estado.Color(30,144,255));
	}
	anillo_estado.setBrightness(200);
	anillo_estado.show();
}
void pintaAnilloEstadoRojo()
{
	for(int i=0; i<NUMPIXELS ; i++)
	{
		anillo_estado.setPixelColor(i, anillo_estado.Color(255,0,0)); // Moderately bright green color.
	}
	anillo_estado.setBrightness(200);
	anillo_estado.show();
}
void apagaAnilloEstado()
{
	for(int i=0; i<NUMPIXELS ; i++)
	{
		anillo_estado.setPixelColor(i, anillo_estado.Color(0,0,0));
	}
	anillo_estado.show();
}
void pintaBlanco()
{
	for(int i=0; i<NUMPIXELS ; i++)
	{
		anillo_estado.setPixelColor(i, anillo_estado.Color(255,255,255));
	}
	anillo_estado.show();
	
	for(int i=0; i<NUMPIXELS ; i++)
	{
		anillo_control.setPixelColor(i, anillo_control.Color(255,255,255));
	}
	anillo_control.show();
}
void GestionaAcelerometro()
{
	
	 mma.read();
// 	 Serial.print("X:\t"); Serial.print(mma.x);
// 	 Serial.print("\tY:\t"); Serial.print(mma.y);
// 	 Serial.print("\tZ:\t"); Serial.print(mma.z);
	 //Serial.println();

	 //TIENE PRIORIDAD GIRAR SOBRE AVANZAR O RETROCEDER, POR ESO la variable
	 //estado_motores_propuesto se sobre-escribe

	posicion_prisma = P_NEUTRO;

	 if(mma.x <= -1500)
	 {
		 //delante
		 posicion_prisma = P_DELANTE;
		 anillo_control.setPixelColor(1, anillo_control.Color(200,0,69)); // Moderately bright green color.
		 anillo_control.setPixelColor(4, anillo_control.Color(0,0,0)); // Moderately bright green color.

			if(mma.x <= -3000)
			{
				//delante
				posicion_prisma = P_DELANTE_RAPIDO ;
				anillo_control.setPixelColor(1, anillo_control.Color(69,0,200)); // Moderately bright green color.
				anillo_control.setPixelColor(4, anillo_control.Color(0,0,0)); // Moderately bright green color.
				//Serial.println("RAPIDO DELANTE");

			}
	 }
	 else if(mma.x >= 1500)
	 {
		 posicion_prisma = P_DETRAS;
		 anillo_control.setPixelColor(1, anillo_control.Color(0,0,0)); // Moderately bright green color.
		 anillo_control.setPixelColor(4, anillo_control.Color(200,0,69)); // Moderately bright green color.
		
		

	 }
	 else
	 {
		 
		 anillo_control.setPixelColor(1, anillo_control.Color(0,0,0)); // Moderately bright green color.

		 anillo_control.setPixelColor(4, anillo_control.Color(0,0,0)); // Moderately bright green color.

	 }

	 if(mma.y <= -1500)
	 {
		 //Izquierda
		 posicion_prisma = P_IZQUIERDA;
		 anillo_control.setPixelColor(5, anillo_control.Color(200,0,69)); // Moderately bright green color.
		 anillo_control.setPixelColor(6, anillo_control.Color(200,0,69)); // Moderately bright green color.
		 
		 anillo_control.setPixelColor(2, anillo_control.Color(0,0,0)); // Moderately bright green color.
		 anillo_control.setPixelColor(3, anillo_control.Color(0,0,0)); // Moderately bright green color.

	 }
	 else if(mma.y >= 1500)
	 {
		 posicion_prisma = P_DERECHA;
		 anillo_control.setPixelColor(5, anillo_control.Color(0,0,0)); // Moderately bright green color.
		 anillo_control.setPixelColor(6, anillo_control.Color(0,0,0)); // Moderately bright green color.

		 anillo_control.setPixelColor(2, anillo_control.Color(200,0,69)); // Moderately bright green color.
		 anillo_control.setPixelColor(3, anillo_control.Color(200,0,69)); // Moderately bright green color.

	 }
	 else
	 {
		 anillo_control.setPixelColor(5, anillo_control.Color(0,0,0)); // Moderately bright green color.
		 anillo_control.setPixelColor(6, anillo_control.Color(0,0,0)); // Moderately bright green color.

		 anillo_control.setPixelColor(2, anillo_control.Color(0,0,0)); // Moderately bright green color.
		 anillo_control.setPixelColor(3, anillo_control.Color(0,0,0)); // Moderately bright green color.

	 }

	 if(robot_encendido < NUMERO_ENCENDIDOS)//HE MOVIDO EL ACELEROMETRO
	 {
		if(posicion_prisma > P_NEUTRO)
		{
			robot_encendido = robot_encendido + 1;
		
			ParpadeoAzul();
			posicion_prisma = P_NEUTRO; //Para que este comando no tenga efecto
		
			if(robot_encendido == NUMERO_ENCENDIDOS)
			{
				SecuenciaEncendido();
				EnviaPaqueteRueda(N_RUEDA_IZQUIERDA, M_PARAR, 6000);
				EnviaPaqueteRueda(N_RUEDA_DERECHA, M_PARAR, 6000);
			}
		
		}
		
	 }
	 else
	 {
		 if(posicion_prisma == P_NEUTRO)
		 {
			//ANILLO CONTROL Azul
			for(int i=0; i<NUMPIXELS ; i++)
			{
				anillo_control.setPixelColor(i, anillo_control.Color(20,20,255));
			}
			anillo_control.setBrightness(200);
			anillo_control.show();
			
			//ANILLO CONTROL Azul
			for(int i=0; i<NUMPIXELS ; i++)
			{
				anillo_estado.setPixelColor(i, anillo_estado.Color(20,20,255));
			}
			anillo_estado.setBrightness(200);
			anillo_estado.show();
		 }
		 else
		 {
			 anillo_control.show(); // This sends the updated pixel color to the hardware.

		 }
	 }
	 
	 
}
void ParpadeoAzul()
{
	//Pinto Azul
	pintaAnillosAzul();
	
	// me espero 2 segundos
	if(robot_encendido <3)
	{
		delay(TIEMPO_MARGEN_ENCENDIDO);
		pintaBlanco();
		pintaBlanco();
		delay (TIEMPO_MARGEN_ENCENDIDO);
	}
	
	else
	delay(TIEMPO_AZUL_ENCENDIDO);
	
	//pinto de blanco
	pintaBlanco();
	pintaBlanco();
}
void SecuenciaEncendido()
{
		apagaAnilloEstado();
		apagaAnilloControl();
		anillo_estado.show();
		anillo_control.setBrightness(200);
		anillo_estado.setBrightness(200);
		
		anillo_control.setPixelColor(1, anillo_control.Color(255,0,69));
		anillo_control.show();
		anillo_estado.setPixelColor(4, anillo_estado.Color(255,0,69));
		anillo_estado.show();
		delay(ESPERA_EN_ARCOIRIS);
		
		anillo_control.setPixelColor(2, anillo_control.Color(69,0,255));
		anillo_control.show();
		anillo_estado.setPixelColor(5, anillo_estado.Color(69,0,255));
		anillo_estado.show();
		delay(ESPERA_EN_ARCOIRIS);
		
		anillo_control.setPixelColor(3, anillo_control.Color(0,100,100));
		anillo_control.show();
		anillo_estado.setPixelColor(6, anillo_estado.Color(0,100,100));
		anillo_estado.show();
		delay(ESPERA_EN_ARCOIRIS);
		
		anillo_control.setPixelColor(4, anillo_control.Color(69,100,0));
		anillo_control.show();
		anillo_estado.setPixelColor(1, anillo_estado.Color(69,100,0));
		anillo_estado.show();
		delay(ESPERA_EN_ARCOIRIS);
		
		anillo_control.setPixelColor(5, anillo_control.Color(0,255,69));
		anillo_control.show();
		anillo_estado.setPixelColor(2, anillo_estado.Color(0,255,69));
		anillo_estado.show();
		delay(ESPERA_EN_ARCOIRIS);
		
		anillo_control.setPixelColor(6, anillo_control.Color(100,69,150));
		anillo_control.show();
		anillo_estado.setPixelColor(3, anillo_estado.Color(100,69,150));
		anillo_estado.show();
		delay(ESPERA_EN_ARCOIRIS);
		
		delay(ESPERA_ANTES_APAGAR_ARCOIRIS);
		
		anillo_control.setPixelColor(1, anillo_control.Color(0,0,0));
		anillo_control.show();
		anillo_estado.setPixelColor(1, anillo_estado.Color(0,0,0));
		anillo_estado.show();
		delay(ESPERA_EN_ARCOIRIS);
		
		anillo_control.setPixelColor(2, anillo_control.Color(0,0,0));
		anillo_control.show();
		anillo_estado.setPixelColor(2, anillo_estado.Color(0,0,0));
		anillo_estado.show();
		delay(ESPERA_EN_ARCOIRIS);
		
		anillo_control.setPixelColor(3, anillo_control.Color(0,0,0));
		anillo_control.show();
		anillo_estado.setPixelColor(3, anillo_estado.Color(0,0,0));
		anillo_estado.show();
		delay(ESPERA_EN_ARCOIRIS);
		
		anillo_control.setPixelColor(4, anillo_control.Color(0,0,0));
		anillo_control.show();
		anillo_estado.setPixelColor(4, anillo_estado.Color(0,0,0));
		anillo_estado.show();
		delay(ESPERA_EN_ARCOIRIS);
		
		anillo_control.setPixelColor(5, anillo_control.Color(0,0,0));
		anillo_control.show();
		anillo_estado.setPixelColor(5, anillo_estado.Color(0,0,0));
		anillo_estado.show();
		delay(ESPERA_EN_ARCOIRIS);
		
		anillo_control.setPixelColor(6, anillo_control.Color(0,0,0));
		anillo_control.show();
		anillo_estado.setPixelColor(6, anillo_estado.Color(0,0,0));
		anillo_estado.show();
		
		
}
void GestionaEnvioPaqueteMotores()
{
	if(posicion_prisma_anterior != posicion_prisma)
	{
		posicion_prisma_anterior = posicion_prisma;
		switch (posicion_prisma)
		{
			case P_NEUTRO:
			if(estado_motor_izquierdo != M_PARAR)
			{
				estado_motor_izquierdo = M_PARAR;
				EnviaPaqueteRueda(N_RUEDA_IZQUIERDA, M_PARAR, 6000);
				EnviaPaqueteRueda(N_RUEDA_DERECHA, M_PARAR, 6000);
			
			}
			break;
			case P_DELANTE:
			if(estado_motor_izquierdo != M_AVANZAR)
			{
				estado_motor_izquierdo = M_AVANZAR;
				EnviaPaqueteRueda(N_RUEDA_IZQUIERDA, M_AVANZAR, 300);
				EnviaPaqueteRueda(N_RUEDA_DERECHA, M_RETROCEDER, 300);

			}
			break;
			case P_DELANTE_RAPIDO :
			if(estado_motor_izquierdo != M_AVANZAR_RAPIDO)
			{
				estado_motor_izquierdo = M_AVANZAR_RAPIDO;
				EnviaPaqueteRueda(N_RUEDA_IZQUIERDA, M_AVANZAR, 200);
				EnviaPaqueteRueda(N_RUEDA_DERECHA, M_RETROCEDER, 200);

			}
			break;
			case P_DETRAS:
			if(estado_motor_izquierdo != M_RETROCEDER)
			{
				estado_motor_izquierdo = M_RETROCEDER;
				EnviaPaqueteRueda(N_RUEDA_IZQUIERDA, M_RETROCEDER , 450);
				EnviaPaqueteRueda(N_RUEDA_DERECHA, M_AVANZAR, 450);

			}
			break;
			case P_DERECHA:
			if(estado_motor_izquierdo != M_AVANZAR)
			{
				estado_motor_izquierdo = M_AVANZAR;
				EnviaPaqueteRueda(N_RUEDA_IZQUIERDA, M_AVANZAR , 600);
				EnviaPaqueteRueda(N_RUEDA_DERECHA, M_AVANZAR, 600);

			}
			break;
			case P_IZQUIERDA:
			if(estado_motor_izquierdo != M_RETROCEDER)
			{
				estado_motor_izquierdo = M_RETROCEDER;
				EnviaPaqueteRueda(N_RUEDA_IZQUIERDA, M_RETROCEDER , 600);
				EnviaPaqueteRueda(N_RUEDA_DERECHA, M_RETROCEDER, 600);

			}			
			break;
			
		
			default:
			Serial.println("NO TENDIRA QUE ENTRAR EN EL DEFAULT DE SWITCH POSICION  PRISMA");
			//EnviaPaqueteRueda(estado_motores, 15000);
			break;
		
		}
	}
}
void loop() {

//CHEQUEO MI CONEXION A LA WIFI
  if (WiFi.status() == WL_CONNECTED) 
  {
	   GestionaUDP();  
	   GestionaAcelerometro(); 
	   if((millis()-timer_udp_enviado) > 100)
	   {
			timer_udp_enviado = millis();
			//solo envio paquetes despues del encendido
			if(robot_encendido >= NUMERO_ENCENDIDOS)
				GestionaEnvioPaqueteMotores();				
	   }
  }
  else
  {
	  Serial.println("RECONECTANDO A LA WIFI");
	  InicializaWifi();
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
	IPAddress ip = WiFi.localIP();
	Serial.print("IP Address: ");
	Serial.println(ip);

	// print the received signal strength:
	long rssi = WiFi.RSSI();
	Serial.print("signal strength (RSSI):");
	Serial.print(rssi);
	Serial.println(" dBm");
}



