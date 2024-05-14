/// новый код с разными страницами под каждую вороту
// со стартом месш через 20 сек после вай фай // не сработало
// исправить конфликт сетей

#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <WebServer.h>
#include <painlessMesh.h>

#define NUMPIXELS 49 // количество светодиодов в ленте
#define PIN 2        // порт, к которому подключена лента

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define MESH_PREFIX "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555
painlessMesh  mesh;

/* Установите здесь свои SSID и пароль */
const char* ssid = "ESP32";  
const char* password = "01234567";  
/* Настройки IP адреса */
IPAddress local_ip(192,168,2,1);
IPAddress gateway(192,168,2,1);
IPAddress subnet(255,255,255,0);
WebServer server(80);

unsigned long wifiStartTime = 0; // Время начала работы Wi-Fi сети
unsigned long meshStartTime = 20000; // Время начала работы MESH сети (20 секунд)

// Переменные для хранения цветов устройств
bool red = LOW;
bool green = LOW;
bool blue = LOW;
bool rainbow = LOW;

void setup() {
  Serial.begin(115200);
  
  // Настройка светодиодной ленты
  pixels.begin();
  pixels.show();
  pixels.setBrightness(50);

  // Настройка Wi-Fi и HTTP сервера
  WiFi.softAP(ssid, password);
  delay(100);
  WiFi.softAPConfig(local_ip, gateway, subnet);

  server.on("/", handle_OnConnect);
  server.on("/control", handle_Control);
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);
  server.on("/led2on", handle_led2on);
  server.on("/led2off", handle_led2off);
  server.on("/led3on", handle_led3on);
  server.on("/led3off", handle_led3off);
  server.on("/led4on", handle_led4on);
  server.on("/led4off", handle_led4off);
  server.onNotFound(handle_NotFound);
  server.begin();

  // Инициализация painlessMesh
  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION); // Настройка уровня отладки для painlessMesh
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);  // Установка обработчика приема сообщений
}

void loop() {
  unsigned long currentTime = millis();

  if (currentTime < meshStartTime) {
    // Время еще не вышло для запуска MESH сети
    mesh.update();  // Обновление состояния сети painlessMesh
  } else {
    // Время для работы MESH сети
    server.handleClient();
    updateLEDs();
  }
}

// Функция для обновления цветов светодиодной ленты
void updateLEDs() {
  if (red) {
    for (int i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, pixels.Color(255, 0, 0));
    }
  } else if (green) {
    for (int i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, pixels.Color(0, 255, 0));
    }
  } else if (blue) {
    for (int i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 255));
    }
  } else if (rainbow) {
    for (int i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, pixels.Color(25, 50, 25));
    }
  }
  pixels.show();
}

void handle_OnConnect() {
  String html = "<!DOCTYPE html> <html>\n";
  html += "<meta http-equiv=\"Content-type\" content=\"text/html; charset=utf-8\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  html += "<title>Управление светодиодными воротами</title>\n";
  html += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  html += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  html += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  html += ".button-on {background-color: #f24c28;}\n";
  html += ".button-on:active {background-color: #2980b9;}\n";
  html += ".button-off {background-color: #34495e;}\n";
  html += ".button-off:active {background-color: #2c3e50;}\n";
  html += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  html += "</style>\n";
  html += "</head>\n";
  html += "<body>\n";
  html += "<h2>ESP32 Веб сервер ESP32</h2>\n";
  html += "<h3>Выберите устройство для управления:</h3>\n";
  html += "<ul>";

  // Добавляем ссылки на устройства в сети
  for (int i = 0; i < mesh.getNodeList().size(); i++) {
    html += "<li><a href=\"/control?device=" + String(i) + "\">Устройство " + String(i+1) + "</a></li>";
  }

  html += "</ul>";
  html += "</body>\n";
  html += "</html>\n";
  server.send(200, "text/html", html);
}

void handle_Control() {
  String deviceID = server.arg("device");
  String html = "<!DOCTYPE html> <html>\n";
  html += "<meta http-equiv=\"Content-type\" content=\"text/html; charset=utf-8\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  html += "<title>Управление цветом устройства</title>\n";
  html += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  html += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  html += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  html += ".button-on {background-color: #f24c28;}\n";
  html += ".button-on:active {background-color: #2980b9;}\n";
  html += ".button-off {background-color: #34495e;}\n";
  html += ".button-off:active {background-color: #2c3e50;}\n";
  html += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  html += "</style>\n";
  html += "</head>\n";
  html += "<body>\n";
  html += "<h2>Управление цветом устройства</h2>\n";
  html += "<p>Выбранное устройство: " + deviceID + "</p>\n";
  html += "<a class=\"button button-on\" href=\"/led1on\">Красный ВКЛ.</a>\n";
  html += "<a class=\"button button-off\" href=\"/led1off\">Красный ВЫКЛ.</a>\n";
  html += "<a class=\"button button-on\" href=\"/led2on\">Зеленый ВКЛ.</a>\n";
  html += "<a class=\"button button-off\" href=\"/led2off\">Зеленый ВЫКЛ.</a>\n";
  html += "<a class=\"button button-on\" href=\"/led3on\">Синий ВКЛ.</a>\n";
  html += "<a class=\"button button-off\" href=\"/led3off\">Синий ВЫКЛ.</a>\n";
  html += "<a class=\"button button-on\" href=\"/led4on\">Разноцветный ВКЛ.</a>\n";
  html += "<a class=\"button button-off\" href=\"/led4off\">Разноцветный ВЫКЛ.</a>\n";
  html += "</body>\n";
  html += "</html>\n";
  server.send(200, "text/html", html);
}

// Обработчики для смены цветов устройств
void handle_led1on() {
  red = HIGH;
  server.sendHeader("Location", "/control", true);
  server.send(302, "text/plain", "");
}

void handle_led1off() {
  red = LOW;
  server.sendHeader("Location", "/control", true);
  server.send(302, "text/plain", "");
}

void handle_led2on() {
  green = HIGH;
  server.sendHeader("Location", "/control", true);
  server.send(302, "text/plain", "");
}

void handle_led2off() {
  green = LOW;
  server.sendHeader("Location", "/control", true);
  server.send(302, "text/plain", "");
}

void handle_led3on() {
  blue = HIGH;
  server.sendHeader("Location", "/control", true);
  server.send(302, "text/plain", "");
}

void handle_led3off() {
  blue = LOW;
  server.sendHeader("Location", "/control", true);
  server.send(302, "text/plain", "");
}

void handle_led4on() {
  rainbow = HIGH;
  server.sendHeader("Location", "/control", true);
  server.send(302, "text/plain", "");
}

void handle_led4off() {
  rainbow = LOW;
  server.sendHeader("Location", "/control", true);
  server.send(302, "text/plain", "");
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

// Обработчик приема сообщений в сети painlessMesh
void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
}
