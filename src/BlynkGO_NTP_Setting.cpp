#include "BlynkGO_Manager.h"

/**********************************************
 * static variables
 **********************************************/

static lv_signal_cb_t ancestor_ntp_setting_signal_cb;


/**********************************************
 * static functions
 **********************************************/
static lv_res_t ntp_setting_signal_cb(lv_obj_t *obj, lv_signal_t sign, void* param);

/**********************************************
 * class member functions
 **********************************************/
void BlynkGO_Manager::ntp_setting(bool standalone){
  this->hidden(false);
#if BLYNKGO_USE_NTP
  if( !NVS.exists("NTP_SERVER"))  { NVS.setString("NTP_SERVER" , NTP_HOSTNAME ); }
  if( !NVS.exists("TIMEZONE"))    { NVS.setFloat("TIMEZONE"    , (float) NTP_TIMEZONE ); }
  if( !NVS.exists("DAYLIGHT"))    { NVS.setInt("DAYLIGHT"      , (int8_t) NTP_DAYLIGHT_SAVING  ); }
#endif
  
  blynkgo_manager_ext_t* ext  = (blynkgo_manager_ext_t*) this->ext_attr();
  ext->child_setting_standalone = standalone;
  ext->obj_ntp_setting          = new_GObject(this);
    ext->lb_ntp_setting_back    = new_GLabel(ext->obj_ntp_setting);
    ext->lb_ntp_setting_title   = new_GLabel(ext->obj_ntp_setting);    
    ext->page_ntp_seting_body   = new_GPage(ext->obj_ntp_setting);
      ext->lb_ntp_server_info   = new_GLabel(ext->page_ntp_seting_body);
      ext->lb_ntp_timezone_info = new_GLabel(ext->page_ntp_seting_body);
      ext->lb_ntp_daylight_info = new_GLabel(ext->page_ntp_seting_body);
      ext->ta_ntp_server        = new_GTextArea(ext->page_ntp_seting_body);
      ext->ta_ntp_timezone      = new_GTextArea(ext->page_ntp_seting_body);
      ext->ta_ntp_daylight      = new_GTextArea(ext->page_ntp_seting_body);

  ext->obj_ntp_setting->size(GScreen);
  ext->obj_ntp_setting->color(TFT_BLACK);

  ext->lb_ntp_setting_back->text("  " SYMBOL_ARROW_LEFT "  ");
  ext->lb_ntp_setting_back->position(3,0);
  ext->lb_ntp_setting_back->clickable(true);
  ext->lb_ntp_setting_back->user_data(this);
  ext->lb_ntp_setting_back->onClicked([](GWidget* w){ 
    BlynkGO_Manager* pManager = (BlynkGO_Manager*) w->user_data();
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) pManager->ext_attr();
    if(ext->child_setting_standalone) pManager->hidden(true);

    free_widget(ext->obj_ntp_setting);

    // ขยายเวลาให้ unlock หาก system keylock เป็นโหมด lock
    if( NVS.getInt("KeyLock") ){
      unlock_state = true;
      unlock_timer = millis() + KEYUNLOCK_TIMEOUT;
    }
    MEM_CHECK; 
  });

  ext->lb_ntp_setting_title->text("ตั้งค่า การซิงค์เวลา NTP");
  ext->lb_ntp_setting_title->align(ext->lb_ntp_setting_back, ALIGN_RIGHT);

  ext->page_ntp_seting_body->size(ext->obj_ntp_setting->width(), ext->obj_ntp_setting->height()- ext->lb_ntp_setting_title->bottomY()+5);
  ext->page_ntp_seting_body->position(0, ext->lb_ntp_setting_title->bottomY()-5);
  ext->page_ntp_seting_body->opa(0);
  ext->page_ntp_seting_body->corner_radius(0);
  ext->page_ntp_seting_body->border(0);
  ext->page_ntp_seting_body->font((GScreen.width()==480)? prasanmit_35: prasanmit_25, TFT_WHITE);

  ext->lb_ntp_server_info->text("NTP Server");
//  ext->lb_ntp_server_info->position( 20, lb_ntp_setting_title.bottomY() - 3);

  ext->ta_ntp_server->width(BlynkGO.width()-40);
  ext->ta_ntp_server->placeholder("NTP Server");
  ext->ta_ntp_server->cursor_type(CURSOR_NONE);
//  ext->ta_ntp_server->align(lb_ntp_server_info, ALIGN_BOTTOM_LEFT);

  ext->lb_ntp_timezone_info->text("TimeZone");
//  ext->lb_ntp_timezone_info->align(ta_ntp_server, ALIGN_BOTTOM_LEFT, 0, 3);
  
  ext->ta_ntp_timezone->width(BlynkGO.width()-40);
  ext->ta_ntp_timezone->placeholder("TimeZone");
  ext->ta_ntp_timezone->cursor_type(CURSOR_NONE);
