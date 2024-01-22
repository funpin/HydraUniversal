//пакет json для передачи всей нужной информации в базу данных
void sendJsonToDB()
{
              
  String json;
  json  =  "{\"system\":{ ";
  //json += "\"Akey\":\"PBarK3_2\", ";
  //json += "\"Serial\":\"02\", ";
  json += "\"Akey\":\"" + String(Akey) + "\",";
  json += "\"Serial\":\"" + String(whoami) + "\", ";
  json += "\"Version\":\"2024-01-11\", ";
  json += "\"RSSI\":\"" + String(WiFi.RSSI()) + "\",";
  json += "\"MAC\":\"" + String(WiFi.macAddress()) + "\",";
  json += "\"IP\":\""; json+= (String)myIP[0] + String(".") + (String)myIP[1] + String(".") +(String)myIP[2] + String(".") + (String)myIP[3]; json += "\"},";

  json += "\"weather\":{ ";
  json += "\"temp\":"+String(temp)+",";
  json += "\"humidity\":"+String(hum)+",";
  json += "\"pressure\":"+String(pressure)+"} }"; 

  Serial.print(json);

  String apiGetData = "http://dbrobo.mgul.ac.ru/core/jsonadd.php";
  http.begin(wifiClient, apiGetData);
  http.addHeader("Content-Type", "application/json");
  int resulthttp = http.POST(json);
  Serial.print("Done: ");
  Serial.println(resulthttp);
  http.end();
}

//для передачи данных на web-страницу
void handle_cJSON() 
{
  String json_2;
  json_2 = "{";  // Формировать строку для отправки в браузер json формат 
  json_2 += "\"ssidName"+String(1)+"\":\"";
  json_2 += ssid_scan[0];
  json_2 += "\",\"signalStrength"+String(1)+"\":\"";
  json_2 += rssi[0];
  json_2 += " dBm";
 for (int i=1;i<wf;i++)
 {
  json_2 += "\",\"ssidName"+String(i+1)+"\":\"";
  json_2 += ssid_scan[i];
  json_2 += "\",\"signalStrength"+String(i+1)+"\":\"";
  json_2 += rssi[i];
  json_2 += " dBm";
 }
 json_2 +="\",\"count_wf\":\"";
 json_2 += count_wifi;
 json_2 += "\"}";
 HTTP.send(200, "text/json", json_2);
}

void handle2_cJSON() 
{
  File READconfigFile = SPIFFS.open("/configs.json", "r");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& NEWroot = jsonBuffer.parseObject(READconfigFile);
  String json_3;
  json_3 = "{";  // Формировать строку для отправки в браузер json формат 
  json_3 += "\"savessidName"+String(1)+"\":\"";
  json_3 += NEWroot["ssidName"+String(1)].as<String>();
  json_3 += "\",\"savessidPassword"+String(1)+"\":\"";
  json_3 += NEWroot["ssidPassword"+String(1)].as<String>();
 for (int i=1;i<wf;i++)
 {
  json_3 += "\",\"savessidName"+String(i+1)+"\":\"";
  json_3 += NEWroot["ssidName"+String(i+1)].as<String>();
  json_3 += "\",\"savessidPassword"+String(i+1)+"\":\"";
  json_3 += NEWroot["ssidPassword"+String(i+1)].as<String>();
 }
 json_3 += "\"}";
 HTTP.send(200, "text/json", json_3);
}
