#include "BlynkGO_Manager.h"

/**********************************************
 * define , typedef , declare, etc..
 **********************************************/
#define LV_OBJX_NAME  "BlynkGO_Manager"

IMG_DECLARE(icon_admin);

#if BLYNKGO_USE_BLYNK
  IMG_DECLARE(icon_blynk);
#endif //BLYNKGO_USE_BLYNK

IMG_DECLARE(icon_blynkgo);
IMG_DECLARE(icon_clock);
IMG_DECLARE(icon_gps);
IMG_DECLARE(icon_line);
#if defined (TOUCH_SCREEN)
  IMG_DECLARE(icon_touch);
#endif
IMG_DECLARE(icon_wifi);

/**********************************************
 * global variables
 **********************************************/
BlynkGO_Manager blynkgo_manager;
BlynkGO_Manager* pManager=NULL;

uint8_t SETTING_BIT_GROUP;
bool unlock_state = false;
uint32_t unlock_timer;

/**********************************************
 * static variables
 **********************************************/
static uint8_t  LIST_ITEM_ADMIN, LIST_ITEM_WIFI, LIST_ITEM_BLYNK, 
                LIST_ITEM_LINE, LIST_ITEM_NTP, LIST_ITEM_GPS, LIST_ITEM_TOUCH, LIST_ITEM_BLYNKGO;

static lv_signal_cb_t ancestor_signal_cb;

/**********************************************
 * static functions
 **********************************************/
static lv_res_t GSignal_cb(lv_obj_t *obj, lv_signal_t sign, void* param);

extern const void* wifi_signal_symbol(int8_t rssi );

/**********************************************
 * class member functions
 **********************************************/

BlynkGO_Manager::BlynkGO_Manager(GWidget& parent) : GObject(&parent) {}
BlynkGO_Manager::BlynkGO_Manager(GWidget* parent) : GObject(parent) {}
BlynkGO_Manager::~BlynkGO_Manager() { this->del(); }


