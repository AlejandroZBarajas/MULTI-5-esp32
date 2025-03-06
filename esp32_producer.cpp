#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "INFINITUM1C29";       
const char* password = "maUk4yEP9d"; 

const char* serverUrl = "http://192.168.1.244:8080/events"; 
const int pirPin = 13; 
bool movimientoDetectado = false;

void setup() {
    Serial.begin(115200);
    pinMode(pirPin, INPUT);

    conectarWiFi();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        conectarWiFi();
    }

    int estadoPIR = digitalRead(pirPin);

    if (estadoPIR == HIGH && !movimientoDetectado) {
        Serial.println("Movimiento detectado, enviando POST...");
        sendPostRequest();
        movimientoDetectado = true;
        delay(5000); 
    } 
    else if (estadoPIR == LOW) {
        movimientoDetectado = false;
    }

    delay(500); // Pequeño retraso para evitar lecturas erráticas
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

void sendPostRequest() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");

        String jsonPayload = "{\"Device_name\":\"ESP32-PIR\", \"message\":\"Movimiento detectado\"}";
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
