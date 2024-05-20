// без месш 
// сайт с кнопками - проверен на вс2811 - работает

#include <Adafruit_NeoPixel.h>
#define NUMPIXELS 9 // количество светодиодов в ленте
#define PIN 2 // порт, к которому подключена лента
// volatile int counter = 0;
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ400);

#include <WiFi.h>
#include <WebServer.h>
/* Установите здесь свои SSID и пароль */
const char* ssid = "ESP32";  
const char* password = "01234567";  
/* Настройки IP адреса */
IPAddress local_ip(192,168,2,1);
IPAddress gateway(192,168,2,1);
IPAddress subnet(255,255,255,0);
WebServer server(80);
bool red = LOW;
bool green = LOW;
bool blue = LOW;
bool rainbow = LOW;


void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  server.on("/", handle_OnConnect);
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
  Serial.println("HTTP server started");

  pixels.begin();
  pixels.show();
  pixels.setBrightness(50);
}


void loop() {
  server.handleClient();
  if(red)
  {
    for(int i=0; i<pixels.numPixels(); i++) 
    {
      pixels.setPixelColor(i, pixels.Color(255,0,0));
      delay(1);
    }
    pixels.show();
    delay(10);
  }

  if(green)
  {
    for(int i=0; i<pixels.numPixels(); i++) 
    {
      pixels.setPixelColor(i, pixels.Color(0,255,0));
      delay(1);
    }
    pixels.show();
    delay(10);
  }

  if(blue)
  {
    for(int i=0; i<pixels.numPixels(); i++) 
    {
      pixels.setPixelColor(i, pixels.Color(0,0,255));
      delay(1);
    }
  pixels.show();
  delay(10);
  }

  if(rainbow)
  {
    for(int i=0; i<pixels.numPixels(); i++) 
    {
      pixels.setPixelColor(i, pixels.Color(25,50,25));
      delay(1);
    }
    pixels.show();
    delay(10);
  }
}

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
  red = LOW;
  green = LOW;
  blue = LOW;
  rainbow = LOW;
  Serial.println("All Status: OFF");
  server.send(200, "text/html", SendHTML(red,green,blue,rainbow)); 
}

void handle_led1on() {
  red = HIGH;
  Serial.println("Red Status: ON");
  server.send(200, "text/html", SendHTML(true,green,blue,rainbow));   
}

void handle_led1off() {
  red = LOW;
  Serial.println("Red Status: OFF");
  server.send(200, "text/html", SendHTML(false,green,blue,rainbow)); 
}

void handle_led2on() {
  green = HIGH;
  Serial.println("Green: ON");
  server.send(200, "text/html", SendHTML(red,true,blue,rainbow)); 
}

void handle_led2off() {
  green = LOW;
  Serial.println("Green: OFF");
  server.send(200, "text/html", SendHTML(red,false, blue, rainbow)); 
}

void handle_led3on() {
  blue = HIGH;
  Serial.println("Blue: ON");
  server.send(200, "text/html", SendHTML(red,green, true, rainbow)); 
}

void handle_led3off() {
  blue = LOW;
  Serial.println("Blue: OFF");
  server.send(200, "text/html", SendHTML(red,green, false, rainbow)); 
}

void handle_led4on() {
  rainbow = HIGH;
  Serial.println("Rainbow: ON");
  server.send(200, "text/html", SendHTML(red,green,blue,true)); 
}

void handle_led4off() {
  rainbow = LOW;
  Serial.println("Rainbow: OFF");
  server.send(200, "text/html", SendHTML(red,green,blue,false)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(uint8_t redstat, uint8_t greenstat, uint8_t bluestat, uint8_t rainbowstat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<meta http-equiv=\"Content-type\" content=\"text/html; charset=utf-8\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Управление светодиодными воротами</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #f24c28;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h2>ESP32 Веб сервер ESP32</h2>\n";
  ptr +="<h3>Переключение режимов</h3>\n";

  if(redstat)
  {ptr +="<p>Красный LED1: ВКЛ.</p><a class=\"button button-off\" href=\"/led1off\">ВЫКЛ.</a>\n";}
  else
  {ptr +="<p>Красный LED1: ВЫКЛ.</p><a class=\"button button-on\" href=\"/led1on\">ВКЛ.</a>\n";}

  if(greenstat)
  {ptr +="<p>Зеленый LED2: ВКЛ.</p><a class=\"button button-off\" href=\"/led2off\">ВЫКЛ.</a>\n";}
  else
  {ptr +="<p>Зеленый LED2: ВЫКЛ.</p><a class=\"button button-on\" href=\"/led2on\">ВКЛ.</a>\n";}

  if(bluestat)
  {ptr +="<p>Синий LED3: ВКЛ.</p><a class=\"button button-off\" href=\"/led3off\">ВЫКЛ.</a>\n";}
  else
  {ptr +="<p>Синий LED3: ВЫКЛ.</p><a class=\"button button-on\" href=\"/led3on\">ВКЛ.</a>\n";}

  if(rainbowstat)
  {ptr +="<p>Разноцветный LED4: ВКЛ.</p><a class=\"button button-off\" href=\"/led4off\">ВЫКЛ.</a>\n";}
  else
  {ptr +="<p>Разноцветный LED4: ВЫКЛ.</p><a class=\"button button-on\" href=\"/led4on\">ВКЛ.</a>\n";}

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
