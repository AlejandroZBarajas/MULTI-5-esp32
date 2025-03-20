#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "INFINITUM1C29";       
const char* password = "maUk4yEP9d"; 


const char* mqtt_server = "34.232.41.236";
const int mqtt_port = 1883;
const char* mqtt_user = "falejandro";
const char* mqtt_password = "falejandrozroot";
const char* queue_topic = "alert.#";  

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Mensaje recibido en tópico: ");
    Serial.println(topic);
    Serial.print("Mensaje: ");
    
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
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

void setup() {
    Serial.begin(115200);
    setup_wifi();
    
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    
    client.loop();
}