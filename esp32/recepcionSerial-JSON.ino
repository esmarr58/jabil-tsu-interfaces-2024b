#include <ArduinoJson.h>

const int ledPin = 13;  // Definir el pin del LED
String inputString = ""; // Cadena para almacenar el mensaje recibido
bool stringComplete = false;  // Estado para saber si se ha completado el mensaje

void setup() {
  Serial.begin(115200);  // Inicializar comunicación serial
  pinMode(ledPin, OUTPUT);  // Definir el pin del LED como salida
  digitalWrite(ledPin, LOW); // Apagar el LED al inicio
}

void loop() {
  // Si se ha recibido una cadena completa, procesarla
  if (stringComplete) {
    StaticJsonDocument<200> doc;

    // Intentar analizar el JSON recibido
    DeserializationError error = deserializeJson(doc, inputString);

    if (error) {
      Serial.print(F("Error al analizar JSON: "));
      Serial.println(error.f_str());
    } else {
      // Obtener los valores del mensaje
      int pin = doc["pin"];
      int estado = doc["estado"];
      if(pin == 4 || pin == 13 || pin == 2){
           pinMode(pin, OUTPUT);
           delay(5);
            // Verificar si el pin es correcto y controlar el LED
          if (estado == 1) {
            digitalWrite(pin, HIGH);  // Encender el LED
            Serial.println("LED encendido");
          } else {
            digitalWrite(pin, LOW);   // Apagar el LED
            Serial.println("LED apagado");
          }
           
      }

     
      } 
    }

    // Limpiar la cadena y resetear el estado
    inputString = "";
    stringComplete = false;
  
}

// Función que se llama cuando llega un byte por el puerto serial
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;

    // Si detecta el carácter de nueva línea, la cadena está completa
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
