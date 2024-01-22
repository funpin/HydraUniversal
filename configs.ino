//загрузка данных сохраненных в файл  configs.json
bool LoadConfig() 
{
  switch (typeDevice) {
    case 0: // OLED
    {
      oled.setCursor(0, 3);  // курсор в 0,4
      oled.print("Load Config...                ");
      break;
    }
    case 1: // 
    case 2: // LCD 
    {
      lcd.setCursor(0,1);
      lcd.print("Load Config...                ");
      break;
    }
    default:
    {
      Serial.println("Display definition error (server ssdp)");
      break;
    }
  }
  //открываем файл для чтения
  File configFile = SPIFFS.open("/configs.json", "r");
  if (!configFile) 
  {
    //если файл не найден  
    Serial.println("Failed to open config file");
    //создаем файл запиав в него данные по умолчанию
    saveConfig();
    configFile.close();
    return false;
  }
  //проверяем размер файла, будем использовать файл размером меньше 1024 байта
  size_t size = configFile.size();
  if (size > 1024) 
  {
    Serial.println("Config file size is too large");
    configFile.close();
    return false;
  }

  //загружаем файл конфигурации в глобальную переменную
  Serial.println("Read config");
  jsonConfig = configFile.readString();
  configFile.close();
  //резервируем память для json обекта буфер может рости по мере необходимти предпочтительно для ESP8266 
  DynamicJsonBuffer jsonBuffer;
  //вызовите парсер JSON через экземпляр jsonBuffer
  //строку возьмем из глобальной переменной String jsonConfig
  JsonObject& root = jsonBuffer.parseObject(jsonConfig);

  //теперь можно получить значения из root
  for (int i=0; i<wf_configs; i++)
  {  
    ssid[i] = root["ssidName"+String(i+1)].as<String>();
    password[i] = root["ssidPassword"+String(i+1)].as<String>();
    _ssid[i] = ssid[i].c_str();
    _password[i] = password[i].c_str();
    Serial.println(_ssid[i]);
    Serial.println(_password[i]);
  }
  return true;
}

//функция сохранения данных во Flash
bool saveConfig() 
{
  //резервируем память для json обекта буфер может рости по мере необходимти предпочтительно для ESP8266 
  DynamicJsonBuffer jsonBuffer;
  //вызовем парсер JSON через экземпляр jsonBuffer
  JsonObject& json = jsonBuffer.parseObject(jsonConfig);
  
  //заполняем поля json 
  for (int i=0; i<wf_configs; i++)
  {
    json["ssidName"+String(i+1)] = ssid[i];
    json["ssidPassword"+String(i+1)] = password[i];
  }
  
  //помещаем созданный json в глобальную переменную json.printTo(jsonConfig);
  json.printTo(jsonConfig);
  //открываем файл для записи
  File configFile = SPIFFS.open("/configs.json", "w+");
  Serial.println("OPEN FILE /configs.json W");
  if (!configFile) 
  {
    Serial.println("ERROR FILE");
    configFile.close();
    return false;
  }
  //записываем строку json в файл 
  json.printTo(configFile);
  configFile.close();
  return true;
}
