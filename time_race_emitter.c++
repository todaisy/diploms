// отправка ик сигнала 
// код для ESP32 S3 Zero

#include <IRremoteESP8266.h>
#include <IRsend.h>

const int irLedPin = 5; // Пин, к которому подключен инфракрасный светодиод
IRsend irsend(irLedPin);

void setup() {
  Serial.begin(115200);
  Serial.println("IR LED Sender");

  // Инициализация объекта irsend
  irsend.begin();
}

void loop() {
  // Отправка инфракрасного сигнала на приемник
  // Пример кода и протокола (NEC)
  irsend.sendNEC(0xFFA25D, 32); // пример "1"
  delay(100); // Пауза между отправками сигнала
}
