#include <ArduinoJson.h>
#include "DHT.h"

// ========================
// Configuración general
// ========================
const uint8_t ledPinDefault = 13;

// Buffer para recibir una línea JSON por Serial
const size_t BUF_LEN = 160;      // Aumentado por si el JSON crece
char inBuf[BUF_LEN];
size_t inLen = 0;

// ========================
// Utilidades DHT22
// ========================
static const uint8_t DHT_TYPE = DHT22; // Forzamos DHT22

// Lee una muestra de DHT22 en 'pinDHT' y escribe JSON en Serial
void leerDHT22(uint8_t pinDHT) {
  DHT dht(pinDHT, DHT_TYPE);
  dht.begin();

  // Pequeña espera inicial. Para lecturas fiables se suele recomendar
  // ~1-2s entre lecturas; aquí sólo hacemos una muestra rápida.
  delay(30);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    StaticJsonDocument<128> out;
    out["ok"]    = false;
    out["error"] = "Fallo al leer DHT22";
    serializeJson(out, Serial);
    Serial.println();
    return;
  }

  StaticJsonDocument<128> out;
  out["ok"]      = true;
  out["temp"]    = t;   // °C
  out["humedad"] = h;   // %
  serializeJson(out, Serial);
  Serial.println();
}

// ========================
// Setup / Loop
// ========================
void setup() {
  pinMode(ledPinDefault, OUTPUT);
  digitalWrite(ledPinDefault, LOW);

  Serial.begin(115200);
  Serial.setTimeout(50);

  // Mensaje de arranque
  Serial.println(F("Listo. Envia JSON por linea: "
                   "{\"pin\":13,\"estado\":1} o {\"sensor\":\"dht22\",\"pin\":4}"));
}

void loop() {
  // Leer caracteres disponibles
  while (Serial.available() > 0) {
    char c = (char)Serial.read();

    if (c == '\r') continue; // Ignora CR

    if (c == '\n') {
      // Termina la cadena y procesa
      inBuf[inLen] = '\0';
      processLine(inBuf);
      inLen = 0;
    } else {
      if (inLen < (BUF_LEN - 1)) {
        inBuf[inLen++] = c;
      } else {
        // Línea demasiado larga: resetea y avisa
        inLen = 0;
        Serial.println(F("Linea demasiado larga"));
      }
    }
  }
}

// ========================
// Procesamiento de la línea JSON
// ========================
void processLine(const char* line) {
  StaticJsonDocument<256> doc;

  DeserializationError err = deserializeJson(doc, line);
  if (err) {
    Serial.print(F("Error al analizar JSON: "));
    Serial.println(err.f_str());
    return;
  }

  // --- Rama LED: compatibilidad con tu formato actual ---
  // Requiere "pin" y "estado"
  if (doc.containsKey("pin") && doc.containsKey("estado")) {
    int pin = doc["pin"].as<int>();
    int estado = doc["estado"].as<int>();

    // Validación opcional de rango:
#if defined(ARDUINO_AVR_UNO)
    if (pin < 0 || pin > 13) {
      Serial.println(F("Pin fuera de rango (0..13 para Arduino UNO)"));
      return;
    }
#endif

    pinMode(pin, OUTPUT);
    delay(2);
    digitalWrite(pin, (estado == 1) ? HIGH : LOW);
    Serial.println((estado == 1) ? F("LED encendido") : F("LED apagado"));
    return;
  }

  // --- Rama DHT22: admite varios alias ---
  // 1) {"sensor":"dht22","pin":4}
  // 2) {"dht":4}
  // 3) {"cmd":"leer_dht","pin":4}
  bool esDht = false;
  int  dhtPin = -1;

  if (doc.containsKey("sensor")) {
    const char* s = doc["sensor"];
    if (s && (strcmp(s, "dht22") == 0)) {
      esDht = true;
      if (doc.containsKey("pin")) dhtPin = doc["pin"].as<int>();
    }
  }

  if (!esDht && doc.containsKey("dht")) {
    esDht = true;
    dhtPin = doc["dht"].as<int>();
  }

  if (!esDht && doc.containsKey("cmd")) {
    const char* c = doc["cmd"];
    if (c && (strcmp(c, "leer_dht") == 0)) {
      esDht = true;
      if (doc.containsKey("pin")) dhtPin = doc["pin"].as<int>();
    }
  }

  if (esDht) {
    if (dhtPin < 0) {
      StaticJsonDocument<128> out;
      out["ok"]    = false;
      out["error"] = "Falta 'pin' para DHT22";
      serializeJson(out, Serial);
      Serial.println();
      return;
    }
    leerDHT22((uint8_t)dhtPin);
    return;
  }

  // Si llegó aquí, el JSON no corresponde a ninguno de los comandos soportados
  Serial.println(F("JSON no reconocido. Usa {\"pin\", \"estado\"} o {\"sensor\":\"dht22\",\"pin\"}"));
}