void BlynkGO_Manager::create() {
  if(this->_par != NULL && !this->_par->isCreated()) this->_par->create();
  if(this->_par->obj == NULL) return;


  if(!this->isCreated()) {
    GObject::create();

    if(this->obj != NULL) {
      this->_created  = true;
      lv_obj_set_user_data(this->obj, this);
      lv_obj_set_event_cb (this->obj, main_event_cb);

      this->size(GScreen);
      this->color(TFT_BLACK);
      
      if( !NVS.exists("KeyLock")) { NVS.setInt("KeyLock", false); }


      blynkgo_manager_ext_t* ext = (blynkgo_manager_ext_t*) this->alloc_ext_attr(sizeof(blynkgo_manager_ext_t)); //this->allocGExtension(sizeof(gicon_ext_t));

      /* alloc ext's GWidgets on PSRAM */
      ext->lb_setting_back    = new_GLabel(this);
      ext->lb_setting_title   = new_GLabel(this);
      ext->list_setting       = new_GList(this);

      ext->lb_setting_back->text("  " SYMBOL_ARROW_LEFT "  ");
      ext->lb_setting_back->position(3,0);
      ext->lb_setting_back->clickable(true);

      ext->lb_setting_back->user_data(this);
      ext->lb_setting_back->onClicked([](GWidget*w) { 
        BlynkGO_Manager* pManager= (BlynkGO_Manager*) w->user_data();
        pManager->hidden(true);
      });
      
      ext->lb_setting_title->text( "ตั้งค่า BlynkGO");
      ext->lb_setting_title->align(ext->lb_setting_back, ALIGN_RIGHT);
      
      ext->list_setting->size( this->width(), this->height()- this->font_height());
      ext->list_setting->position(0, this->font_height() );
      
      ext->list_setting->padding_top(7);                   // ระยะช่องไฟด้านบนของ item
      ext->list_setting->padding_bottom(7);                // ระยะช่องไฟด้านล่างของ item
      ext->list_setting->padding_left(20);                 // ระยะช่องไฟด้านซ้ายของ item
      ext->list_setting->font((BlynkGO.width()==480)? prasanmit_40 : prasanmit_30, TFT_WHITE);
    
      ext->list_setting->style[RELEASED].color(TFT_BLACK);
      ext->list_setting->style[PRESSED].color(TFT_DARKGRAY);

      LIST_ITEM_ADMIN   = ext->list_setting->addItem(icon_admin,    "Admin");
      LIST_ITEM_WIFI    = ext->list_setting->addItem(icon_wifi,     "WiFi");
#if BLYNKGO_USE_BLYNK
      LIST_ITEM_BLYNK   = ext->list_setting->addItem(icon_blynk,    "Blynk");
#endif //BLYNKGO_USE_BLYNK
      LIST_ITEM_LINE    = ext->list_setting->addItem(icon_line,     "LINE Notify");
      LIST_ITEM_NTP     = ext->list_setting->addItem(icon_clock,    "ซิงค์เวลา NTP");
      LIST_ITEM_GPS     = ext->list_setting->addItem(icon_gps,      "พิกัด GPS");
#if defined (TOUCH_SCREEN)
      LIST_ITEM_TOUCH   = ext->list_setting->addItem(icon_touch,    "จอสัมผัส");
#endif // TOUCH_SCREEN
      LIST_ITEM_BLYNKGO = ext->list_setting->addItem(icon_blynkgo,  "BlynkGO");


      ext->list_setting->onClicked([](GWidget* widget) {
        BlynkGO_Manager * pManager = (BlynkGO_Manager *) widget->_par;
        blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) pManager->ext_attr();
        
        uint8_t setting_id = ext->list_setting->item_id(ext->list_setting->item_selected());
        Serial.printf("List Clicked : [%d] %s\n", setting_id, ext->list_setting->item_selected().c_str());

        if(setting_id == LIST_ITEM_ADMIN) {
          pManager->admin_setting();
        }else
        if(setting_id == LIST_ITEM_WIFI) {
          Serial.println("WiFi Setting");
          if(! NVS.getInt("KeyLock")) {
            pManager->wifi_setting();
          }else{
            if( unlock_state && ( millis() >= unlock_timer )) unlock_state = false;

            if( unlock_state ) {
                pManager->wifi_setting();
            }else{
              ext->keyunlocker = new_GKeyUnlocker();
              ext->keyunlocker->size(GScreen);
              ext->keyunlocker->user_data(pManager);
              ext->keyunlocker->mode( GKEYUNLOCKER_MODE_LOCK);
              ext->keyunlocker->onApply([](GWidget* w){
                Serial.println("GKeyUnlocker Apply");
                BlynkGO_Manager* pManager = (BlynkGO_Manager*) w->user_data();
                blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) pManager->user_data();   
                               
                unlock_state = true;
                unlock_timer = millis() + KEYUNLOCK_TIMEOUT;

                pManager->wifi_setting();
                
                bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 1 ) ;
              });
              ext->keyunlocker->onCancel([](GWidget* w){
                Serial.println("GKeyUnlocker Cancel");
                blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();

                bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 1 ) ;
              });
            }
          }
        }
#if BLYNKGO_USE_BLYNK
        else if(setting_id == LIST_ITEM_BLYNK) {
          
          Serial.println("Blynk Setting");
          if(! NVS.getInt("KeyLock")) {
            pManager->blynk_setting();
          }else{
            if( unlock_state && ( millis() >= unlock_timer )) unlock_state = false;

            if( unlock_state ) {
                pManager->blynk_setting();
            }else{
              ext->keyunlocker = new_GKeyUnlocker();
              ext->keyunlocker->size(GScreen);
              ext->keyunlocker->user_data(pManager);
              ext->keyunlocker->mode( GKEYUNLOCKER_MODE_LOCK);
              ext->keyunlocker->onApply([](GWidget* w){
                Serial.println("GKeyUnlocker Apply");
                BlynkGO_Manager* pManager = (BlynkGO_Manager*) w->user_data();
                blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) pManager->user_data();   
                               
                unlock_state = true;
                unlock_timer = millis() + KEYUNLOCK_TIMEOUT;

                pManager->blynk_setting();
                
                bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 1 ) ;
              });
              ext->keyunlocker->onCancel([](GWidget* w){
                Serial.println("GKeyUnlocker Cancel");
                blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();

                bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 1 ) ;
              });
            }
          }
        }
