/*
Arduino   v1.8.19  ok
ESP32     v3.0.5   ok
DHT Sensor library v1.4.6 by Adafruit

  */

#include "DHT.h"

#define DHTPIN 4     // Pin digital conectado al sensor DHT
#define DHTTYPE DHT22   // DHT 22 (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.println(F("Sensor DHTxx listo. Esperando la entrada 'a' seguida de '\\n' para leer la temperatura."));
  
  dht.begin();
}

void loop() {
  // Verificar si hay datos disponibles en el puerto serial
  if (Serial.available() > 0) {
    // Leer el carácter entrante
    char incomingChar = Serial.read();

    // Si el carácter es 'a' y seguido de un salto de línea (\n), tomar la lectura
    if (incomingChar == 'a') {
      // Esperar a que llegue el carácter de salto de línea
      while (Serial.available() == 0) {
        // Esperar los datos entrantes
      }
      if (Serial.read() == '\n') {
        // Tomar las lecturas de temperatura y humedad
        float h = dht.readHumidity();
        float t = dht.readTemperature();

        // Comprobar si alguna lectura falló
        if (isnan(h) || isnan(t)) {
          Serial.println(F("¡Error al leer del sensor DHT!"));
          return;
        }

        // Preparar el mensaje JSON
        Serial.print("{\"temp\":");
        Serial.print(t, 2);  // Imprimir temperatura con 2 decimales
        Serial.print(", \"humedad\":");
        Serial.print(h, 2);  // Imprimir humedad con 2 decimales
        Serial.println("}");
      }
    }
  }
  // Opcional: pequeño retraso para evitar sobrecargar el bucle
  delay(100);
}
