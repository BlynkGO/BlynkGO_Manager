#include "BlynkGO_Manager.h"

/**********************************************
 * static variables
 **********************************************/

static lv_signal_cb_t ancestor_gps_setting_signal_cb;


/**********************************************
 * static functions
 **********************************************/
static lv_res_t gps_setting_signal_cb(lv_obj_t *obj, lv_signal_t sign, void* param);

/**********************************************
 * class member functions
 **********************************************/

//  GObject * obj_gps_setting;
//  GLabel  * lb_gps_setting_back;//("  " SYMBOL_ARROW_LEFT "  ", obj_gps_setting );
//  GLabel  * lb_gps_setting_title;//("ตั้งค่า พิกัด GPS", obj_gps_setting);
//  GLabel  * lb_gps_lat_info;//("ละติจูด", obj_gps_setting);
//  GLabel  * lb_gps_long_info;//("ลองจิจูด", obj_gps_setting);
//  GTextArea*ta_gps_lat;//(obj_gps_setting);
//  GTextArea*ta_gps_long;//(obj_gps_setting);
  
void BlynkGO_Manager::gps_setting(bool standalone){
  this->hidden(false);
  blynkgo_manager_ext_t* ext  = (blynkgo_manager_ext_t*) this->ext_attr();
  ext->child_setting_standalone = standalone;
  ext->obj_gps_setting          = new_GObject(this);
    ext->lb_gps_setting_back    = new_GLabel(ext->obj_gps_setting);
    ext->lb_gps_setting_title   = new_GLabel(ext->obj_gps_setting);
    ext->page_gps_setting_body  = new_GPage(ext->obj_gps_setting);
      ext->lb_gps_lat_info        = new_GLabel(ext->page_gps_setting_body);
      ext->lb_gps_long_info       = new_GLabel(ext->page_gps_setting_body);
      ext->ta_gps_lat             = new_GTextArea(ext->page_gps_setting_body);
      ext->ta_gps_long            = new_GTextArea(ext->page_gps_setting_body);

  ext->obj_gps_setting->size(GScreen);
  ext->obj_gps_setting->color(TFT_BLACK);

  ext->lb_gps_setting_back->text("  " SYMBOL_ARROW_LEFT "  ");
  ext->lb_gps_setting_back->position(3,0);
  ext->lb_gps_setting_back->clickable(true);
  ext->lb_gps_setting_back->user_data(this);
  ext->lb_gps_setting_back->onClicked([](GWidget* w){
    BlynkGO_Manager* pManager = (BlynkGO_Manager*) w->user_data();
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) pManager->ext_attr();
    if(ext->child_setting_standalone) pManager->hidden(true);

    free_widget(ext->obj_gps_setting);

    // ขยายเวลาให้ unlock หาก system keylock เป็นโหมด lock
    if( NVS.getInt("KeyLock") ){
      unlock_state = true;
      unlock_timer = millis() + KEYUNLOCK_TIMEOUT;
    }
    MEM_CHECK; 
  });

  ext->lb_gps_setting_title->text("ตั้งค่า พิกัด GPS");
  ext->lb_gps_setting_title->align(ext->lb_gps_setting_back, ALIGN_RIGHT);


  ext->page_gps_setting_body->size(ext->obj_gps_setting->width(), ext->obj_gps_setting->height()- ext->lb_gps_setting_title->bottomY()+5);
  ext->page_gps_setting_body->position(0, ext->lb_gps_setting_title->bottomY()-5);
  ext->page_gps_setting_body->opa(0);
  ext->page_gps_setting_body->corner_radius(0);
  ext->page_gps_setting_body->border(0);
  ext->page_gps_setting_body->font((GScreen.width()==480)? prasanmit_35: prasanmit_25, TFT_WHITE);

  ext->lb_gps_lat_info->text("ละติจูด");
//  ext->lb_gps_lat_info->position( 20, ext->lb_gps_setting_title->bottomY() -5);

  ext->ta_gps_lat->width(BlynkGO.width()-40);
  ext->ta_gps_lat->placeholder("ละติจูด");
  ext->ta_gps_lat->cursor_type(CURSOR_NONE);