#endif //BLYNKGO_USE_BLYNK        
        else if(setting_id == LIST_ITEM_LINE) {
          Serial.println("LINE Setting");
          if(! NVS.getInt("KeyLock")) {
            pManager->line_setting();
          }else{
            if( unlock_state && ( millis() >= unlock_timer )) unlock_state = false;

            if( unlock_state ) {
                pManager->line_setting();
            }else{
              ext->keyunlocker = new_GKeyUnlocker();
              ext->keyunlocker->size(GScreen);
              ext->keyunlocker->user_data(pManager);
              ext->keyunlocker->mode( GKEYUNLOCKER_MODE_LOCK);
              ext->keyunlocker->onApply([](GWidget* w){
                Serial.println("GKeyUnlocker Apply");
                BlynkGO_Manager* pManager = (BlynkGO_Manager*) w->user_data();
                blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) pManager->user_data();   
                               
                unlock_state = true;
                unlock_timer = millis() + KEYUNLOCK_TIMEOUT;

                pManager->line_setting();
                
                bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 1 ) ;
              });
              ext->keyunlocker->onCancel([](GWidget* w){
                Serial.println("GKeyUnlocker Cancel");
                blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();

                bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 1 ) ;
              });
            }
          }
        }else
        if(setting_id == LIST_ITEM_NTP) {
          Serial.println("NTP Setting");
          if(! NVS.getInt("KeyLock")) {
            pManager->ntp_setting();
          }else{
            if( unlock_state && ( millis() >= unlock_timer )) unlock_state = false;

            if( unlock_state ) {
                pManager->ntp_setting();
            }else{
              ext->keyunlocker = new_GKeyUnlocker();
              ext->keyunlocker->size(GScreen);
              ext->keyunlocker->user_data(pManager);
              ext->keyunlocker->mode( GKEYUNLOCKER_MODE_LOCK);
              ext->keyunlocker->onApply([](GWidget* w){
                Serial.println("GKeyUnlocker Apply");
                BlynkGO_Manager* pManager = (BlynkGO_Manager*) w->user_data();
                blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) pManager->user_data();   
                               
                unlock_state = true;
                unlock_timer = millis() + KEYUNLOCK_TIMEOUT;

                pManager->ntp_setting();
                
                bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 1 ) ;
              });
              ext->keyunlocker->onCancel([](GWidget* w){
                Serial.println("GKeyUnlocker Cancel");
                blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();

                bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 1 ) ;
              });
            }
          }
        }else
        if(setting_id == LIST_ITEM_GPS) {
          Serial.println("GPS Setting");
          if(! NVS.getInt("KeyLock")) {
            pManager->gps_setting();
          }else{
            if( unlock_state && ( millis() >= unlock_timer )) unlock_state = false;

            if( unlock_state ) {
                pManager->gps_setting();
            }else{
              ext->keyunlocker = new_GKeyUnlocker();
              ext->keyunlocker->size(GScreen);
              ext->keyunlocker->user_data(pManager);
              ext->keyunlocker->mode( GKEYUNLOCKER_MODE_LOCK);
              ext->keyunlocker->onApply([](GWidget* w){
                Serial.println("GKeyUnlocker Apply");
                BlynkGO_Manager* pManager = (BlynkGO_Manager*) w->user_data();
                blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) pManager->user_data();   
                               
                unlock_state = true;
                unlock_timer = millis() + KEYUNLOCK_TIMEOUT;

                pManager->gps_setting();
                
                bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 1 ) ;
              });
              ext->keyunlocker->onCancel([](GWidget* w){
                Serial.println("GKeyUnlocker Cancel");
                blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();

                bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 1 ) ;
              });
            }
          }
        }
#if defined (TOUCH_SCREEN)
        else if(setting_id == LIST_ITEM_TOUCH){

          Serial.println("Blynk Setting");
          if(! NVS.getInt("KeyLock")) {
            bitWrite(SETTING_BIT_GROUP, TOUCHCALIBRATE_BIT, 1);
          }else{
            if( unlock_state && ( millis() >= unlock_timer )) unlock_state = false;

            if( unlock_state ) {
              bitWrite(SETTING_BIT_GROUP, TOUCHCALIBRATE_BIT, 1);
            }else{
              ext->keyunlocker = new_GKeyUnlocker();
              ext->keyunlocker->size(GScreen);
              ext->keyunlocker->user_data(pManager);
              ext->keyunlocker->mode( GKEYUNLOCKER_MODE_LOCK);
              ext->keyunlocker->onApply([](GWidget* w){
                Serial.println("GKeyUnlocker Apply");
                BlynkGO_Manager* pManager = (BlynkGO_Manager*) w->user_data();
                blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) pManager->user_data();   
                               
                unlock_state = true;
                unlock_timer = millis() + KEYUNLOCK_TIMEOUT;

                bitWrite(SETTING_BIT_GROUP, TOUCHCALIBRATE_BIT, 1);
                
                bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 1 ) ;
              });
              ext->keyunlocker->onCancel([](GWidget* w){
                Serial.println("GKeyUnlocker Cancel");
                blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();

                bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 1 ) ;
              });
            }
          }
        }
