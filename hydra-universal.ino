#include <ESP8266WiFi.h>              // Библиотека для взаимодействия платы с wi-fi сетями
#include <ESP8266WebServer.h>         // Библиотека для создания веб-сервера
#include <ESP8266HTTPClient.h> 
#include <Wire.h>                     // Библиотека для работы с шиной I2C
#include <Adafruit_BME280.h>          // Библиотека для работы с датчиками bme280
#include <DNSServer.h>
#include <WiFiManager.h>         
#include <ESP8266SSDP.h>
#include <aREST.h>
#include <FS.h>                       // Библиотека для SPIFFS
#include <ArduinoJson.h>              // Библиотека для обработки данных
#include <ESP8266WiFiMulti.h>         // Библиотека для работы с множеством точек досутпа
#include <GyverOLED.h>                // Библиотека для OLED экрана
#include <LiquidCrystal_I2C_mod.h>    // Библиотека для LCD экрана (МОДИФИЦИРОВАННАЯ)

using namespace std;
#define PERIOD (5*60*1000)            // 5 минут в миллисекундах
#define BME280_ADDRESS (0x76)         // Адрес для подключения датчиков bme280 через I2C

ESP8266WebServer HTTP(80);            //Web интерфейс для устройства
HTTPClient http;
WiFiClient wifiClient;
WiFiServer SERVERaREST(8080);         //aREST и сервер для него
ESP8266WiFiMulti wifiMulti;
IPAddress myIP;                       //хранит IP адресс  
GyverOLED<SSD1306_128x32, OLED_NO_BUFFER> oled;      // Объект OLED
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled64;    // Объект OLED без пропуска строки
LiquidCrystal_I2C lcd(0x1, 16, 2);    // Объект LCD
size_t indexAppendSsid = 0;
File fsUploadFile;

unsigned long oldmillis=PERIOD+PERIOD;//затравочное ("неправильное") значение
unsigned long when = millis();        //Таймер для отправки в БД
unsigned long mainDisplayTimer;       //Таймер для отображения инф. с датчика (осн.) на дисплее
unsigned long wifiDisplayTimer;       //Таймер для отображения инф. о подключении на дисплее

double pressure=0; 
double temp=0; 
double hum=0; 
double mmHg=133.3;                    //величина для перевода паскалей в мм.рт.ст
const int n=9; 
const int wf=7;                       //количество сканирируемых сетей
const int wf_configs=20;              //количество сохраняемых элементов в памяти
int dispayShowType = 0;               //Тип отображаемой инф. на дисплее
                  // 0 - информация с датчика 
                  // 1 - информаиця о wifi (1 стр)
                  // 2 - информаиця о wifi (2 стр) 
                  // 3 - информаиця о wifi (3 стр)

int count_wifi=20;                    //количество wifi сетей
int rssi[wf]={};                      // может принимать значения от 0 до -100 дБм 
int typeDevice = 3;                   // Тип найденного дисплея
              // 0 - OLED
              // 1 - LCD (0x27)
              // 2 - LCD (0x?)
              // 3 - Ничего (default)
String ssid[wf_configs]={};           //массив имен сетей
String password[wf_configs]={};       //массив паролей сетей
String myAP;                          //режим точки доступа
String mySSID;                        //флаг использования собственной точки доступа WiFi
String ssid_scan[wf]={};
String whoami;
String Akey;
String jsonConfig = "{}";             //переменная, хранящая файл конфигурации
String ssidAP;                        // SSID точки доступа
String passwordAP = "";               // Пароль точки доступа

const char* _ssid[wf_configs] ={};
const char* _password[wf_configs] ={};

aREST rest = aREST();
Adafruit_BME280 bme;                  // Датчик