//  ext->ta_gps_lat->align(ext->lb_gps_lat_info, ALIGN_BOTTOM_LEFT);


  ext->lb_gps_long_info->text("ลองจิจูด");
//  ext->lb_gps_long_info->align(ext->ta_gps_lat, ALIGN_BOTTOM_LEFT, 0, 5);

  ext->ta_gps_long->width(BlynkGO.width()-40);
  ext->ta_gps_long->placeholder("ลองจิจูด");
  ext->ta_gps_long->cursor_type(CURSOR_NONE);
//  ext->ta_gps_long->align(ext->lb_gps_long_info, ALIGN_BOTTOM_LEFT);

  ext->ta_gps_lat->user_data(ext);
  ext->ta_gps_lat->onValueChanged([](GWidget* w){
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();
    
    String latitude = ext->ta_gps_lat->text();
    Serial.printf("GPS Latitude : %s\n",latitude.c_str());
    NVS.setString("GPS_LAT", latitude);
  });
  
  ext->ta_gps_long->user_data(ext);
  ext->ta_gps_long->onValueChanged([](GWidget* w){
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();
    
    String longitude = ext->ta_gps_long->text();
    Serial.printf("GPS Longitude : %s\n",longitude.c_str());
    NVS.setString("GPS_LONG", longitude);
  });

  if(NVS.exists("GPS_LAT"))   {  ext->ta_gps_lat->text(NVS.getString("GPS_LAT")); } 
  if(NVS.exists("GPS_LONG"))  {  ext->ta_gps_long->text(NVS.getString("GPS_LONG")); }
  
  if(ancestor_gps_setting_signal_cb == NULL) ancestor_gps_setting_signal_cb = ext->obj_gps_setting->signal_cb();
  ext->obj_gps_setting->signal_cb(gps_setting_signal_cb);
}


/**********************************************
 * static functions
 **********************************************/

//  GObject * obj_gps_setting;
//  GLabel  * lb_gps_setting_back;//("  " SYMBOL_ARROW_LEFT "  ", obj_gps_setting );
//  GLabel  * lb_gps_setting_title;//("ตั้งค่า พิกัด GPS", obj_gps_setting);
//  GLabel  * lb_gps_lat_info;//("ละติจูด", obj_gps_setting);
//  GLabel  * lb_gps_long_info;//("ลองจิจูด", obj_gps_setting);
//  GTextArea*ta_gps_lat;//(obj_gps_setting);
//  GTextArea*ta_gps_long;//(obj_gps_setting);
// 
static lv_res_t gps_setting_signal_cb(lv_obj_t *obj, lv_signal_t sign, void* param)
{
  lv_res_t res;
  res = ancestor_gps_setting_signal_cb( obj, sign, param);
  if(res != LV_RES_OK ) return res;
  if(sign == LV_SIGNAL_GET_TYPE) 
    return lv_obj_handle_get_type_signal((lv_obj_type_t*)param, "");

  /* หลัง del() หรือ deconstrutor ถูกเรียก และภายในได้ลบ child ไปแล้ว */
  if(sign == LV_SIGNAL_CLEANUP) {
    Serial.println("[GPS Setting] signal cleanup");
    GObject* obj_gps_setting = (GObject*) lv_obj_get_user_data(obj);
    BlynkGO_Manager* pManager = (BlynkGO_Manager*) obj_gps_setting->_par;
    blynkgo_manager_ext_t* ext = (blynkgo_manager_ext_t*)  pManager->ext_attr();

    free_widget(ext->lb_gps_setting_back);
    free_widget(ext->lb_gps_setting_title);

    free_widget(ext->lb_gps_lat_info);
    free_widget(ext->lb_gps_long_info);
    free_widget(ext->ta_gps_lat);
    free_widget(ext->ta_gps_long);
    free_widget(ext->page_gps_setting_body);

    if(ext->obj_gps_setting->hasLocalStyle()) {ext->obj_gps_setting->freeLocalStyle(); } //pIcon->_has_localstyle = false; } // 
    ext->obj_gps_setting->_created = false;

    MEM_CHECK;
  }
  return LV_RES_OK;
}