#endif //TOUCH_SCREEN
        else if(setting_id == LIST_ITEM_BLYNKGO){
          pManager->blynkgo_about();
        }
      });

      this->align(ALIGN_CENTER);

      pManager = this;
  
      if(ancestor_signal_cb == NULL) ancestor_signal_cb = this->signal_cb();
      this->signal_cb(GSignal_cb);
    }
  }
}


void BlynkGO_Manager::create(GWidget& parent) {
  this->_par = &parent;
  create();
}

void BlynkGO_Manager::create(GWidget* parent) {
  this->_par = parent;
  create();
}


void BlynkGO_Manager::run(){

  if( SETTING_BIT_GROUP ){
    /*******************************************
     * WiFi Scan Display
     *******************************************/
    if( bitRead(SETTING_BIT_GROUP, WIFI_SCAN_BIT ) ) {
      bitWrite(SETTING_BIT_GROUP, WIFI_SCAN_BIT, 0 ) ;
      Serial.println("WiFi Scan start");
      blynkgo_manager_ext_t* ext = (blynkgo_manager_ext_t*) this->ext_attr();
      
      ext->obj_wifiscan = new_GObject(ext->obj_wifi_setting);
      ext->obj_wifiscan->size(GScreen.width()-30, 220);
      ext->obj_wifiscan->offset(0,10);
      ext->obj_wifiscan->corner_radius(20);
      ext->obj_wifiscan->color(TFT_BLACK);
      ext->obj_wifiscan->border(5,TFT_WHITE);      
      ext->obj_wifiscan->hidden(true);

      // WiFi.scanNetworks will return the number of networks found
      int n = WiFi.scanNetworks();
      Serial.println("scan done");
      if (n == 0) {
        Serial.println("no networks found");
//        btn_wifiscan.mode(BUTTON_MODE_NORMAL);

        ext->obj_wifiscan->hidden(false);

        ext->lb_wifiscan = new_GLabel(ext->obj_wifiscan);
        ext->lb_wifiscan->text( "ไม่พบเครือข่าย WiFi" );

        ext->obj_wifiscan->user_data(ext);
        ext->obj_wifiscan->onClicked([](GWidget* w){
          blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();

          ext->lb_wifiscan->del();    free_widget(ext->lb_wifiscan);
          ext->obj_wifiscan->del();   free_widget(ext->obj_wifiscan);
          MEM_CHECK;

          ext->btn_wifiscan->mode(BUTTON_MODE_NORMAL);
          pManager->wifi_setting_clickable(true);
        });
      } else {
        Serial.print(n);
        Serial.println(" networks found");
        
        ext->obj_wifiscan->hidden(false);

        ext->list_wifiscan = new_GList(ext->obj_wifiscan);
        ext->list_wifiscan->size(ext->obj_wifiscan->width()-20, ext->obj_wifiscan->height()-20);
        ext->list_wifiscan->padding_left(20);                 // ระยะช่องไฟด้านซ้ายของ item
        ext->list_wifiscan->font(prasanmit_30, TFT_WHITE);
      
        ext->list_wifiscan->style[RELEASED].color(TFT_BLACK);
        ext->list_wifiscan->style[RELEASED].image_color(TFT_WHITE);
        ext->list_wifiscan->style[PRESSED].color(TFT_DARKGRAY);

        ext->list_wifiscan->user_data(ext);
        ext->list_wifiscan->onClicked([](GWidget* w){
          blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();
          
          String item_selected =  ext->list_wifiscan->item_selected();
          int index = item_selected.lastIndexOf(" (");
          ext->ta_ssid->text( (index )? item_selected.substring(0, index) : item_selected );

          ext->list_wifiscan->del();  free_widget(ext->list_wifiscan);
          ext->obj_wifiscan->del();   free_widget(ext->obj_wifiscan);

          ext->btn_wifiscan->mode(BUTTON_MODE_NORMAL);
          pManager->wifi_setting_clickable(true);
        });
        
        n = (n > 20)? 20 : n;
        Serial.println("Before1");
        ext->list_wifiscan->hidden(true);
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
          // if( WiFi.SSID(i) == "") continue;
          Serial.println(WiFi.RSSI(i));

          Serial.print(i + 1);
          Serial.print(": ");
          Serial.print(WiFi.SSID(i));
          Serial.print(" (");
          Serial.print(WiFi.RSSI(i));
          Serial.print(")");
          Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");

          ext->list_wifiscan->addItem( wifi_signal_symbol(WiFi.RSSI(i)) , WiFi.SSID(i) + " (" + WiFi.RSSI(i)+ "dBm)");
          delay(10);
        }
      }
      Serial.println("");

      if( ext->list_wifiscan->item_num() ) {
        ext->list_wifiscan->hidden(false);
      }else{
        // Serial.println("Error can't add item to GList");
        ext->list_wifiscan->del();  free_widget(ext->list_wifiscan);

        ext->lb_wifiscan = new_GLabel(ext->obj_wifiscan);
        ext->lb_wifiscan->text( "เกิดข้อผิดพลาด \nปิดแล้วเปิด WiFi ของคุณ ใหม่" );
        ext->lb_wifiscan->text_align(LABEL_ALIGN_CENTER);
        
        ext->obj_wifiscan->user_data(ext);
        ext->obj_wifiscan->onClicked([](GWidget* w){
          blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();

          ext->lb_wifiscan->del();    free_widget(ext->lb_wifiscan);
          ext->obj_wifiscan->del();   free_widget(ext->obj_wifiscan);

          MEM_CHECK;

          ext->btn_wifiscan->mode(BUTTON_MODE_NORMAL);
          pManager->wifi_setting_clickable(true);
        });
      }

    }else
    /*******************************************
     * WiFi Info Display
     *******************************************/
    if( bitRead(SETTING_BIT_GROUP, WIFI_INFO_BIT ) ) {
      bitWrite(SETTING_BIT_GROUP, WIFI_INFO_BIT, 0 ) ;

      blynkgo_manager_ext_t* ext = (blynkgo_manager_ext_t*) this->ext_attr();
      /* ใช้ obj_wifiscan และ lb_wifiscan ในการแสดง wifi_info */
      ext->obj_wifiscan = new_GObject(ext->obj_wifi_setting);
      ext->lb_wifiscan  = new_GLabel(ext->obj_wifiscan);

      ext->obj_wifiscan->size(GScreen.width()-30, 220);
      ext->obj_wifiscan->offset(0,10);
      ext->obj_wifiscan->corner_radius(20);
      ext->obj_wifiscan->color(TFT_BLACK);
      ext->obj_wifiscan->border(5,TFT_WHITE);
      ext->obj_wifiscan->user_data(ext);
      ext->obj_wifiscan->onClicked([](GWidget* w){
        blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();
        ext->lb_wifiscan->del();    free_widget(ext->lb_wifiscan);
        ext->obj_wifiscan->del();   free_widget(ext->obj_wifiscan);
        MEM_CHECK;
        pManager->wifi_setting_clickable(true);
      });

      if( WiFi.isConnected() ) {
        if( WiFi.SSID() != ext->ta_ssid->text()){
          WiFi.disconnect();
          WiFi.begin(pManager->ssid().c_str(), pManager->password().c_str());
          *ext->lb_wifiscan = "Please Waiting for WiFi Connection";
          while(!WiFi.isConnected()) delay(400);
        }
      }

      *ext->lb_wifiscan = StringX::printf("SSID : %s\nBSSID : %s\nGateway : %s\nLocal IP : %s", 
                              WiFi.SSID().c_str(), 
                              WiFi.BSSIDstr().c_str(),
                              WiFi.gatewayIP().toString().c_str(),
                              WiFi.localIP().toString().c_str()
                          );
    }else
    /*******************************************
     * Keyunlocker Del
     *******************************************/
    if( bitRead(SETTING_BIT_GROUP, KEYUNLOCK_BIT ) ) {
      bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 0 ) ;

      blynkgo_manager_ext_t* ext = (blynkgo_manager_ext_t*) this->ext_attr();
      
      ext->keyunlocker->del(); free_widget(ext->keyunlocker);
      MEM_CHECK;
    }
