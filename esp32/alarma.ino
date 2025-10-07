#include <ArduinoJson.h>
#include "DHT.h"

// ========================
// Configuración general
// ========================
const uint8_t ledPinDefault = 13;

// --- Nuevo: Buzzer (ajusta el pin si lo necesitas) ---
const uint8_t buzzerPin = 8;      // Pin digital del buzzer
bool buzzerActivo = false;        // Estado de alarma/buzzer

// Patrón de beep no-bloqueante
static unsigned long buzzerLastMs = 0;
static bool buzzerPulsando = false;
const unsigned long buzzerPeriodoMs = 500;  // alterna cada 500 ms
const unsigned int  buzzerTonoHz    = 1000; // 1 kHz
const unsigned int  buzzerPulsoMs   = 200;  // 200 ms de tono

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

  // Pequeña espera; para lecturas ultra confiables se recomienda 1–2s entre lecturas
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
// Prototipos
// ========================
void processLine(const char* line);

// ========================
// Setup / Loop
// ========================
void setup() {
  pinMode(ledPinDefault, OUTPUT);
  digitalWrite(ledPinDefault, LOW);

  pinMode(buzzerPin, OUTPUT);
  noTone(buzzerPin);    // Asegura buzzer apagado al inicio

  Serial.begin(115200);
  Serial.setTimeout(50);

  // Mensaje de arranque
  Serial.println(F("Listo. Envia JSON por linea: "
                   "{\"pin\":13,\"estado\":1}  "
                   "{\"sensor\":\"dht22\",\"pin\":4}  "
                   "{\"buzzer\":1}"));
}

void loop() {
  // --- Lectura de líneas por Serial ---
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

  // --- Patrón de buzzer no-bloqueante (alarma) ---
  if (buzzerActivo) {
    unsigned long now = millis();
    if (now - buzzerLastMs >= buzzerPeriodoMs) {
      buzzerLastMs = now;
      buzzerPulsando = !buzzerPulsando;

      if (buzzerPulsando) {
        // Arranca un pulso corto de tono (no bloquea)
        tone(buzzerPin, buzzerTonoHz, buzzerPulsoMs);
      } else {
        // Silencio entre pulsos
        noTone(buzzerPin);
      }
    }
  } else {
    // Si no hay alarma, aseguramos silencio
    noTone(buzzerPin);
    buzzerPulsando = false;
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

  // --- Rama BUZZER/ALARMA: {"buzzer":1} ó {"buzzer":0} ---
  if (doc.containsKey("buzzer")) {
    int estadoBuzzer = doc["buzzer"].as<int>();
    buzzerActivo = (estadoBuzzer == 1);

    if (!buzzerActivo) {
      noTone(buzzerPin); // Apaga inmediatamente si se desactiva
      buzzerPulsando = false;
    }
    Serial.println(buzzerActivo ? F("Buzzer activado") : F("Buzzer desactivado"));
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
  Serial.println(F("JSON no reconocido. Usa {\"pin\", \"estado\"}  "
                   "{\"sensor\":\"dht22\",\"pin\"}  "
                   "{\"buzzer\":1}"));
}
