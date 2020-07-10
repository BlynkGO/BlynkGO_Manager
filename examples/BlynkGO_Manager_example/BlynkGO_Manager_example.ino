#include <BlynkGO.h>
#include <BlynkGO_Manager.h>

#define BLYNKGO_KEY           "----------------"

BlynkGO_Manager blynkgo_manager;

time_t now_timestamp; 
struct tm tm;

void setup() {
  Serial.begin(115200); Serial.println();
  BlynkGO.begin(BLYNKGO_KEY);

  blynkgo_manager.create();
  blynkgo_manager.hidden(true);

  WiFi.begin(blynkgo_manager.ssid().c_str(), blynkgo_manager.password().c_str());  
}

void loop() {
  BlynkGO.update();
  blynkgo_manager.run();
}

WIFI_CONNECTED(){
  Serial.print("WiFi Connected. IP : ");
  Serial.println(WiFi.localIP());
}

WIFI_DISCONNECTED(){
  Serial.println("WiFi Disconnected");
}


NTP_SYNCED(){
  static GTimer tm_timer;
  // ตั้งให้ update เวลา tm ทุกๆ 1 วินาที  (สามารถเรียกใช้ tm ได้เลย)
  tm_timer.setInterval(1000L,[](){  
  	time(&now_timestamp); localtime_r(&now_timestamp, &tm);
  });
}