#if defined (TOUCH_SCREEN)
    /*******************************************
     * Touch Calibration
     *******************************************/
    else if( bitRead(SETTING_BIT_GROUP, TOUCHCALIBRATE_BIT) ){
      bitWrite(SETTING_BIT_GROUP, TOUCHCALIBRATE_BIT, 0);

      this->hidden(true);
      BlynkGO.touch_calibrate();
      this->hidden(false);
      MEM_CHECK;
    }
#endif
  }
}

void BlynkGO_Manager::hidden(bool enable){
  GObject::hidden(enable);
  if(enable==false) GObject::toForeground();
}

bool BlynkGO_Manager::hidden(){
  return GObject::hidden();
}

String BlynkGO_Manager::ssid() {
  String ssid= "";
  if( NVS.exists(KEY_WIFI_SSID)) ssid = NVS.getString(KEY_WIFI_SSID);
  return ssid;
}

void  BlynkGO_Manager::ssid(String ssid){
  NVS.setString(KEY_WIFI_SSID, ssid);
}

String BlynkGO_Manager::password(){
  String pass= "";
  if( NVS.exists(KEY_WIFI_PASS)) pass = NVS.getString(KEY_WIFI_PASS);
  return pass;
}

void BlynkGO_Manager::password(String password){
  NVS.setString(KEY_WIFI_PASS, password);
}

