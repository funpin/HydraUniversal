//функции для создания веб-страницы
void SSDP_init(void) 
{
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName("FSWebServer");
  SSDP.setSerialNumber("001788102201");
  SSDP.setURL("/");
  SSDP.setModelName("FSWebServer");
  SSDP.setModelNumber("000000000001");
  SSDP.begin();
}

void HTTP_init(void) 
{
  //SSDP дескриптор
  HTTP.on("/description.xml", HTTP_GET, []() 
  {
    SSDP.schema(HTTP.client());
  });

  //инициализация FFS
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) 
    {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
    }
  }

  //HTTP страницы для работы с FFS
  HTTP.on("/list", HTTP_GET, handleFileList);
  //загрузка редактора editor
  HTTP.on("/edit", HTTP_GET, []() 
  {
    if (!handleFileRead("/edit.html")) HTTP.send(404, "text/plain", "FileNotFound");
  });
  
  //создание файла
  HTTP.on("/edit", HTTP_PUT, handleFileCreate);
  
  //удаление файла
  //HTTP.on("/edit", HTTP_DELETE, handleFileDelete);
  HTTP.on("/edit", HTTP_POST, []() 
  {
    HTTP.send(200, "text/plain", "");
  }, handleFileUpload);

  HTTP.onNotFound([]() 
  {
    if (!handleFileRead(HTTP.uri()))
      HTTP.send(404, "text/plain", "FileNotFound");
  });

  HTTP.on("/ssid", handle_Set_Ssid);          //установить имя и пароль роутера по запросу вида /ssid?ssid=home2&password=12345678
  HTTP.on("/delete", handle_delete_config);
       
  HTTP.on("/restart", handle_Restart);           //перезагрузка модуля по запросу вида /restart?device=ok
  HTTP.on("/c.json", handle_cJSON);              //формирование c.json страницы для передачи данных в web интерфейс
  HTTP.on("/config.json", handle2_cJSON);        //загрузка сохраненной конфигурации    
   
  //обновление
   HTTP.on("/update", HTTP_POST, []()
   {
      HTTP.sendHeader("Connection", "close");
      HTTP.sendHeader("Access-Control-Allow-Origin", "*");
      HTTP.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
      ESP.restart();
    },[](){
      HTTPUpload& upload = HTTP.upload();
      if(upload.status == UPLOAD_FILE_START)
      {
        Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if(!Update.begin(maxSketchSpace)){//start with max available size
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_WRITE)
      {
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_END){
        if(Update.end(true)){ //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      }
      yield();
    });

  HTTP.begin();
}

void AREST_init(void) 
{
  // Определяем имя name и ИД ID устройства aREST
  rest.set_id("1");
  char* aRest = new char[strlen("aRest") + 1];
  strcpy(aRest, "aRest");
  rest.set_name(aRest);
  // Запускаем сервер
  SERVERaREST.begin();
}

//функция для aREST обработки
int irControl(String command) 
{
  unsigned long state = command.toInt();
  return 1;
}


//функции для работы с файловой системой
String getContentType(String filename) 
{
  if (HTTP.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) 
{
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) 
  {
    if (SPIFFS.exists(pathWithGz))
    path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = HTTP.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload() 
{
  if (HTTP.uri() != "/edit") return;
  HTTPUpload& upload = HTTP.upload();
  if (upload.status == UPLOAD_FILE_START) 
  {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;

    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) 
  {
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) 
  {
    if (fsUploadFile)
      fsUploadFile.close();

  }
}

void handleFileDelete() 
{
  if (HTTP.args() == 0) return HTTP.send(500, "text/plain", "BAD ARGS");
  String path = HTTP.arg(0);

  if (path == "/")
  return HTTP.send(500, "text/plain", "BAD PATH");
  
  if (!SPIFFS.exists(path))
  return HTTP.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  HTTP.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate() 
{
  if (HTTP.args() == 0)
  return HTTP.send(500, "text/plain", "BAD ARGS");
  String path = HTTP.arg(0);

  if (path == "/")
  return HTTP.send(500, "text/plain", "BAD PATH");
  
  if (SPIFFS.exists(path))
  return HTTP.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if (file) file.close();
  else return HTTP.send(500, "text/plain", "CREATE FAILED");
  HTTP.send(200, "text/plain", "");
  path = String();
}

void handleFileList() 
{
  if (!HTTP.hasArg("dir")) 
  {
    HTTP.send(500, "text/plain", "BAD ARGS");
    return;
  }
  String path = HTTP.arg("dir");
  Dir dir = SPIFFS.openDir(path);
  path = String();
  String output = "[";
  while (dir.next()) 
  {
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  HTTP.send(200, "text/json", output);
}

//очистка памяти вемоса
void handle_delete_config() {
    for(size_t i = 0; i < wf_configs; ++i) {
      ssid[i] = "";
      password[i] = "";
    }
    saveConfig();
    HTTP.send(200, "text/plain", "OK");
    count_wifi=20;
    HTTP.on("/c.json", handle_cJSON);
    HTTP.on("/config.json", handle2_cJSON);
}

//установка параметров для подключения к внешней AP
void handle_Set_Ssid() 
{
  indexAppendSsid = 0;
  if (!LoadConfig()) {
    for(size_t i = 0; i < wf_configs; ++i) {
      ssid[i] = "";
      password[i] = "";
    }
  } else {
    for(size_t i = 0; i < wf_configs; ++i) {
      if (ssid[i] != "") {
        ++indexAppendSsid;
      }
    }
  }
  String nameSsid = HTTP.arg("ssid");            //получаем значение ssid из запроса сохраняем в глобальной переменной
  String passwordSsid = HTTP.arg("password");    //получаем значение password из запроса сохраняем в глобальной переменной
  HTTP.send(200, "text/plain", "OK");     //отправляем ответ о выполнении
  bool needAppend = true;
  for(size_t i = 0; i < wf_configs; ++i) {
    if(ssid[i] == nameSsid) {
      needAppend = false;
      password[i] = passwordSsid;
      break;
    }
  }
  //indexAppendSsid
  if (needAppend) {
    ssid[indexAppendSsid] = nameSsid;
    password[indexAppendSsid] = passwordSsid;
    indexAppendSsid = (indexAppendSsid >= wf_configs - 1) ? 0 : ++indexAppendSsid;
  }
  saveConfig();                           //функция сохранения данных во Flash
}


//перезагрузка моудля
void handle_Restart() 
{
  String restart = HTTP.arg("device");            //получаем значение device из запроса
  if (restart == "ok") {                          //если значение равно Ок
    HTTP.send(200, "text / plain", "Reset OK");   //отправляем ответ Reset OK

    switch (typeDevice) {
      case 0: // OLED
        oled.clear();
        oled.home();
        oled.print("Restart");
        break;
      case 1: 
      case 2: // LCD 
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Restart");
        break;
      default:
        Serial.println("Display definition error (server)");
        break;
    }
    
    Serial.println();
    Serial.print("Restart");
    ESP.restart();    //перезагружаем модуль
  }
  else {       //иначе
    HTTP.send(200, "text / plain", "No Reset");  // отправляем ответ No Reset

    switch (typeDevice) {
      case 0: // OLED
      {
        oled.clear();
        oled.home();
        oled.print("NO Restart");
        break;
      }
      case 1: 
      case 2: // LCD 
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("NO Restart");
        break;
      }
      default:
      {
        Serial.println("Display definition error (server)");
        break;
      }
    }
  }
}
