#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "INFINITUM1C29";       
const char* password = "maUk4yEP9d"; 

const String serie = "id0002";  

const char* mqtt_server = "34.232.41.236";
const int mqtt_port = 1883;
const char* mqtt_user = "falejandro";
const char* mqtt_password = "falejandrozroot";
const char* queue_topic = "alert.#";  

#define BUZZER_PIN 25

WiFiClient espClient;
PubSubClient client(espClient);

void sendHttpPost(const char* payload) {
    alert();
  
    HTTPClient http;
    http.begin("http://13.216.151.168:8000/sensor");  
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(payload);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    
    http.end();
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Mensaje recibido en tópico: ");
    Serial.println(topic);
    Serial.print("Mensaje: ");

    String receivedMsg;
    for (int i = 0; i < length; i++) {
        receivedMsg += (char)payload[i];
    }
    Serial.println(receivedMsg);

    // Parsear el JSON recibido
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, receivedMsg);
    
    if (error) {
        Serial.print("Error al parsear JSON: ");
        Serial.println(error.c_str());
        return;
    }

    // Filtrar solo mensajes que coincidan con la serie global
    String receivedSerie = doc["serie"].as<String>();
    if (receivedSerie != serie.c_str()) {  
        Serial.println("Serie no coincide, ignorando mensaje.");
        return;
    }

    // Construcción del nuevo JSON para el POST
    StaticJsonDocument<512> postDoc;
    postDoc["id"] = doc["id"];
    postDoc["title"] = doc["title"];
    postDoc["description"] = String(doc["title"].as<const char*>()) + " " + String(doc["description"].as<const char*>());
    postDoc["emmiter"] = doc["emitter"];
    postDoc["topic"] = receivedSerie;
    postDoc["created_at"] = doc["createdAt"];

    String postPayload;
    serializeJson(postDoc, postPayload);

    Serial.println("Enviando HTTP POST con el siguiente JSON:");
    Serial.println(postPayload);

    sendHttpPost(postPayload.c_str());
}

void setup_wifi() {
    Serial.print("Conectando a ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    
    Serial.println("\nWi-Fi conectado");
    Serial.print("IP local: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Conectando a RabbitMQ...");

        if (client.connect("ESP32_Client", mqtt_user, mqtt_password)) {
            Serial.println("Conectado!");
            client.subscribe(queue_topic); 
            Serial.println("Suscrito a "+ String(queue_topic) + "!"); 
        } else {
            Serial.print("Fallo, rc=");
            Serial.print(client.state());
            Serial.println(" Intentando de nuevo en 5 segundos...");
            delay(5000);
        }
    }
}
void alert(){
    static unsigned long lastBuzzTime = 0;
    static bool buzzerState = false;
    unsigned long currentMillis = millis();

    if (currentMillis - lastBuzzTime >= 500) {
        buzzerState = !buzzerState;
        digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
        lastBuzzTime = currentMillis;
    }
}

void setup() {
    Serial.begin(115200);
    setup_wifi();

    pinMode(BUZZER_PIN, OUTPUT);
    
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    
    client.loop();
}