#if BLYNKGO_USE_BLYNK
void BlynkGO_Manager::blynk_server(String server){
  NVS.setString(KEY_BLYNK_HOST, server);
}

String BlynkGO_Manager::blynk_server(){
  String server= "";
  if( NVS.exists(KEY_BLYNK_HOST)) server = NVS.getString(KEY_BLYNK_HOST);
  return server;
}

void BlynkGO_Manager::blynk_port(uint16_t port){
  NVS.setInt(KEY_BLYNK_PORT, (uint16_t) port);
}

uint16_t BlynkGO_Manager::blynk_port(){
  uint16_t port = 0;
  if( NVS.exists(KEY_BLYNK_PORT)) port = (uint16_t)NVS.getInt(KEY_BLYNK_PORT);
  return port;
}

void BlynkGO_Manager::blynk_auth(String auth){
  NVS.setString(KEY_BLYNK_AUTH, auth );
}

String BlynkGO_Manager::blynk_auth(){
  String auth = "";
  if( NVS.exists(KEY_BLYNK_AUTH)) auth = NVS.getString(KEY_BLYNK_AUTH);
  return auth;
}
#endif //BLYNKGO_USE_BLYNK

void BlynkGO_Manager::line_token(String token){
  NVS.setString(KEY_LINE_TOKEN, token);
}

String BlynkGO_Manager::line_token(){
  String token = "";
  if( NVS.exists(KEY_LINE_TOKEN)) token = NVS.getString(KEY_LINE_TOKEN);
  return token;
}

/**********************************************
 * static functions
 **********************************************/
static lv_res_t GSignal_cb(lv_obj_t *obj, lv_signal_t sign, void* param)
{
  lv_res_t res;
  res = ancestor_signal_cb( obj, sign, param);
  if(res != LV_RES_OK ) return res;
  if(sign == LV_SIGNAL_GET_TYPE) 
    return lv_obj_handle_get_type_signal((lv_obj_type_t*)param, LV_OBJX_NAME);

  /* หลัง del() หรือ deconstrutor ถูกเรียก และภายในได้ลบ child ไปแล้ว */
  if(sign == LV_SIGNAL_CLEANUP) {
    Serial.println("[BlynkGO_Manager] signal cleanup");
    BlynkGO_Manager* pManager = (BlynkGO_Manager*) lv_obj_get_user_data(obj);

    blynkgo_manager_ext_t* ext = (blynkgo_manager_ext_t*)  pManager->ext_attr();
    
    ext->lb_setting_back->del();      free_widget(ext->lb_setting_back);
    ext->lb_setting_title->del();     free_widget(ext->lb_setting_title);
    ext->list_setting->del();         free_widget(ext->list_setting);

    if(pManager->hasLocalStyle()) {pManager->freeLocalStyle(); } //pIcon->_has_localstyle = false; } // 
    pManager->_created = false;
  }
  return LV_RES_OK;
}
