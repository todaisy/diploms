// есп раздает
// под юлин ик приемник - первый код
// receiver
// готовая проверенная система - работает отсчет, старт, стоп, сброс, итоговое время, обновление сайта каждые 5 секунд

#include <WiFi.h>
#include <WebServer.h>
#include <IRremote.h>
#include <EEPROM.h>

// Адрес в EEPROM для хранения значения startDetected
#define START_DETECTED_ADDR 0
#define START_TIME_ADDR 10 // Адрес в EEPROM для хранения значения startTime
#define FINISH_TIME_ADDR 20 // Адрес в EEPROM для хранения значения finishTime
#define TOTAL_TIME_ADDR 30 // Адрес в EEPROM для хранения значения totalTime

IRrecv irrecv(4); // порт, к которому подключен ик приемник
decode_results results;

/* Установите здесь свои SSID и пароль */
const char* ssid = "ESP32";  
const char* password = "01234567";  

/* Настройки IP адреса */
IPAddress local_ip(192,168,2,1);
IPAddress gateway(192,168,2,1);
IPAddress subnet(255,255,255,0);
WebServer server(80);

int num_circle = 1; // Переменная для хранения количества кругов
unsigned long circleTimes[10]; // Массив для хранения времени каждого круга
int circleIndex = 0; // Индекс текущего круга

bool startDetected = false;
unsigned long startTime = 0;
unsigned long finishTime = 0;
unsigned long totalTime = 0; // Общее время всех кругов
//unsigned long totalElapsedTime = 0; // Переменная для хранения суммарного времени всех кругов
unsigned long lastTriggerTime = 0; // Переменная для хранения времени последнего триггера
// unsigned long currentTime = 0;

void setup() {
  Serial.begin(115200);
  irrecv.enableIRIn();

  // Инициализация EEPROM
  EEPROM.begin(sizeof(startDetected) + sizeof(startTime) + sizeof(finishTime) + sizeof(totalTime));
  // Загрузка значений из EEPROM
  EEPROM.get(START_DETECTED_ADDR, startDetected);
  EEPROM.get(START_TIME_ADDR, startTime);
  EEPROM.get(FINISH_TIME_ADDR, finishTime);
  EEPROM.get(TOTAL_TIME_ADDR, totalTime);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  server.on("/", handle_OnConnect);
  server.on("/start", start);
  server.on("/stop", stop);
  server.on("/setNumCircles", HTTP_POST, setNumCircles); // Добавляем обработчик для изменения количества кругов
  server.on("/resetTime", resetTime);
  server.onNotFound(handle_NotFound);
  server.begin();

  Serial.println("HTTP server started");

  // Если старт еще не обнаружен, устанавливаем его в false
  if (!startDetected) {
    startDetected = false;
    // Сохраняем значение startDetected в EEPROM
    EEPROM.put(START_DETECTED_ADDR, startDetected);
    Serial.println("startDetected set to false and saved to EEPROM");
  }
}

void loop() {
  server.handleClient();
  if (startDetected && circleIndex < num_circle) { // Если таймер запущен и не все круги пройдены
    if (irrecv.decode(&results)) {
      Serial.println(results.value, HEX);
      unsigned long currentTime = millis();
      if (currentTime - lastTriggerTime > 15000) { // Проверяем прошло ли уже 15 секунд с последнего триггера
        finishTime = currentTime;
        if (!startDetected) {
          startDetected = true;
          startTime = currentTime;
          Serial.println("Start detected!");
        } 
        else {
          unsigned long elapsedTime = finishTime - startTime;
          Serial.print("Time taken: ");
          Serial.print(elapsedTime);
          Serial.println(" ms");

          // Сохраняем время текущего круга
          circleTimes[circleIndex++] = elapsedTime;
          if (circleIndex >= num_circle) {
            // Если все круги засечены, считаем сумму времени и отправляем данные на страницу
            totalTime = 0; // Сбрасываем общее время перед подсчетом для новых кругов
            for (int i = 0; i < num_circle; i++) {
              totalTime += circleTimes[i];
            }
            circleIndex = 0;
            server.send(200, "text/plain", "Все круги завершены. Итоговое время: " + String(totalTime) + " ms.");
          }

          // Сбрасываем таймер и ждем 15 секунд перед продолжением
          startTime = currentTime; // Сбрасываем время для нового круга
          lastTriggerTime = currentTime;
        }
      }
      irrecv.resume();
    }
  } else if (startDetected && circleIndex >= num_circle) { // Если таймер запущен и все круги пройдены
    // Остановка отсчета времени
    startDetected = false;
    // Остановка приема IR-сигналов
    irrecv.disableIRIn();
    // Отправка данных на страницу
    server.send(200, "text/plain", "Все круги завершены. Итоговое время: " + String(totalTime) + " ms.");
    // Сброс индекса текущего круга
    circleIndex = 0;
    // Перенаправление на главную страницу
    redirectToMainPage();
  }
}


