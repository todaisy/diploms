/*
MH-Z19 11А
TX: 3<->5 16
RX: 3<->5 17
PWM:      12
Gnd:      Gnd
Vin:      +5v

S8:
G+:       +5v
G0:       Gnd
TX: 3<->5 1
RX: 3<->5 3

DS3231
SCL:      A5
SDA:      A4
Gnd:      Gnd
VDD:      5v
*/

// для датчиков
//#include <SoftwareSerial.h>
#include "Wire.h"
#include <Arduino.h>
#include "MHZ19.h"

// для бота
#include "WiFi.h" 
#include "WiFiClientSecure.h" 
#include "UniversalTelegramBot.h" 
#include "ArduinoJson.h" 

// Укажите свои данные сети
const char* ssid = "Greenvent";
const char* password = "11223344"; // 9850590871
 
// Используйте @myidbot, чтобы получить ID пользователя или группы
// Помните, что бот сможет вам писать только после нажатия вами кнопки /start
#define CHAT_ID "333152420"
// Запустите бот Telegram
#define BOTtoken "6888409297:AAG6HpFuiMcUaG4i0ePldaCZSmkItD73DGk"  
 
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
 
// Каждую секунду проверяет новые сообщения
int botRequestDelay = 1000;
unsigned long lastTimeBotRan; 

// мб 41 и 40 пин - 0 тх рх
// при загрузке кода обязательно снимать пины 1, 3
#define S8_TX 1 // GPIO pin for S8 sensor rX 
#define S8_RX 3 // GPIO pin for S8 sensor tX
HardwareSerial mySerial_s8(1); // тут какой порт???

#define Z19_TX 17 // GPIO pin for S8 sensor rX
#define Z19_RX 16 // GPIO pin for S8 sensor tX
#define BAUDRATE 9600 
// HardwareSerial mySerial_s8(2);
MHZ19 myMHZ19;    // Constructor for library
HardwareSerial mySerial(2);    // On ESP32 we do not require the SoftwareSerial library, since we have 2 USARTS available

unsigned long getDataTimer = 0;

int s8_co2;
int s8_co2_mean;
int s8_co2_mean2;

float smoothing_factor = 0.5;
float smoothing_factor2 = 0.15;

byte cmd_s8[]       = {0xFE, 0x04, 0x00, 0x03, 0x00, 0x01, 0xD5, 0xC5};
byte abc_s8[]       = {0xFE, 0x03, 0x00, 0x1F, 0x00, 0x01, 0xA1, 0xC3};
byte response_s8[7] = {0, 0, 0, 0, 0, 0, 0};
const int r_len = 7;
const int c_len = 8;
 
// Задаем действия при получении новых сообщений
void handleNewMessages(int numNewMessages) 
{
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));
 
  for (int i = 0; i < numNewMessages; i++) 
  { 
    // Идентификатор чата запроса 
    String chat_id = String(bot.messages[i].chat_id); 
    if (chat_id != CHAT_ID) 
    { 
      bot.sendMessage(chat_id, "Unauthorized user", ""); 
      continue; 
    } 
 
    // Выводим полученное сообщение 
    String text = bot.messages[i].text; 
    Serial.println(text); 
    String from_name = bot.messages[i].from_name; 
 
    if (text == "/start") 
    { 
      String welcome = "Welcome, " + from_name + ".\n"; 
      welcome += "Use the following command to get current readings.\n\n";
      welcome += "/readings \n"; 
 
      bot.sendMessage(chat_id, welcome, ""); 
    } 
 
    if (text == "/readings") // надо это условие на миллисы переписать 
    { 
      String readings = co2_measure(); 
      bot.sendMessage(chat_id, readings, ""); 
    } 
  } 
} 



// Запрос показаний датчика S8 и запись их в переменную типа String
String co2_measure() {
  //s8Request(cmd_s8);
  s8_co2 = getValue_s8(response_s8);
  
  if (!s8_co2_mean) s8_co2_mean = s8_co2;
  s8_co2_mean = s8_co2_mean - smoothing_factor*(s8_co2_mean - s8_co2);

  if (!s8_co2_mean2) s8_co2_mean2 = s8_co2;
  s8_co2_mean2 = s8_co2_mean2 - smoothing_factor2*(s8_co2_mean2 - s8_co2);

  // MH-Z1911B
  int CO2;
  /* note: getCO2() default is command "CO2 Unlimited". This returns the correct CO2 reading even
  if below background CO2 levels or above range (useful to validate sensor). You can use the
  usual documented command with getCO2(false) */

  CO2 = myMHZ19.getCO2(); // Request CO2 (as ppm)
  Serial.print("CO2 (ppm): ");
  Serial.println(CO2);

  int8_t Temp;
  Temp = myMHZ19.getTemperature(); // Request Temperature (as Celsius)
  Serial.print("Temperature (C): ");
  Serial.println(Temp);

  String message = "S8 CO2 value: " + String(s8_co2) + " M1Value:" + String(s8_co2_mean) + " M2Value:" + String(s8_co2_mean2) 
                  + "; MH-Z1911B CO2 value (ppm): " + String(CO2) + " Temperature (C): " + String(Temp);
  return message;
}

// это таймер для бота, потом исправить
//if (millis() - getDataTimer >= 2000)
//{
//  getDataTimer = millis();
//}


void setup() 
{ 
  Serial.begin(115200);
  mySerial_s8.begin(9600, SERIAL_8N1, S8_RX, S8_TX); // Initialize hardware serial for S8 sensor 

  mySerial.begin(BAUDRATE);   // (Uno example) device to MH-Z19 serial start
  myMHZ19.begin(mySerial);    // *Serial(Stream) reference must be passed to library begin().
  myMHZ19.autoCalibration();  // Turn auto calibration ON (OFF autoCalibration(false))
 
  // Подключаемся к Wi-Fi 
  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password);
 
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Добавляем корневой сертификат для api.telegram.org 
 
  while (WiFi.status() != WL_CONNECTED) 
  { 
    delay(1000); 
    Serial.println("Подключение WiFi.."); 
  } 
 
  // Выводим IP ESP32
  Serial.println(WiFi.localIP()); 
} 


void sendRequest_s8(byte packet[]) {
  mySerial_s8.write(packet, 7);
  mySerial_s8.flush(); // Ensure all data is sent before reading
  delay(100); // Wait for response
}

unsigned long getValue_s8(byte packet[]) {
  unsigned long val = 0;
  if (mySerial_s8.available() >= 9) { // Ensure there are enough bytes to read
    mySerial_s8.readBytes(packet, 9);
    if (packet[0] == 0xFF && packet[1] == 0x86) { // Check for correct response format
      int high = packet[2];
      int low = packet[3];
      val = high * 256 + low;
    } else {
      Serial.println("Invalid response from S8 sensor");
    }
    } else {
      Serial.println("No response from S8 sensor");
    }
  return val;
}
 
void loop() 
{ 
  if (millis() > lastTimeBotRan + botRequestDelay)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
 
    while(numNewMessages) 
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
        
    lastTimeBotRan = millis();
  }
}
