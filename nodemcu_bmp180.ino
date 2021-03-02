#include <Adafruit_BMP085.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>

Adafruit_BMP085 bmp;

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
  Wire.begin();

  if (!bmp.begin()) { // Initicializa el sensor
    // Error en la detección del sensor
    Serial.print("ERROR BMP");
    while (1); // Bucle infinito
  }

  delay(100); // breve espera para que se concrete la conexion

  // Conexion al WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
    delay(100); //aqui debemos esperar unos instantes
  Serial.println("Conexión exitosa a WiFi");
  // Iniciar el servidor
  Serial.println("Iniciado el servidor");

  // Escribir la direccion IP
  Serial.print("el IP es: ");
  Serial.print(WiFi.localIP());
  Serial.println("");
  timeClient.begin();
}

void loop() {
  float t, p, a;

  t = bmp.readTemperature ();
  p = bmp.readPressure ();
  a = bmp.readAltitude (103000);
  //Serial.println(t);
  //Serial.println(p);
  //Serial.println(p);
  
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
    client.print("Temp: ");
    client.println(t);
    client.print("Press: ");
    client.println(p);
    client.print("time: ");
    client.println(timeClient.getFormattedTime());
    client.println("</body> </html>");
  }
  delay(1000);
}
