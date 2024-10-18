void setup() {
  // Inicia la comunicación serial a 115200 bps
  Serial.begin(115200);
}

void loop() {
  // Verifica si hay datos disponibles en el puerto serial
  if (Serial.available() > 0) {
    // Lee el dato recibido
    int dato = Serial.read();

    // Envia de vuelta el mismo dato recibido
    Serial.print("Recibido: ");
    Serial.println(dato);
  }

  // Envía un mensaje cada 5 segundos
  delay(5000);
  Serial.println("Esperando datos...");
}


