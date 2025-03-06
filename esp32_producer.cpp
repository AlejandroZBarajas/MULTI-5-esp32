#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "INFINITUM1C29";       
const char* password = "maUk4yEP9d"; 


const char* serverUrl = "http://192.168.1.244:8080/events";  

void setup() {
    Serial.begin(115200);
    
    WiFi.begin(ssid, password);
    Serial.print("Conectando a WiFi...");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    
    Serial.println("\nConectado a WiFi con IP: " + WiFi.localIP().toString());
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) { 
        HTTPClient http;
        
        Serial.println("Enviando POST...");
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");

        String jsonPayload = "{\"Device_name\":\"ESP32\"}";

        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
            Serial.print("Código de respuesta: ");
            Serial.println(httpResponseCode);
            String response = http.getString();
            Serial.println("Respuesta del servidor: " + response);
        } else {
            Serial.print("Error en la solicitud: ");
            Serial.println(httpResponseCode);
        }

        http.end(); 
    } else {
        Serial.println("WiFi no conectado, reintentando...");
    }

    delay(5000); 
}