#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <BH1750.h>


const char* ssid = "INFINITUM1C29";       
const char* password = "maUk4yEP9d"; 

const char* serverUrl = "http://192.168.1.248:8080/events";

const String serie = "id0002";

//PINES temperatura
#define DHTPIN 23       
#define DHTTYPE DHT11 

DHT dht(DHTPIN, DHTTYPE);  

//PINES gyro
const int pinX = 33, pinY = 32, pinZ = 35;

//PINES luz
#define SDA_PIN 21
#define SCL_PIN 22 
BH1750 lightMeter;


//sonido
const int pinSonido = 5, umbralEventos = 5, ventanaTiempo = 2000; 
unsigned long tiempoInicio = 0;
int detecciones = 0;

//leds
#define wifiLed 13
#define postLed 14
#define tempLed 4
#define alertLed 18
#define soundLed 26


void setup() {
  Serial.begin(115200);
  //leds
  pinMode(tempLed, OUTPUT);
  pinMode(wifiLed, OUTPUT);
  pinMode(postLed, OUTPUT);
  pinMode(alertLed, OUTPUT);
  pinMode(soundLed, OUTPUT);

  pinMode(pinSonido, INPUT);
  
  Wire.begin(SDA_PIN, SCL_PIN);
  dht.begin();
  lightMeter.begin();
  
  
  tiempoInicio = millis();

  conectarWiFi();  
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

void sendPostRequest(String _title, String _description, String _emitter, String _topic) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");

        String jsonPayload = "{\"serie\":\"" + serie + "\", \"title\":\"" + _title + "\", \"description\":\"" + _description + "\", \"emitter\":\"" + _emitter + "\",\"topic\":\"" + _topic + "\"}";
        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
            Serial.print("Respuesta HTTP: ");
            Serial.println(httpResponseCode);
            digitalWrite(postLed, HIGH);
            delay(900);
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


void isTempOk(){
  float temp = dht.readTemperature();

  if (isnan(temp)) {
    Serial.println("Error al leer el sensor DHT!");
    return;
  }

  String _title = "";
  String _description = "";
  String _emitter = "DHT11";
  String _topic ="";

  Serial.println(temp);

  if(temp  >= 20 && temp <= 27){
    _title = "Temperature";
    _topic="notification";
  }
  if(temp <=19 ){
    _title = "Temperature baja";
    _topic="alert";
  }
  if (temp >= 28){
    _title = "Temperatura Alta";
    _topic="alert";   
  }
    _description = String(temp);      
    sendPostRequest(_title, _description, _emitter, _topic);

  delay(2000);
}

void isMoving() {
    
    int rawX1 = analogRead(pinX);
    int rawY1 = analogRead(pinY);
    int rawZ1 = analogRead(pinZ);
    
    const float umbral = 0.02;  // Ajusta según la sensibilidad del acelerómetro

    delay(500);  

    int rawX2 = analogRead(pinX);
    int rawY2 = analogRead(pinY);
    int rawZ2 = analogRead(pinZ);

    float Xdif = abs(rawX1 - rawX2);
    float Ydif = abs(rawY1 - rawY2);
    float Zdif = abs(rawZ1 - rawZ2);

    if (Xdif > umbral || Ydif > umbral || Zdif > umbral) {
        Serial.println("¡Movimiento detectado!");
    } else {
        Serial.println("Sin movimiento.");
    }
}

bool isDark() {
    float lux = lightMeter.readLightLevel();
    return (lux < 2);  
}

void isCrying() {
    int volumen = analogRead(pinSonido);  
    Serial.print("Volumen detectado: ");
    Serial.println(volumen);
    delay(100);
}

void loop() {
  
  if(isDark()){  
    Serial.println("corriendo servicio");
      isMoving();
      isCrying();
      isTempOk();
  }

  delay(800);

}