#include "BlynkGO_Manager.h"


/**********************************************
 * static variables
 **********************************************/

static lv_signal_cb_t ancestor_wifi_setting_signal_cb;

/**********************************************
 * static functions
 **********************************************/
static lv_res_t wifi_setting_signal_cb(lv_obj_t *obj, lv_signal_t sign, void* param);

/**********************************************
 * class member functions
 **********************************************/

void BlynkGO_Manager::wifi_setting(bool standalone){
  this->hidden(false);

  blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) this->ext_attr();

  ext->child_setting_standalone = standalone;
  ext->obj_wifi_setting         = new_GObject(this);
    ext->lb_wifi_setting_back   = new_GLabel(ext->obj_wifi_setting);
    ext->lb_wifi_setting_title  = new_GLabel(ext->obj_wifi_setting);
    ext->btn_wifiscan           = new_GButton(ext->obj_wifi_setting);
    ext->page_wifi_setting_body = new_GPage(ext->obj_wifi_setting);
      ext->lb_ssid_info         = new_GLabel(ext->page_wifi_setting_body);
      ext->lb_pass_info         = new_GLabel(ext->page_wifi_setting_body);
      ext->ta_ssid              = new_GTextArea(ext->page_wifi_setting_body);
      ext->ta_password          = new_GTextArea(ext->page_wifi_setting_body);
      ext->cont_buttons         = new_GContainer(ext->page_wifi_setting_body);
      ext->btn_wifi_qrcode      = new_GButton(ext->cont_buttons);
      ext->btn_wifi_info        = new_GButton(ext->cont_buttons);

  ext->obj_wifi_setting->size(GScreen);
  ext->obj_wifi_setting->color(TFT_BLACK);

  ext->lb_wifi_setting_back->text("  " SYMBOL_ARROW_LEFT "  ");
  ext->lb_wifi_setting_back->position(3,0);
  ext->lb_wifi_setting_back->clickable(true);
  ext->lb_wifi_setting_back->user_data(this);
  ext->lb_wifi_setting_back->onClicked([](GWidget* w){  
    BlynkGO_Manager* pManager = (BlynkGO_Manager*) w->user_data();
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) pManager->ext_attr();
    if(ext->child_setting_standalone) pManager->hidden(true);

    free_widget(ext->obj_wifi_setting);

    // ขยายเวลาให้ unlock หาก system keylock เป็นโหมด lock
    if( NVS.getInt("KeyLock") ){
      unlock_state = true;
      unlock_timer = millis() + KEYUNLOCK_TIMEOUT;
    }
    MEM_CHECK; 

#if BLYNKGO_USE_WIFI
    if(WiFi.isConnected() && WiFi.SSID() != pManager->ssid() ) {
      WiFi.disconnect(true, true);
      WiFi.begin(pManager->ssid().c_str(), pManager->password().c_str());
    }
#endif //BLYNKGO_USE_WIFI
  });

  ext->lb_wifi_setting_title->text("ตั้งค่า WiFi");
  ext->lb_wifi_setting_title->align(ext->lb_wifi_setting_back, ALIGN_RIGHT);

  ext->page_wifi_setting_body->size(ext->obj_wifi_setting->width(), ext->obj_wifi_setting->height()- ext->lb_wifi_setting_title->bottomY()+5);
  ext->page_wifi_setting_body->position(0, ext->lb_wifi_setting_title->bottomY()-5);
  ext->page_wifi_setting_body->opa(0);
  ext->page_wifi_setting_body->corner_radius(0);
  ext->page_wifi_setting_body->border(0);
  ext->page_wifi_setting_body->font((GScreen.width()==480)? prasanmit_35: prasanmit_25, TFT_WHITE);

  ext->btn_wifiscan->text("สแกน WiFi");
  ext->btn_wifiscan->font(prasanmit_25, TFT_WHITE);
  ext->btn_wifiscan->height(45);
  ext->btn_wifiscan->rounded(true);
  ext->btn_wifiscan->align(ALIGN_TOP_RIGHT, -20, 5);
  ext->btn_wifiscan->user_data(ext);
  ext->btn_wifiscan->onClicked([](GWidget* w){
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();
    Serial.println("WiFi Scan!!");
    bitWrite(SETTING_BIT_GROUP, WIFI_SCAN_BIT, 1 ) ;

    ext->btn_wifiscan->mode(BUTTON_MODE_INACTIVE);
    pManager->wifi_setting_clickable(false);
  });

  ext->lb_ssid_info->text("SSID");