//  ext->ta_ntp_timezone->align(lb_ntp_timezone_info, ALIGN_BOTTOM_LEFT);
  
  ext->lb_ntp_daylight_info->text("Daylight saving time (DST)");
//  ext->lb_ntp_daylight_info->align(ta_ntp_timezone, ALIGN_BOTTOM_LEFT, 0, 3);

  ext->ta_ntp_daylight->width(BlynkGO.width()-40);
  ext->ta_ntp_daylight->placeholder("Daylight saving time (DST)");
  ext->ta_ntp_daylight->cursor_type(CURSOR_NONE);
//  ext->ta_ntp_daylight->align(lb_ntp_daylight_info, ALIGN_BOTTOM_LEFT);

  ext->ta_ntp_server->user_data(ext);
  ext->ta_ntp_server->onValueChanged([](GWidget* w){
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();
    
    String ntp_server = ext->ta_ntp_server->text();

#if BLYNKGO_USE_NTP
    if(ntp_server == "") {
      ntp_server = NTP_HOSTNAME;
      ext->ta_ntp_server->text(ntp_server);
    }
#endif

    Serial.printf("NTP Server : %s\n",ntp_server.c_str());
    NVS.setString("NTP_SERVER", ntp_server);
  });

  ext->ta_ntp_timezone->user_data(ext);
  ext->ta_ntp_timezone->onValueChanged([](GWidget* w){
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();
    
    float timezone = ext->ta_ntp_timezone->text().toFloat();
    
#if BLYNKGO_USE_NTP
    if( ext->ta_ntp_timezone->text() == "") {
      timezone = NTP_TIMEZONE;
      ext->ta_ntp_server->text( String(timezone,1) );
    }
#endif

    Serial.printf("TimeZone : %.2f\n", timezone);
    NVS.setFloat("TIMEZONE",  timezone);
  });
  
  ext->ta_ntp_daylight->user_data(ext);
  ext->ta_ntp_daylight->onValueChanged([](GWidget* w){
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();
    
    int8_t daylight = (int8_t) ext->ta_ntp_daylight->text().toInt();

#if BLYNKGO_USE_NTP
    if( ext->ta_ntp_daylight->text() == "") {
      daylight = NTP_DAYLIGHT_SAVING;
      ext->ta_ntp_daylight->text( String(daylight) );
    }
#endif

    Serial.printf("Daylight Saving Time : %d\n",daylight);
    NVS.setInt("DAYLIGHT", (int8_t)daylight);
  });

  if(NVS.exists("NTP_SERVER"))   {  ext->ta_ntp_server->text(NVS.getString("NTP_SERVER")); } 
  if(NVS.exists("TIMEZONE"))     {  ext->ta_ntp_timezone->text( String((float ) NVS.getFloat("TIMEZONE"),1) );  }
  if(NVS.exists("DAYLIGHT"))     {  ext->ta_ntp_daylight->text( String( (int8_t) NVS.getInt("DAYLIGHT") ) );  }
  
  if(ancestor_ntp_setting_signal_cb == NULL) ancestor_ntp_setting_signal_cb = ext->obj_ntp_setting->signal_cb();
  ext->obj_ntp_setting->signal_cb(ntp_setting_signal_cb);
}



/**********************************************
 * static functions
 **********************************************/
static lv_res_t ntp_setting_signal_cb(lv_obj_t *obj, lv_signal_t sign, void* param)
{
  lv_res_t res;
  res = ancestor_ntp_setting_signal_cb( obj, sign, param);
  if(res != LV_RES_OK ) return res;
  if(sign == LV_SIGNAL_GET_TYPE) 
    return lv_obj_handle_get_type_signal((lv_obj_type_t*)param, "");

  /* หลัง del() หรือ deconstrutor ถูกเรียก และภายในได้ลบ child ไปแล้ว */
  if(sign == LV_SIGNAL_CLEANUP) {
    Serial.println("[NTP Setting] signal cleanup");
    GObject* obj_ntp_setting = (GObject*) lv_obj_get_user_data(obj);
    BlynkGO_Manager* pManager = (BlynkGO_Manager*) obj_ntp_setting->_par;
    blynkgo_manager_ext_t* ext = (blynkgo_manager_ext_t*)  pManager->ext_attr();

    free_widget(ext->lb_ntp_setting_back);
    free_widget(ext->lb_ntp_setting_title);
    free_widget(ext->page_ntp_seting_body);
    free_widget(ext->lb_ntp_server_info);
    free_widget(ext->lb_ntp_timezone_info);
    free_widget(ext->lb_ntp_daylight_info);
    free_widget(ext->ta_ntp_server);
    free_widget(ext->ta_ntp_timezone);
    free_widget(ext->ta_ntp_daylight);


    if(ext->obj_ntp_setting->hasLocalStyle()) {ext->obj_ntp_setting->freeLocalStyle(); } //pIcon->_has_localstyle = false; } // 
    ext->obj_ntp_setting->_created = false;

    MEM_CHECK;
  }
  return LV_RES_OK;
}
