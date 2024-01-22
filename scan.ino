//сканирование точек досутпа
void scan()
{
    int r = WiFi.scanNetworks();
    int *rssi_r = new int[r];
    String *ssid_r=new String[r];
    Serial.print("Scan start ... "); //запускаем сканирование
    Serial.print(r); //количество найденных точек доступа
    Serial.println(" network(s) found");  
    for (int i = 0; i < r; i++)
    {
        ssid_r[i]=WiFi.SSID(i);
        rssi_r[i]=WiFi.RSSI(i);
        Serial.println();
        Serial.print(WiFi.SSID(i));
        Serial.print("    ");
        Serial.print(WiFi.RSSI(i));
        Serial.print("   dBm");
    }
  sort_2(rssi_r, ssid_r,r);
  Serial.println();
  Serial.print("SORT:");
  int t=0;
  if (r<wf) t=r;
  else t=wf;
  for (int i=0;i<t;i++)
  {
    ssid_scan[i]=ssid_r[i];
    rssi[i]=rssi_r[i];
    Serial.println();
    Serial.print(ssid_scan[i]);
    Serial.print("  ");
    Serial.print(rssi[i]);
    Serial.print("   dBm");
  }
  Serial.println();
  delay(5000);
  delete [] rssi_r; // очистка памяти
  delete [] ssid_r;
}