//  ext->lb_ssid_info->position( 20, ext->lb_wifi_setting_title->bottomY() + 10);
//  ext->lb_ssid_info->position( 20,-5);

  ext->ta_ssid->width(BlynkGO.width()-40);
  ext->ta_ssid->placeholder("WiFi SSID");
  ext->ta_ssid->cursor_type(CURSOR_NONE);
  ext->ta_ssid->align(ext->lb_ssid_info, ALIGN_BOTTOM_LEFT);

  ext->ta_ssid->user_data(ext);
  ext->ta_ssid->onValueChanged([](GWidget* w){
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();
    Serial.printf("SSID : %s\n", ext->ta_ssid->text().c_str());
    String ssid = ext->ta_ssid->text();
    NVS.setString(KEY_WIFI_SSID, ssid);

    ext->btn_wifi_qrcode->mode(( ext->ta_ssid->text() == "")? BUTTON_MODE_INACTIVE : BUTTON_MODE_NORMAL);
  });

  ext->lb_pass_info->text("พาสเวิร์ด");
//  ext->lb_pass_info->align(ext->ta_ssid, ALIGN_BOTTOM_LEFT, 0, 20);

  ext->ta_password->width(ext->ta_ssid);
  ext->ta_password->password_mode(true);
  ext->ta_password->placeholder("WiFi Password");
  ext->ta_password->cursor_type(CURSOR_NONE);
//  ext->ta_password->align(ext->lb_pass_info, ALIGN_BOTTOM_LEFT);


  ext->ta_password->user_data(ext);
  ext->ta_password->onValueChanged([](GWidget* w){
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();
    Serial.printf("Password : %s\n", ext->ta_password->text().c_str());
    NVS.setString(KEY_WIFI_PASS, ext->ta_password->text());
  });

//  ext->page_wifi_setting_body->height( ext->ta_password->bottomY() + 100);
  ext->cont_buttons->fit2(FIT_NONE, FIT_TIGHT);
  ext->cont_buttons->width( ((GPage*)ext->cont_buttons->_par)->width_fit()-10);
  ext->cont_buttons->layout(LAYOUT_PRETTY);//LAYOUT_ROW_M);
  ext->cont_buttons->border(0);
  ext->cont_buttons->opa(0);
  // ext->cont_buttons->padding_left(20);

  ext->btn_wifi_qrcode->text("QR Code");
  ext->btn_wifi_qrcode->font(prasanmit_25, TFT_WHITE);
  ext->btn_wifi_qrcode->color(TFT_GREEN);
  ext->btn_wifi_qrcode->height(45);
  ext->btn_wifi_qrcode->rounded(true);
//  ext->btn_wifi_qrcode->align(ext->ta_password, ALIGN_BOTTOM_LEFT, 30, 20);
  ext->btn_wifi_qrcode->user_data(ext);
  ext->btn_wifi_qrcode->onClicked([](GWidget* w){
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();

    pManager->wifi_setting_clickable(false);
    // ext->lb_wifi_setting_back->clickable(false);
    // ext->btn_wifiscan->clickable(false);
    // ext->btn_wifi_qrcode->clickable(false);
    // ext->btn_wifi_info->clickable(false);
    // ext->ta_ssid->clickable(false);
    // ext->ta_password->clickable(false);

    String ssid, pass;
    if( NVS.exists(KEY_WIFI_SSID)) ssid = NVS.getString(KEY_WIFI_SSID);
    if( NVS.exists(KEY_WIFI_PASS)) ssid = NVS.getString(KEY_WIFI_PASS);
    if( ssid != "") {
      ext->qrcode = new_GQRCode(ext->obj_wifi_setting);
      ext->qrcode->encodeWiFi( ssid, pass);
      ext->qrcode->user_data(ext);
      ext->qrcode->onClicked([](GWidget* w) {
        blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();
        pManager->wifi_setting_clickable(true);
        ext->qrcode->del(); free_widget(ext->qrcode);
      });
    }
  });

  ext->btn_wifi_info->text("WiFi Info");
  ext->btn_wifi_info->font(prasanmit_25, TFT_WHITE);
  ext->btn_wifi_info->color(TFT_ORANGE);
  ext->btn_wifi_info->height(45);
  ext->btn_wifi_info->rounded(true);
  ext->btn_wifi_info->user_data(ext);
  ext->btn_wifi_info->onClicked([](GWidget* w){
    Serial.println("WiFi Info!!");
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();

    bitWrite(SETTING_BIT_GROUP, WIFI_INFO_BIT, 1 ) ;

    pManager->wifi_setting_clickable(false);

  });

  if( NVS.exists(KEY_WIFI_SSID)) {
    ext->ta_ssid->text(NVS.getString(KEY_WIFI_SSID));
    ext->btn_wifi_qrcode->mode(BUTTON_MODE_NORMAL);
  }else{
    ext->btn_wifi_qrcode->mode(BUTTON_MODE_INACTIVE);
  }

  if( NVS.exists(KEY_WIFI_PASS)) 
    ext->ta_password->text(NVS.getString(KEY_WIFI_PASS));


  if(ancestor_wifi_setting_signal_cb == NULL) ancestor_wifi_setting_signal_cb = ext->obj_wifi_setting->signal_cb();
  ext->obj_wifi_setting->signal_cb(wifi_setting_signal_cb);
}

