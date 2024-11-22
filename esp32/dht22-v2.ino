#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// Cambiar esta variable a `false` para simular datos
bool useSensor = false;

void setup() {
  Serial.begin(115200);
  Serial.println(F("Sensor DHTxx listo. Esperando la entrada 'a' seguida de '\\n' para leer datos."));

  if (useSensor) {
    Serial.println(F("Modo: Usando sensor DHT22"));
    dht.begin();
  } else {
    Serial.println(F("Modo: Simulación de datos"));
  }
}

void loop() {
  if (Serial.available() > 0) {
    char incomingChar = Serial.read();

    if (incomingChar == 'a') {
      unsigned long startTime = millis();
      while (Serial.available() == 0) {
        if (millis() - startTime > 2000) {
          Serial.println(F("Error: no se recibió el salto de línea."));
          return;
        }
      }
      if (Serial.read() == '\n') {
        float h, t;

        if (useSensor) {
          // Leer datos reales del sensor
          h = dht.readHumidity();
          t = dht.readTemperature();

          // Comprobar si alguna lectura falló
          if (isnan(h) || isnan(t)) {
            Serial.println(F("¡Error al leer del sensor DHT!"));
            return;
          }
        } else {
          // Generar datos aleatorios para simulación
          h = random(3000, 8000) / 100.0; // Humedad simulada (30.00% a 80.00%)
          t = random(2000, 3500) / 100.0; // Temperatura simulada (20.00°C a 35.00°C)
        }

        // Imprimir los datos en formato JSON
        Serial.print("{\"temp\":");
        Serial.print(t, 2);
        Serial.print(", \"humedad\":");
        Serial.print(h, 2);
        Serial.println("}");
      }
    }
  }
  delay(100);
}