void handle_OnConnect() {
  if (!startDetected) {
    // Если старт еще не обнаружен, устанавливаем его в false
    startDetected = false;
    // Сохраняем значение startDetected в EEPROM
    EEPROM.put(START_DETECTED_ADDR, startDetected);
  }
  server.send(200, "text/html", SendHTML(startDetected));
}

void start() {
  if (!startDetected) {
    startDetected = true;
    startTime = millis();
    // currentTime = millis();
    server.send(200, "text/html", SendHTML(startDetected));
    Serial.println("Timer started!");
  }
  else {
    Serial.println("Timer is already started!");
  }
  redirectToMainPage();
}

void stop() {
  startDetected = false;
  //finishTime = 0; // Убираем обнуление finishTime
  // Сохраняем значение startDetected в EEPROM
  EEPROM.put(START_DETECTED_ADDR, startDetected);
  Serial.println("Time status: Stop");

  // Сбрасываем время всех кругов
  // totalTime = 0;
  // Отправляем страницу с итоговым временем на сайт
  handle_OnConnect();
  // Добавляем перенаправление на главную страницу
  redirectToMainPage();
}

void setNumCircles() {
  if (server.args() > 0) {
    num_circle = server.arg(0).toInt(); // Получаем новое значение из формы
    redirectToMainPage();
  }
  server.send(200, "text/html", SendHTML(startDetected));
  //redirectToMainPage();
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

void resetTime(){
  // Сбрасываем все переменные, связанные с отсчетом времени
  startDetected = false;
  startTime = 0;
  finishTime = 0;
  totalTime = 0;
  //currentTime = 0;

  // Сброс времени каждого круга
  for (int i = 0; i < 10; i++) {
    circleTimes[i] = 0;
  }

  // Сохраняем значения переменных в EEPROM
  EEPROM.put(START_DETECTED_ADDR, startDetected);
  EEPROM.put(START_TIME_ADDR, startTime);
  EEPROM.put(FINISH_TIME_ADDR, finishTime);
  EEPROM.put(TOTAL_TIME_ADDR, totalTime);

  redirectToMainPage();
}

void redirectToMainPage() {
  Serial.println("Redirecting to the main page...");
  server.sendHeader("Location", "/", true);
  server.send(303);
}

String SendHTML(bool isStarted){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<meta http-equiv=\"Content-type\" content=\"text/html; charset=utf-8\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Засечка ESP32</title>\n";
  ptr += "<meta http-equiv=\"refresh\" content=\"5\">"; // Автоматическая перезагрузка каждые 5 секунд
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h2 {color: #444444;margin-bottom: 20px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #78f46b;}\n";
  ptr +=".button-on:active {background-color: #25df11;}\n";
  ptr +=".button-stop {background-color: #f24c28;}\n";
  ptr +=".button-stop:active {background-color: #c82f0d;}\n"; 
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h2>ESP32 Веб сервер</h2>\n";

  // Отображение времени от старта, если таймер запущен
  if (isStarted) {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - startTime;
    ptr += "<p>Время от старта: ";
    ptr += elapsedTime / 1000; // Переводим миллисекунды в секунды
    ptr += " с</p>\n";
  }

  ptr +="<h2>Количество кругов</h2>\n";
  ptr += num_circle;
  ptr += "</p>\n";
  ptr += "<form action=\"/setNumCircles\" method=\"POST\">\n"; // Форма для изменения количества кругов
  ptr += "<input type=\"text\" name=\"numCircles\">\n";
  ptr += "<input type=\"submit\" value=\"Set\">\n";
  ptr += "</form>\n";

  // Добавляем вывод времени каждого круга
  ptr += "<h2>Время за каждый круг:</h2>\n";
  ptr += "<ul>\n";
  for (int i = 0; i < num_circle; i++) {
    ptr += "<li>Круг ";
    ptr += i + 1;
    ptr += ": ";
    // Вычисляем секунды и сотые секунды
    unsigned long seconds = circleTimes[i] / 1000;
    unsigned long hundredths = (circleTimes[i] % 1000) / 10;
    ptr += seconds;
    ptr += ".";
    if (hundredths < 10) ptr += "0"; // Добавляем ведущий ноль, если сотые меньше 10
    ptr += hundredths;
    ptr += " с</li>\n";
  }
  ptr += "</ul>\n";

  // Вывод итогового времени за все круги
  if (!isStarted && circleIndex == 0) {
    ptr += "<p>Итоговое время за все круги: ";
    ptr += String(totalTime);
    ptr += " мс</p>\n";
  }

  if(isStarted)
  {ptr +="<p>Start/Stop: Start</p><a class=\"button button-stop\" href=\"/stop\">Stop</a>\n";}
  else
  {ptr +="<p>Start/Stop: Stop</p><a class=\"button button-on\" href=\"/start\">Start</a>\n";}

  // Кнопка сброса времени
  ptr += "<form action=\"/resetTime\" method=\"POST\">\n";
  ptr += "<input type=\"submit\" value=\"Reset Time\">\n";
  ptr += "</form>\n";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