void BlynkGO_Manager::wifi_setting_clickable(bool enable){
  blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) this->ext_attr();

  ext->lb_wifi_setting_back->clickable(enable);
  ext->btn_wifiscan->clickable(enable);
  ext->btn_wifi_qrcode->clickable(enable);
  ext->btn_wifi_info->clickable(enable);
  ext->ta_ssid->clickable(enable);
  ext->ta_password->clickable(enable);
}

/**********************************************
 * static functions
 **********************************************/
static lv_res_t wifi_setting_signal_cb(lv_obj_t *obj, lv_signal_t sign, void* param)
{
  lv_res_t res;
  res = ancestor_wifi_setting_signal_cb( obj, sign, param);
  if(res != LV_RES_OK ) return res;
  if(sign == LV_SIGNAL_GET_TYPE) 
    return lv_obj_handle_get_type_signal((lv_obj_type_t*)param, "");

  /* หลัง del() หรือ deconstrutor ถูกเรียก และภายในได้ลบ child ไปแล้ว */
  if(sign == LV_SIGNAL_CLEANUP) {
    Serial.println("[WiFi Setting] signal cleanup");
    GObject* obj_wifi_setting = (GObject*) lv_obj_get_user_data(obj);
    BlynkGO_Manager* pManager = (BlynkGO_Manager*) obj_wifi_setting->_par;
    blynkgo_manager_ext_t* ext = (blynkgo_manager_ext_t*)  pManager->ext_attr();
    
    free_widget(ext->btn_wifi_qrcode);
    free_widget(ext->btn_wifi_info);
    free_widget(ext->cont_buttons);

    free_widget(ext->btn_wifiscan);
    free_widget(ext->ta_password);
    free_widget(ext->ta_ssid);
    free_widget(ext->lb_pass_info);
    free_widget(ext->lb_ssid_info);
    free_widget(ext->page_wifi_setting_body);
    free_widget(ext->lb_wifi_setting_title);
    free_widget(ext->lb_wifi_setting_back);

    if(ext->obj_wifi_setting->hasLocalStyle()) {ext->obj_wifi_setting->freeLocalStyle(); } //pIcon->_has_localstyle = false; } // 
    ext->obj_wifi_setting->_created = false;

    MEM_CHECK;
  }
  return LV_RES_OK;
}


/**********************************************
 * global functions
 **********************************************/

const void* wifi_signal_symbol(int8_t rssi ){
  int8_t wifi_signal =  constrain(100+ rssi, 0, 90);
//  Serial.printf("[WiFi] signal : %d\n", wifi_signal);

  if( wifi_signal >=0 && wifi_signal < 30 ){
    return SYMBOL_WIFI_1;
  }else
  if( wifi_signal >=30 && wifi_signal < 40 ){
    return SYMBOL_WIFI_2;
  }else
  if( wifi_signal >=40 && wifi_signal < 50 ){
    return SYMBOL_WIFI_3;
  }else
  if( wifi_signal >=50 ){
    return SYMBOL_WIFI_4;
  }
}
