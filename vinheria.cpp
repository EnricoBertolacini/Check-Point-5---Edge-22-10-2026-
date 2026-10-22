
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define NUM     "077"
#define DEVICE  "device" NUM

const char* SSID     = "FIAP-IOT";
const char* PASSWORD = "F!@p25.IOT";
// Broker MQTT
const char* BROKER_MQTT = "3.135.234.101";
const int   BROKER_PORT = 1883;
const char* ID_MQTT     = "fiware_" NUM;
// Topicos
const char* TOPICO_SUBSCRIBE = "/TEF/" DEVICE "/cmd";       
const char* TOPICO_ESTADO    = "/TEF/" DEVICE "/attrs";    
const char* TOPICO_TEMP      = "/TEF/" DEVICE "/attrs/t";   
const char* TOPICO_UMID      = "/TEF/" DEVICE "/attrs/h";   
const char* TOPICO_LUZ       = "/TEF/" DEVICE "/attrs/p";   
// Pinos
#define DHTPIN  15
#define DHTTYPE DHT11
#define LDRPIN  34
#define LEDPIN  2
const unsigned long INTERVALO = 5000;
unsigned long ultimoEnvio = 0;
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient MQTT(espClient);
char EstadoSaida = '0';
void reconectWiFi() {
 if (WiFi.status() == WL_CONNECTED) return;
 Serial.print("Conectando na rede: ");
 Serial.println(SSID);
 WiFi.begin(SSID, PASSWORD);
 while (WiFi.status() != WL_CONNECTED) {
   delay(100);
   Serial.print(".");
 }
 Serial.println();
 Serial.print("Conectado! IP: ");
 Serial.println(WiFi.localIP());
}
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
 String msg;
 for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
 msg.trim();
 Serial.print("- Topico: ");
 Serial.print(topic);
 Serial.print(" | Mensagem: [");
 Serial.print(msg);
 Serial.println("]");
 if (msg.indexOf("@on") >= 0) {
   digitalWrite(LEDPIN, HIGH);
   EstadoSaida = '1';
   MQTT.publish(TOPICO_ESTADO, "s|on");
 }
 if (msg.indexOf("@off") >= 0) {
   digitalWrite(LEDPIN, LOW);
   EstadoSaida = '0';
   MQTT.publish(TOPICO_ESTADO, "s|off");
 }
}
void reconnectMQTT() {
 while (!MQTT.connected()) {
   Serial.print("* Conectando ao Broker MQTT: ");
   Serial.println(BROKER_MQTT);
   if (MQTT.connect(ID_MQTT)) {
     Serial.println("Conectado ao broker MQTT!");
     MQTT.subscribe(TOPICO_SUBSCRIBE);
     MQTT.publish(TOPICO_ESTADO, EstadoSaida == '1' ? "s|on" : "s|off");
   } else {
     Serial.print("Falha, rc=");
     Serial.print(MQTT.state());
     Serial.println(" - nova tentativa em 2s");
     delay(2000);
   }
 }
}
void enviaSensores() {
 float temperatura = dht.readTemperature();
 float umidade     = dht.readHumidity();
 int   luz         = map(analogRead(LDRPIN), 0, 4095, 0, 100);
 if (isnan(temperatura) || isnan(umidade)) {
   Serial.println("Falha na leitura do DHT11");
 } else {
   MQTT.publish(TOPICO_TEMP, String(temperatura, 1).c_str());
   MQTT.publish(TOPICO_UMID, String(umidade, 1).c_str());
 }
 MQTT.publish(TOPICO_LUZ, String(luz).c_str());
 MQTT.publish(TOPICO_ESTADO, EstadoSaida == '1' ? "s|on" : "s|off");
 Serial.printf("Temp: %.1f C | Umid: %.1f %% | Luz: %d %% | LED: %s\n",
               temperatura, umidade, luz, EstadoSaida == '1' ? "on" : "off");
}
void setup() {
 Serial.begin(115200);
 pinMode(LEDPIN, OUTPUT);
 digitalWrite(LEDPIN, LOW);
 analogReadResolution(12);
 dht.begin();
 reconectWiFi();
 MQTT.setServer(BROKER_MQTT, BROKER_PORT);
 MQTT.setCallback(mqtt_callback);
}
void loop() {
 reconectWiFi();
 if (!MQTT.connected()) reconnectMQTT();
 MQTT.loop();
 if (millis() - ultimoEnvio >= INTERVALO) {
   ultimoEnvio = millis();
   enviaSensores();
 }
}