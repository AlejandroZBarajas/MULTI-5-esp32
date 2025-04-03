#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <BH1750.h>


const char* ssid = "INFINITUM1C29";       
const char* password = "maUk4yEP9d"; 

const char* serverUrl = "http://192.164.1.248:8080/events";

const String serie = "id0002";

//PINES temperatura
#define DHTPIN 23       
#define DHTTYPE DHT11 

DHT dht(DHTPIN, DHTTYPE);  

//PINES gyro
const int pinX = 33, pinY = 32, pinZ = 35;
const int umbral = 40;

//PINES luz
#define SDA_PIN 21
#define SCL_PIN 22 
BH1750 lightMeter;


//sonido
const int pinAnalog = 34;
int volumenBase = 0;
unsigned long tiempoInicio = 0; 

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

  pinMode(pinAnalog, INPUT);
  volumenBase = analogRead(pinAnalog);
  
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

void sendPostRequest(String _title, float _description, String _emitter, String _topic) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");

        String jsonPayload = "{\"serie\":\"" + serie + 
                              "\", \"title\":\"" + _title + 
                              "\", \"description\":" + String(_description) + 
                              ", \"emitter\":\"" + _emitter + 
                              "\",\"topic\":\"" + _topic + "\"}";

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
  float _description = temp;
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


    sendPostRequest(_title, _description, _emitter, _topic);

  delay(2000);
}

int leerPromedio(int pin) {
    int suma = 0;
    for (int i = 0; i < 5; i++) {  // Leer 5 veces para suavizar valores
        suma += analogRead(pin);
        delay(10);
    }
    return suma / 5;  // Devolver el promedio
}

void isMoving() {
  String _title = "";
  float _description = 0;
  String _emitter = "GY-61";
  String _topic ="";

  int rawX1 = leerPromedio(pinX);
  int rawY1 = leerPromedio(pinY);
  int rawZ1 = leerPromedio(pinZ);

  delay(500);  

  int rawX2 = leerPromedio(pinX);
  int rawY2 = leerPromedio(pinY);
  int rawZ2 = leerPromedio(pinZ);

  int Xdif = abs(rawX1 - rawX2);
  int Ydif = abs(rawY1 - rawY2);
  int Zdif = abs(rawZ1 - rawZ2);

  Serial.print("Xdif: "); Serial.print(Xdif);
  Serial.print(" | Ydif: "); Serial.print(Ydif);
  Serial.print(" | Zdif: "); Serial.println(Zdif);

  if (Xdif > umbral || Ydif > umbral || Zdif > umbral) {
      Serial.println("¡Movimiento detectado!");
      _title = "¡Alerta! movimiento detectado";
      _topic="alert"; 
      _description = 1;
  } else {
      Serial.println("Sin movimiento.");
      _title = "En calma";
      _topic="notification"; 
      _description = 0;
  }
  sendPostRequest(_title, _description, _emitter, _topic);
}

bool isDark() {
  float lux = lightMeter.readLightLevel();

  String _title = "Hora de dormir. Sistema corriendo.";
  float _description = 1;
  String _emitter = "GY-302";
  String _topic ="alert";

  if (isnan(lux)) {
    Serial.println("Error al leer el sensor de luz!");
    return false;
  }
  
  if(lux < 2){
    Serial.println("Hora de dormir. Sistema corriendo. enviando post");
    sendPostRequest(_title, _description, _emitter, _topic);

  }
  return (lux < 2);  
}

void isCrying() {
  int volumenActual = analogRead(pinAnalog);  
  int diferencia = abs(volumenActual - volumenBase);  

  String _title = "";
  float _description = volumenActual;
  String _emitter = "sonido";
  String _topic ="";
  

/*     Serial.print("Volumen: ");
    Serial.print(volumenActual);
    Serial.print(" | Variación: ");
    Serial.println(diferencia); */

    if (diferencia > 50) {  
        Serial.println("..............................................¡Ruido detectado!");
        _title = "Fuerte sonido detectado";
        _topic="alert";
    } else{
      _title = "Sonido";
      _topic="notification";
    }

    volumenBase = (volumenBase + volumenActual)/2;
    sendPostRequest(_title, _description, _emitter, _topic);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado, intentando reconectar...");
    conectarWiFi();
}
  isTempOk();

  for(int i = 0; i<30; i++){
    
      if(isDark()){  
        Serial.println("corriendo servicio");
        isMoving();
        isCrying();
      }

  }
  delay(500);
}