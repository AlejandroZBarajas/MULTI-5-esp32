#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "INFINITUM1C29";       
const char* password = "maUk4yEP9d"; 

const char* serverUrl = "http://192.168.1.244:8080/events";

#define DHTPIN 23       
#define DHTTYPE DHT11  

#define wifiLed 13
#define postLed 14
#define tempLed 26


DHT dht(DHTPIN, DHTTYPE);  

void setup() {
  Serial.begin(115200);
  pinMode(tempLed, OUTPUT);
  pinMode(wifiLed, OUTPUT);
  pinMode(postLed, OUTPUT);

  conectarWiFi();
  
  dht.begin();
  
  Serial.println("Iniciando sensor...");
  delay(2000);
}

void conectarWiFi() {
    Serial.print("Conectando a WiFi...");
    WiFi.begin(ssid, password);
    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 20) {
        delay(500);
        Serial.print(".");
        intentos++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConectado a WiFi");
        digitalWrite(wifiLed, HIGH);
    } else {
        Serial.println("\nNo se pudo conectar a WiFi");
        digitalWrite(wifiLed, LOW);
    }
}

void get_temperature(){
  float temperature = dht.readTemperature();

  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Error al leer el sensor DHT!");
    return;
  }

  Serial.print("Temperatura: ");
  Serial.print(temperature);
  Serial.print(" °C ");
  Serial.print("Humedad: ");
  Serial.print(humidity);
  Serial.println(" %");

  isTempOk(temperature);

  sendPostRequest(temperature, humidity);
  
  delay(2000);
}

bool isTempOk(bool temp){
  
  if(temp >= 20 && temp <= 24){
    
    digitalWrite(tempLed, LOW);
    return true;
    
  }
  digitalWrite(tempLed, HIGH);
  return false;
}

void sendPostRequest(float temperature, float humidity) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");

        String jsonPayload = "{\"title\":\"Temperatura\", \"description\":\"Temperatura: " + String (temperature) + "°C, Humedad: " + String(humidity) + " %\", \"emitter\":\"DHT11\"}";
        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
            Serial.print("Respuesta HTTP: ");
            Serial.println(httpResponseCode);
            digitalWrite(postLed, HIGH);
            delay(800);
            digitalWrite(postLed, LOW);
        } else {
            Serial.print("Error en la solicitud: ");
            Serial.println(http.errorToString(httpResponseCode).c_str());
        }

        http.end();
    } else {
        Serial.println("No conectado a WiFi, intentando reconectar...");
        conectarWiFi();
    }
}

void loop() {
  conectarWiFi();
  get_temperature();
  
}



/* #include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "INFINITUM1C29";       
const char* password = "maUk4yEP9d"; 

const char* serverUrl = "http://192.168.1.244:8080/events";

#define DHTPIN 23       
#define DHTTYPE DHT11  

DHT dht(DHTPIN, DHTTYPE);  

void setup() {
  Serial.begin(115200);

  conectarWiFi();
  
  dht.begin();
  
  Serial.println("Iniciando sensor...");
  delay(2000);
}

void conectarWiFi() {
    Serial.print("Conectando a WiFi...");
    WiFi.begin(ssid, password);
    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 20) {
        delay(500);
        Serial.print(".");
        intentos++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConectado a WiFi");
    } else {
        Serial.println("\nNo se pudo conectar a WiFi");
    }
}

void get_temperature(){
  float temperature = dht.readTemperature();

  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Error al leer el sensor DHT!");
    return;
  }

  Serial.print("Temperatura: ");
  Serial.print(temperature);
  Serial.print(" °C ");
  Serial.print("Humedad: ");
  Serial.print(humidity);
  Serial.println(" %");

  sendPostRequest(temperature, humidity);
  delay(2000);
}

void sendPostRequest(float temperature, float humidity) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");

        String jsonPayload = "{\"title\":\"Temperatura\", \"description\":\"Temperatura: " + String (temperature) + "°C, Humedad: " + String(humidity) + " %\", \"emitter\":\"DHT11\"}";
        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
            Serial.print("Respuesta HTTP: ");
            Serial.println(httpResponseCode);
        } else {
            Serial.print("Error en la solicitud: ");
            Serial.println(http.errorToString(httpResponseCode).c_str());
        }

        http.end();
    } else {
        Serial.println("No conectado a WiFi, intentando reconectar...");
        conectarWiFi();
    }
}

void loop() {
  conectarWiFi();
  get_temperature();
  
}
 */