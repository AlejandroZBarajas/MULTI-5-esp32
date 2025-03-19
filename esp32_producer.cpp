#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

//const char* ssid = "arquitest";       
//const char* password = "arquitest"; 


const char* ssid = "INFINITUM1C29";       
const char* password = "maUk4yEP9d"; 

const char* serverUrl = "http://192.168.1.248:8080/events";

#define DHTPIN 23       
#define DHTTYPE DHT11 

const int pinX = 35; 
const int pinY = 32; 
const int pinZ = 33;

#define sensLight 25

#define wifiLed 13
#define postLed 14
#define tempLed 26
#define alertLed 18

const float sensibilidad = 300.0;

DHT dht(DHTPIN, DHTTYPE);  

int offsetX = 1916;  // Valor en reposo para X
int offsetY = 1855;  // Valor en reposo para Y
int offsetZ = 2307;

void setup() {
  Serial.begin(115200);
  pinMode(tempLed, OUTPUT);
  pinMode(wifiLed, OUTPUT);
  pinMode(postLed, OUTPUT);
  pinMode(alertLed, OUTPUT);

  analogReadResolution(12);  
  analogSetAttenuation(ADC_11db);

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

  String _title = "Temperatura";
  String _description = "Temperatura: " + String(temperature) + " °C, Humedad: " + String(humidity) + "%";
  String _emitter = "DHT11";

  if(isTempOk(temperature)){

    sendPostRequest(_title, _description, _emitter);

  }

  delay(2000);
}

bool isTempOk(float temp){

  String title = "ALERTA TEMPERATURA";
  String description = "";
  String emitter = "DHT11";
  
  if(temp >= 20 && temp <= 24){
    
    digitalWrite(tempLed, LOW);
    return true;
    
  }

  digitalWrite(tempLed, HIGH);

  if(temp <=19 ){
    description = "Temperatura debajo de 20°C";
  } else{
    description = "Temperatura superior a 24°C";
  }
  digitalWrite(alertLed, HIGH);
  sendPostRequest(title, description, emitter);
  return false;
}

void sendPostRequest(String _title, String _description, String _emitter) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");

        String jsonPayload = "{\"title\":\"" + _title + "\", \"description\":\"" + _description + "\", \"emitter\":\"" + _emitter + "\"}";
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

void read_accelerometer() {
    int rawX = analogRead(pinX);
    int rawY = analogRead(pinY);
    int rawZ = analogRead(pinZ);

    // Convertir a valores de aceleración en g
    float accelX = (rawX - offsetX) / sensibilidad;
    float accelY = (rawY - offsetY) / sensibilidad;
    float accelZ = (rawZ - offsetZ) / sensibilidad;

    Serial.print("X: "); Serial.print(accelX, 3); Serial.print(" g | ");
    Serial.print("Y: "); Serial.print(accelY, 3); Serial.print(" g | ");
    Serial.print("Z: "); Serial.print(accelZ, 3); Serial.println(" g");
}

void loop() {

  read_accelerometer();
   if (WiFi.status() != WL_CONNECTED) {
        conectarWiFi();  
    }
  get_temperature();
  delay(1500);
  
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