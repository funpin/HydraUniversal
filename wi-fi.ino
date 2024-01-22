//(Пере)инициализировать WiFi
void ReinitWiFi(int n)           
{
  if(n==1 && (wifiMulti.run() == WL_CONNECTED))
  {
    // всё в порядке
    Serial.print("OK");
    //return;
  }

  if (n==0 && (wifiMulti.run() != WL_CONNECTED))
  {
    return;
  }
  
  Serial.print(F("\nSearch Wi-Fi AP: "));
  //оборвать прежнюю связь
  Serial.print(F("#"));
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  delay(300);

  //попытаться войти в связь (адреса, пароли, явки заданы ранее)
  WiFi.mode(WIFI_STA);

  lcd.setCursor(0,0);
  switch (typeDevice) {
    case 0: // OLED
      oled.clear();
      oled.home();
      oled.print("      Hydra Lite   "); 
      oled.println(whoami);
      oled.println("Connecting...");
      break;
    case 1: 
    case 2: // LCD 
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Connecting...");
      lcd.setCursor(0,1);
      break;
    default:
      Serial.println("Display definition error (wi-fi)");
      break;
  }

  for(int i=0;i<wf_configs;i++)
  {
    if(wifiMulti.run() != WL_CONNECTED) 
    {
      delay(300);
      Serial.print(F("*"));
      
      switch (typeDevice) {
        case 0: // OLED
          oled.print(F("*"));
          break;
        case 1: 
        case 2: // LCD 
          lcd.leftToRight();
          lcd.print("*");
          break;
        default:
          Serial.println("Display definition error (wi-fi loop)");
          break;
      }
    } else
      {
        Serial.println(F(" Connecting"));
        myIP=WiFi.localIP();
        mySSID=WiFi.SSID();
        myAP="STA"; 
     }
   }
   
  if(wifiMulti.run() != WL_CONNECTED)  //создание собственной точки доступа 
  {  
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    delay(500);
    WiFi.mode(WIFI_AP);
    if(WiFi.softAP(ssidAP))
    {
      myIP=WiFi.softAPIP();
    }
    else
    {
      myIP=IPAddress(0,0,0,0);
    }
    mySSID=ssidAP;
    myAP="AP";
  }
  lcd_v();
  // Перезапускаем сервер
  SERVERaREST.begin();
  //сканирование сетей (происходит уже после попытки подлкючения)
  scan();
  delay(1000);
}

//вывод на lcd-экран и в монитор порта название точки доступа, к которой удалось подключиться и ip-адреса
void lcd_v()  
{
  Serial.println();
  Serial.print("WiFi SSID:");
  Serial.println(mySSID);
  Serial.print("WiFi IP:");
  Serial.println(myIP);
  Serial.print("Mode:");
  Serial.println(myAP);

  switch (typeDevice) {
    case 0: // OLED
    {
      oled.init();
      oled.clear();
      oled.home();
      oled.print("      Hydra Lite   ");  oled.println(whoami);
      oled.print("SSID: "); oled.println(mySSID);
      oled.print("IP: ");   oled.println(myIP);
      oled.print("Mode: "); oled.print(myAP);
      delay(5250);
      oled.clear();
      oled64.init();
      oled64.drawBitmap(0, 0, img_bitmap, 128, 64, BITMAP_NORMAL, BUF_ADD);
      delay(5250);
      break;
    }
    case 1: 
    case 2: // LCD 
    {
      lcd.clear();
      lcd.setCursor(0,0); lcd.print("Hydra-L1");
      lcd.setCursor(10,0); lcd.print("#");
      lcd.setCursor(11,0); lcd.print(whoami);
      lcd.setCursor(0,1); lcd.print("RSSI:");
      lcd.setCursor(6,1); lcd.print((int)(WiFi.RSSI()));
      lcd.setCursor(10,1); lcd.print("dBm");
      delay(3500);
      lcd.clear();
      lcd.setCursor(0,0); lcd.print("Mode:");
      lcd.setCursor(6,0); lcd.print(myAP);
      lcd.setCursor(0,1); lcd.print(myIP);
      delay(3500);
      lcd.clear();
      lcd.setCursor(0,0); lcd.print("SSID:");
      lcd.setCursor(0,1); lcd.print(mySSID);
      delay(3500);
      break;
    }
    default:
    {
      Serial.println("Display definition error (server)");
      break;
    }
  }
}
