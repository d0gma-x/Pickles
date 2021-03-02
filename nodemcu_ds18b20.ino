#include <OneWire.h>
#include <DallasTemperature.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>

//const int oneWireBus = 4;
OneWire ourWire(0);
DallasTemperature sensors(&ourWire);
float t;

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
  t = sensors.getTempCByIndex(0); //Se obtiene la temperatura en ºC

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
    client.println(timeClient.getFormattedTime());
    client.println("</body> </html>");
  }
  delay(1000);
}