// Код изображения для OLED экрана
static const PROGMEM uint8_t img_bitmap[] = {
  0x00, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x7f, 0x66, 0x86, 0x84, 0x26, 0x26, 0x26, 0x23, 0x03, 0x13, 
  0x13, 0xd3, 0xf3, 0x3e, 0x0c, 0x06, 0x03, 0x01, 0x81, 0xc1, 0xf1, 0x39, 0x3d, 0x7f, 0x1f, 0x80, 
  0xff, 0x01, 0x01, 0x01, 0xff, 0x00, 0x7f, 0x7f, 0x3d, 0x39, 0xe3, 0xc7, 0x8b, 0x19, 0x11, 0xe7, 
  0x8e, 0x1e, 0xe3, 0xc3, 0x81, 0x87, 0x86, 0x82, 0x03, 0x03, 0x07, 0x12, 0x02, 0xfb, 0x1f, 0x1f, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xc0, 0x80, 0x80, 0x80, 0xc0, 0x61, 0x1b, 0x0f, 0x07, 0x02, 0x02, 0x03, 
  0xc3, 0xf3, 0xbb, 0x0f, 0x06, 0x04, 0x06, 0xc7, 0x87, 0x0f, 0x1f, 0x9e, 0x6c, 0x2c, 0x00, 0xff, 
  0x03, 0x00, 0x00, 0x00, 0x3f, 0xf8, 0x00, 0x3e, 0x0c, 0x06, 0x07, 0x07, 0x07, 0x0e, 0x3c, 0x46, 
  0x07, 0x07, 0x81, 0x30, 0x40, 0x08, 0x10, 0x22, 0x43, 0x00, 0x18, 0x0c, 0x0f, 0xcd, 0xf8, 0xf8, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x10, 0x18, 0x24, 0xc2, 0x80, 0x00, 
  0xe4, 0x94, 0x94, 0x7c, 0xf8, 0x00, 0x30, 0x48, 0x84, 0x84, 0xff, 0x84, 0xc8, 0x78, 0x00, 0x30, 
  0xf8, 0xa4, 0xa4, 0xac, 0x38, 0x00, 0x80, 0xf0, 0x8c, 0x84, 0x84, 0xfc, 0x80, 0x00, 0xfc, 0xc8, 
  0x84, 0x84, 0x8c, 0x78, 0x00, 0x00, 0xe4, 0x94, 0x94, 0x7c, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x1f, 0x0f, 0x07, 0x86, 0x84, 0x84, 0xc4, 0x0c, 0x0c, 0x8c, 0xee, 0x77, 
  0x33, 0x30, 0xf0, 0xf2, 0xf2, 0x1e, 0x1a, 0x19, 0x89, 0x8e, 0xcf, 0xce, 0xfc, 0xf0, 0xfe, 0x1f, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf8, 0xf8, 0xfc, 0xcc, 0x8f, 0x8f, 0x8b, 0x1b, 0x1a, 0x1a, 
  0x1a, 0x14, 0x33, 0xf3, 0xe6, 0x64, 0x64, 0x69, 0xce, 0x06, 0x86, 0x86, 0x87, 0x00, 0x00, 0x01, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0xc0, 0xe0, 0xe0, 0xe0, 
  0xe0, 0xe0, 0x20, 0x00, 0x00, 0x00, 0xe1, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe1, 0xe0, 0xe3, 0xe0, 
  0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0x03, 0x01, 0x01, 0xc1, 0x07, 0x0e, 0x07, 0xe1, 0xe0, 
  0x00, 0x00, 0xff, 0xff, 0xff, 0x80, 0x80, 0x80, 0xff, 0xff, 0x80, 0xe0, 0x38, 0x0f, 0x01, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1f, 0x78, 0xc0, 0x80, 0xff, 0xff, 0x80, 0x80, 0xff, 
  0xff, 0x03, 0x03, 0x1f, 0xff, 0x60, 0x40, 0x80, 0x0f, 0x8f, 0x87, 0x01, 0x01, 0xff, 0xff, 0xfe, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0x7f, 0x1f, 0x0f, 0x07, 0x03, 
  0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x81, 0xc1, 
  0xe1, 0xf1, 0xf9, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xc0, 0xe0, 0xf0, 0x7f, 0x7f, 0x7e, 0x7f, 0x3f, 0x3f, 
  0x3e, 0x1e, 0x1f, 0x1f, 0x1f, 0x0f, 0x0f, 0x0f, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x07, 0x07, 0x0f, 0x0f, 0x0f, 
  0x1f, 0x1f, 0x1f, 0x3f, 0x3f, 0x3e, 0x3c, 0x7c, 0x78, 0x79, 0xf1, 0xf0, 0xf0, 0xff, 0xff, 0xff, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x78, 
  0x78, 0x78, 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xc7, 0x83, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x1e, 0x1f, 0x1f, 0x1f, 
  0x1f, 0x3f, 0x3d, 0x3c, 0x7c, 0xf8, 0xf8, 0xf0, 0xf0, 0xe0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x18, 0x38, 0x78, 0x78, 0xf8, 0xf8, 0xf8, 0xd8, 0x98, 
  0x98, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x38, 0x78, 0xf8, 0xe0, 0xc0, 0x80, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xf0, 0xf8, 0x78, 0x38, 0x38, 0x38, 0x38, 0x38, 
  0x38, 0x38, 0x38, 0x38, 0xb8, 0xb8, 0xf8, 0xf8, 0xf8, 0x78, 0x78, 0x38, 0x18, 0xff, 0xff, 0xff, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x0f, 0x3f, 0x7f, 0xff, 0xfe, 0xfc, 0xf8, 0xe0, 0xc0, 0x80, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0xc1, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3c, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x10, 0x1c, 0x99, 0x19, 0x31, 
  0x33, 0x73, 0x77, 0x07, 0x07, 0xe6, 0x4e, 0xce, 0x0e, 0x0e, 0x0c, 0x8c, 0xcf, 0x1f, 0x1f, 0xde, 
  0xdc, 0xd8, 0x10, 0xd8, 0xde, 0xdf, 0xdf, 0x8f, 0x8f, 0xcc, 0xcc, 0xcc, 0xee, 0xce, 0xe6, 0x66, 
  0x26, 0x67, 0x63, 0x03, 0x3b, 0x39, 0x9d, 0x0c, 0x18, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x1f, 0x3f, 0x3f, 
  0x3f, 0x3e, 0x3c, 0x38, 0x20, 0x00, 0x1f, 0x3f, 0x3e, 0x3e, 0x3c, 0x3c, 0x7c, 0x7c, 0x7c, 0x7c, 
  0x7c, 0x7c, 0x3e, 0x3e, 0x3f, 0x1f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x07, 0x0f, 0x0e, 0x0e, 0x1c, 0x1c, 0x03, 0x07, 0x07, 
  0x07, 0x06, 0x0e, 0x70, 0x70, 0x70, 0x70, 0x60, 0x60, 0x1c, 0x1d, 0x1d, 0x1c, 0x1c, 0x1c, 0xe1, 
  0xe1, 0xe1, 0xe0, 0xe1, 0xe0, 0x19, 0x1c, 0x1c, 0x1d, 0x1c, 0x1c, 0xe0, 0xe0, 0x60, 0x70, 0x70, 
  0x70, 0x0e, 0x06, 0x07, 0x07, 0x03, 0x03, 0x1c, 0x1c, 0x1e, 0x0e, 0x0f, 0x07, 0x07, 0x03, 0x01, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void setup() 
{
  Wire.begin();
  Serial.begin(115200);               // инициализируем последовательный порт (для отладочных целей)

  while (!Serial);
    Serial.println("\nI2C Scanner");
    
    uint8_t error;

    // Адреса дисплеев
    // 0x3C (60) - OLED
    // 0x27 - LCD
    // 0x1 - LCD (Вписать нужный)
    uint8_t scanDevice[] = {0x3C, 0x27, 0x1};
 
    Serial.println("Scanning I2C device...");

    for(int address = 0; address < 2; address++ ){
        Wire.beginTransmission(scanDevice[address]);
        error = Wire.endTransmission();
    
        if (error == 0){
            typeDevice = address;
        }              
        else if (error==4) {
            Serial.print("Unknown error");
        }
    } 
    
    Serial.println("I2C detect done\n");
    Serial.print("Display found: ");
    switch (typeDevice) {
      case 0: // OLED
      {
        Serial.println("OLED (0x3C)");
        oled.init();
        oled.clear();
        oled.home();  // курсор в 0,0
        oled.println("    OLED detected  ");
        oled.println("        0x3C       ");
        break;
      }
      case 1: // LCD 1
      {
        Serial.println("LCD (0x27)");
                                      // Модифицированная версия библиотеки LiquidCrystal_I2C_mod.h
        lcd.setAddr(0x27);            // Адрес 0x27 для 1го случая
        lcd.begin();
        lcd.backlight();
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("LCD detect:");
        lcd.setCursor(12,0);
        lcd.print("0x27");
        break;
      }
      case 2: // LCD 2
      {
        Serial.println("LCD (0x?)");
        lcd.setAddr(0x1);             // Внести адрес 0x? для 2го случая
        lcd.begin();
        lcd.backlight();
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("LCD detect:");
        lcd.setCursor(12,0);
        lcd.print("0x?");             // Внести адрес 0x? для 2го случая
        break;
      }
      default:
      {
        Serial.println("Display definition error (main setup)");
        break;
      }
    }
    switch (typeDevice) {
    case 0: // OLED
    {
      oled.setCursor(0, 3);  // курсор в 0,4
      oled.print("Starting...                ");
      break;
    }
    case 1: // 
    case 2: // LCD 
    {
      lcd.setCursor(0,1);
      lcd.print("Starting...                ");
      break;
    }
    default:
    {
      Serial.println("Display definition error (main setup)");
      break;
    }
  }
  delay(2000);

  switch (typeDevice) {
    case 0: // OLED
    {
      oled.setCursor(0, 3);  // курсор в 0,4
      oled.print("SPIFFS init...                ");
      break;
    }
    case 1: // 
    case 2: // LCD 
    {
      lcd.setCursor(0,1);
      lcd.print("SPIFFS init...                ");
      break;
    }
    default:
    {
      Serial.println("Display definition error (main setup)");
      break;
    }
  }
  if (!SPIFFS.begin()) {
    Serial.println("Error: No SPIFFS");
    switch (typeDevice) {
      case 0: // OLED
      {
        oled.setCursor(0, 3);  // курсор в 0,4
        oled.print("SPIFFS error                   ");
        break;
      }
      case 1: // 
      case 2: // LCD 
      {
        lcd.setCursor(0,1);
        lcd.print("SPIFFS error                   ");
        break;
      }
      default:
      {
        Serial.println("Display definition error (main setup)");
        break;
      }
    } 
    return;
  }

  File f = SPIFFS.open("/f_config.txt", "r");
  if (!f)
  {
    Serial.print("Error: file does not exist");

    switch (typeDevice) {
      case 0: // OLED
      {
        oled.setCursor(0, 3);  // курсор в 0,4
        oled.print("SPIFFS file err                ");
        delay(2000);
        break;
      }
      case 1: // 
      case 2: // LCD 
      {
        lcd.setCursor(0,1);
        lcd.print("SPIFFS file err                ");
        delay(2000);
        break;
      }
      default:
      {
        Serial.println("Display definition error (main setup)");
        break;
      }
    }
    
    return;
  }
  else 
  {
    whoami= f.readStringUntil('\n');
    if (whoami.charAt(whoami.length()-1) == 13)
    whoami=whoami.substring(0,whoami.length()-1);

    Akey = f.readStringUntil('\n');
    if (Akey.charAt(Akey.length()-1) == 13)
    Akey=Akey.substring(0,Akey.length()-1);

    ssidAP = f.readStringUntil('\n');
    if (ssidAP.charAt(ssidAP.length()-1) == 13)
    ssidAP=ssidAP.substring(0,ssidAP.length()-1);
  }
  
  HTTP_init();                        //настраиваем HTTP интерфейс
  SSDP_init();                        //запускаем SSDP сервис

  char* irc = new char[strlen("irc") + 1];
  strcpy(irc, "irc");                 
  rest.function(irc, irControl);      // регистрируем в aRest функции irControl
  AREST_init();                       // включаем aREST и сервер к нему 

  //загрузка конфигурации
  LoadConfig(); 
  delay(1000);

  oled.setCursor(0, 3);  // курсор в 0,4
  oled.print("BME init...                ");
  if (!bme.begin(BME280_ADDRESS)) 
  {
    switch (typeDevice) {
      case 0: // OLED
      {
        oled.setCursor(0, 3);  // курсор в 0,4
        oled.print(F("BME Error  ")); //если датчик не найден - выводим ошибку и номер входа мультиплексора, к которому подключен неисправный датчик
        while (1);
        break;
      }
        case 1:
        case 2: // LCD
      {
        lcd.setCursor(0,1);
        lcd.print("BME Error  ");
        while (1);
        break;
      }
      default:
        Serial.println("Display definition error (main setup)");
        break;
      }
   }
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,     //режим работы датчика bme280
                  Adafruit_BME280::SAMPLING_X2,     //точность изм. температуры  
                  Adafruit_BME280::SAMPLING_X16,    //точность изм. давления 
                  Adafruit_BME280::SAMPLING_X2,     //точность изм. влажности 
                  Adafruit_BME280::FILTER_X16,      //уровень фильтрации
                  Adafruit_BME280::STANDBY_MS_0_5); //период просыпания, мСек

  for(size_t i = 0; i < wf_configs; i++) {
      if (ssid[i] != "") 
      {
        count_wifi=count_wifi-1;
      }
  }

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);                //переключение моудля в режим станции
  
  for (int i=0; i<wf_configs;i++)
  {
     wifiMulti.addAP(_ssid[i],_password[i]);
  }

  switch (typeDevice) {
    case 0: // OLED
    {
      oled.setCursor(0, 3);  // курсор в 0,4
      oled.print("Wi-Fi init...                ");
      break;
    }
    case 1: // 
    case 2: // LCD 
    {
      lcd.setCursor(0,1);
      lcd.print("Wi-Fi init...                ");
      break;
    }
    default:
    {
      Serial.println("Display definition error (wifi Reinit)");
      break;
    }
  }

  mainDisplayTimer = millis();
  dispayShowType = 0;
  ReinitWiFi(0);
  delay(500);
}

