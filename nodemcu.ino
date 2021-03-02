#include <Adafruit_BMP085.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include<SH1106.h>

//const int oneWireBus = 4;
OneWire ourWire(0);
DallasTemperature sensors(&ourWire);
float t_w;
float t_a, p_pascal, p_a;
Adafruit_BMP085 bmp;
SH1106 display(0x3c, SDA, SCL);

const char* ssid = "*****";
const char* password = "*****";
// Puerto (se suele usar 80 por defecto, pero en algunos casos hay que
// consultar al proveedor de internet de cual dispone en su conexion)
WiFiServer server(80);
//iniciamos el cliente udp para su uso con el server NTP
WiFiUDP ntpUDP;
// cuando creamos el cliente NTP podemos especificar el servidor al
// que nos vamos a conectar en este caso
// 0.south-america.pool.ntp.org SudAmerica
// también podemos especificar el offset en segundos para que nos
// muestre la hora según nuestra zona horaria en este caso
// restamos -10800 segundos ya que estoy en chile.
// y por ultimo especificamos el intervalo de actualización en
// mili segundos en este caso 5000
NTPClient timeClient(ntpUDP, "0.south-america.pool.ntp.org", -10800, 5000);

void setup() {

  Serial.begin(115200);
  sensors.begin();
  Wire.begin();
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  if (!bmp.begin()) { // Initicializa el sensor
    // Error en la detección del sensor
    Serial.print("Ooops, BMP085 no detectado ... Comprueba las conexiones!");
    while (1); // Bucle infinito
  }

  delay(100); // breve espera para que se concrete la conexion

  // Conexion al WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
    delay(100); //aqui debemos esperar unos instantes
  Serial.println("Conexión exitosa a WiFi");
  // Iniciar el servidor
  server.begin();
  Serial.println("Iniciado el servidor");

  // Escribir la direccion IP
  Serial.print("el IP es: ");
  Serial.print(WiFi.localIP());
  Serial.println("");
  timeClient.begin();
}

void loop() {

  sensors.requestTemperatures();   //Se envía el comando para leer la temperatura
  t_w = sensors.getTempCByIndex(0); //Se obtiene la temperatura en ºC
  t_a = bmp.readTemperature ();
  p_pascal = bmp.readPressure (); //resultado en pascales
  p_a = (p_pascal / 100); //resultado en hectopascales

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  String tw = "t_w: " + String(t_w) + " *C";
  String ta = "t_a: " + String(t_a) + " *C";
  String pa = "p_a: " + String(p_a) + " hPa";
  //display.drawString(0, 0, "ip: 192.168.100.21");
  display.drawString(0, 10, tw);
  display.drawString(0, 20, ta);  
  display.drawString(0, 30, pa);
  display.display();
  
  timeClient.update(); //sincronizamos con el server NTP
  // Comprobar si se ha conectado algun cliente
  WiFiClient client = server.available();

  if (client) {
    Serial.println("Nueva conexion");
    // Escribir las cabeceras HTML
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");
    client.println("<!DOCTYPE HTML>");

    //Escribir el contenido que vamos a visualizar
    client.println("<html> <head> <title>");
    client.println("Control Acuario");
    client.println("</title> <meta http-equiv=\"refresh\" content=\"5\"> </head> <br/>");
    client.println("<body>");
    client.print("t_w: ");
    client.println(t_w);
    client.print("t_a: ");
    client.println(t_a);
    client.print("p_a: ");
    client.println(p_a);
    client.print("time: ");
    client.println(timeClient.getFormattedTime());
    client.println("</body> </html>");
  }
  delay(1000);
}