void loop() 
{
  switch (dispayShowType) {
    case 0: // Основная информация (с датчика)
    {
      if (millis() - mainDisplayTimer >= 2000) {
        switch (typeDevice) {
          case 0: // OLED
          {
            oled.init();
            oled.clear();
            oled.home();  // курсор в 0,0
            oled.print("      Hydra Lite   ");
            oled.println(whoami);
            oled.print(" ");   
            if (temp>0)
            {
              oled.print("+");
            }
            else if (temp=0)
            {
              oled.print(" ");
            }
            oled.print(temp); oled.print(" '"); oled.println("C");
            oled.print(" "); oled.print(pressure); oled.println(" мм рт.ст.");
            oled.print(" "); oled.print(hum); oled.print(" %      "); oled.print(WiFi.RSSI()); oled.println(" дБм");
            oled.update();
            break;
          }
          case 1:
          case 2: // LCD
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("T="); lcd.print(temp,1); lcd.print((char)223); lcd.print("C ");
            lcd.print("H="); lcd.print(hum,1); lcd.print("%");
            lcd.setCursor(0,1);
            lcd.print("P="); lcd.print(pressure,1); lcd.print("mmHg");
            lcd.setCursor(13,1);
            lcd.print((int)(WiFi.RSSI()));
            break;
          default:
            Serial.println("Display definition error (main loop)");
            break;
          }
        }
        mainDisplayTimer = millis();
      }
      break;
    }
    case 1: // wifi 1 стр
    {
      if (millis() - wifiDisplayTimer >= 4000) {
        wifiDisplayTimer = millis();  // обнуляем таймер отображения
        dispayShowType = 2;           // запоминаем 2 стр.
        lcd_v2();                     // переходим на 2 стр.
      }
      break;
    }
      case 2: // wifi 2 стр
    {
      if (millis() - wifiDisplayTimer >= 4000) {
        wifiDisplayTimer = millis();  // обнуляем таймер отображения
        dispayShowType = 3;           // запоминаем 3 стр.
        lcd_v3();                     // переходим на 3 стр.
      }
      break;
    }
    case 3: // wifi 3 стр
    {
      if (millis() - wifiDisplayTimer >= 4000) {
        wifiDisplayTimer = millis();  // обнуляем таймер отображения
        dispayShowType = 0;           // возвразаемся обратно к осн. отображаемой инф.
      }
      break;
    }
    default:
      Serial.println("dispayShowType err");
      break;
    }

  if (!bme.begin(BME280_ADDRESS)) 
  {
    switch (typeDevice) {
      case 0: // OLED
      {
        oled.setCursor(0, 3);  // курсор в 0,4
        oled.print(F("BME Error  ")); //если датчик не найден - выводим ошибку и номер входа мультиплексора, к которому подключен неисправный датчик
        while (1);
        break;
      }
        case 1:
        case 2: // LCD
      {
        lcd.setCursor(0,1);
        lcd.print("BME Error  ");
        while (1);
        break;
      }
      default:
        Serial.println("Display definition error (main loop)");
        break;
      }
  }

  pressure=0;                         // атмосферное давление
  temp=0;                             // температура
  hum=0;                              // влажность
      
  pressure=bme.readPressure()/mmHg; //сумма значений атмосферного давления 
  temp=bme.readTemperature();       //сумма значений температуры
  hum=bme.readHumidity();           //сумма значений влажности

  //передача пакета данных json раз в минуту в базу данных вуза
  //вывод на жисплей и SerialPort название точки доступа, к которой удалось подключиться и ip-адреса раз в минуту
  //проверка/переподключение к сети wi-fi раз в 5 минут
  
  unsigned long mls;
  unsigned long mls_t;
  mls=millis();
  mls_t=mls%PERIOD;

  if(mls_t<oldmillis)
  {
   ReinitWiFi(1);
   oldmillis=mls_t;
  }

  if (millis() - when >= 60000)
  {
    when = millis();
    sendJsonToDB();
    wifiDisplayTimer = millis(); 
    dispayShowType = 1;
    lcd_v();
  }
  HTTP.handleClient();
  delay(1);
  WiFiClient client = SERVERaREST.available();
  if (!client) { return; }
  while (!client.available()) { delay(1); }
  rest.handle(client);
}
