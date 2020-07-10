#include "BlynkGO_Manager.h"
#if BLYNKGO_USE_BLYNK

/**********************************************
 * static variables
 **********************************************/

static lv_signal_cb_t ancestor_blynk_setting_signal_cb;


/**********************************************
 * static functions
 **********************************************/
static lv_res_t blynk_setting_signal_cb(lv_obj_t *obj, lv_signal_t sign, void* param);

/**********************************************
 * class member functions
 **********************************************/

void BlynkGO_Manager::blynk_setting(bool standalone){
  this->hidden(false);
#if BLYNKGO_USE_BLYNK
  if( !NVS.exists("BLYNK_HOST")) { NVS.setString("BLYNK_HOST" , BLYNK_DEFAULT_DOMAIN ); }
  if( !NVS.exists("BLYNK_PORT")) { NVS.setInt("BLYNK_PORT"    , (uint16_t) BLYNK_DEFAULT_PORT ); }
#endif

  blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) this->ext_attr();

  ext->child_setting_standalone = standalone;

  ext->obj_blynk_setting          = new_GObject(this);
    ext->lb_blynk_setting_back    = new_GLabel(ext->obj_blynk_setting);//("  " SYMBOL_ARROW_LEFT "  ", obj_blynk_setting );
    ext->lb_blynk_setting_title   = new_GLabel(ext->obj_blynk_setting);//("ตั้งค่า Blynk", obj_blynk_setting);
    ext->page_blynk_setting_body  = new_GPage(ext->obj_blynk_setting);
      ext->lb_blynk_auth_info     = new_GLabel(ext->page_blynk_setting_body);//("Blynk Auth", obj_blynk_setting);
      ext->lb_blynk_host_info     = new_GLabel(ext->page_blynk_setting_body);//("Blynk Host", obj_blynk_setting);
      ext->lb_blynk_port_info     = new_GLabel(ext->page_blynk_setting_body);//("Blynk Port", obj_blynk_setting);
      ext->ta_blynk_auth          = new_GTextArea(ext->page_blynk_setting_body);//(obj_blynk_setting);
      ext->ta_blynk_host          = new_GTextArea(ext->page_blynk_setting_body);//(obj_blynk_setting);
      ext->ta_blynk_port          = new_GTextArea(ext->page_blynk_setting_body);//(obj_blynk_setting);
  
  ext->obj_blynk_setting->size(GScreen);
  ext->obj_blynk_setting->color(TFT_BLACK);

  ext->lb_blynk_setting_back->text("  " SYMBOL_ARROW_LEFT "  ");
  ext->lb_blynk_setting_back->position(3,0);
  ext->lb_blynk_setting_back->clickable(true);
  ext->lb_blynk_setting_back->user_data(this);
  ext->lb_blynk_setting_back->onClicked([](GWidget* w){ 
    BlynkGO_Manager* pManager = (BlynkGO_Manager*) w->user_data();
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) pManager->ext_attr();
    if(ext->child_setting_standalone) pManager->hidden(true);

    ext->obj_blynk_setting->del();       free_widget(ext->obj_blynk_setting);

    // ขยายเวลาให้ unlock หาก system keylock เป็นโหมด lock
    if( NVS.getInt("KeyLock") ){
      unlock_state = true;
      unlock_timer = millis() + KEYUNLOCK_TIMEOUT;
    }
    MEM_CHECK; 

  });

  ext->lb_blynk_setting_title->text("ตั้งค่า Blynk");
  ext->lb_blynk_setting_title->align(ext->lb_blynk_setting_back, ALIGN_RIGHT);

  ext->page_blynk_setting_body->size(ext->obj_blynk_setting->width(), ext->obj_blynk_setting->height()- ext->lb_blynk_setting_title->bottomY()+5);
  ext->page_blynk_setting_body->position(0, ext->lb_blynk_setting_title->bottomY()-5);
  ext->page_blynk_setting_body->opa(0);
  ext->page_blynk_setting_body->corner_radius(0);
  ext->page_blynk_setting_body->border(0);
//  ext->page_blynk_setting_body->padding_top(-10);
//  ext->page_blynk_setting_body->padding_left(12);
//  ext->page_blynk_setting_body->drag_direction(DRAG_DIR_VER);
  ext->page_blynk_setting_body->font((GScreen.width()==480)? prasanmit_35: prasanmit_25, TFT_WHITE);

  ext->lb_blynk_auth_info->text("Blynk Auth");
//  ext->lb_blynk_auth_info->position( 20, ext->lb_blynk_setting_title->bottomY() - 3);

  ext->ta_blynk_auth->width(BlynkGO.width()-40);
  ext->ta_blynk_auth->placeholder("Blynk Auth");
  ext->ta_blynk_auth->cursor_type(CURSOR_NONE);
  ext->ta_blynk_auth->max_length(32);
//  ext->ta_blynk_auth->align(ext->lb_blynk_auth_info, ALIGN_BOTTOM_LEFT);

  ext->lb_blynk_host_info->text("Blynk Host");
//  ext->lb_blynk_host_info->align(ext->ta_blynk_auth, ALIGN_BOTTOM_LEFT, 0, 3);
    
  ext->ta_blynk_host->width(BlynkGO.width()-40);
  ext->ta_blynk_host->placeholder("Blynk Host");
  ext->ta_blynk_host->cursor_type(CURSOR_NONE);
//  ext->ta_blynk_host->align(ext->lb_blynk_host_info, ALIGN_BOTTOM_LEFT);
  
  ext->lb_blynk_port_info->text("Blynk Port");
//  ext->lb_blynk_port_info->align(ext->ta_blynk_host, ALIGN_BOTTOM_LEFT);
  
  ext->ta_blynk_port->width(BlynkGO.width()-40);
  ext->ta_blynk_port->placeholder("Blynk Port");
  ext->ta_blynk_port->max_length(6);
  ext->ta_blynk_port->cursor_type(CURSOR_NONE);
//  ext->ta_blynk_port->align(ext->lb_blynk_port_info, ALIGN_BOTTOM_LEFT);

  ext->ta_blynk_auth->user_data(ext);
  ext->ta_blynk_auth->onValueChanged([](GWidget* w){
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();
    Serial.printf("Blynk Auth : %s\n", ext->ta_blynk_auth->text().c_str());
    NVS.setString(KEY_BLYNK_AUTH, ext->ta_blynk_auth->text());
  });

  ext->ta_blynk_host->user_data(ext);
  ext->ta_blynk_host->onValueChanged([](GWidget* w){
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();

    String new_host = ext->ta_blynk_host->text();
    if(new_host == ""){
      new_host = BLYNK_DEFAULT_DOMAIN;
      ext->ta_blynk_host->text(new_host);
    }
        
    Serial.printf("Blynk Host : %s\n", new_host.c_str());
    NVS.setString(KEY_BLYNK_HOST, new_host);
  });

  ext->ta_blynk_port->user_data(ext);
  ext->ta_blynk_port->onValueChanged([](GWidget* w){
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();

    uint16_t new_port = ext->ta_blynk_port->text().toInt();
    if(new_port == 0){
      new_port = BLYNK_DEFAULT_PORT;
      ext->ta_blynk_port->text(String(new_port));
    }
    Serial.printf("Blynk Port : %d\n", new_port);
    NVS.setInt(KEY_BLYNK_PORT, (uint16_t) new_port);
  });
    
  if(NVS.exists(KEY_BLYNK_HOST)) {  ext->ta_blynk_host->text(NVS.getString(KEY_BLYNK_HOST)); }
  if(NVS.exists(KEY_BLYNK_PORT)) {  ext->ta_blynk_port->text( String((uint16_t)NVS.getInt(KEY_BLYNK_PORT)) ); }
  if(NVS.exists(KEY_BLYNK_AUTH)) {  ext->ta_blynk_auth->text(NVS.getString(KEY_BLYNK_AUTH)); }

  if(ancestor_blynk_setting_signal_cb == NULL) ancestor_blynk_setting_signal_cb = ext->obj_blynk_setting->signal_cb();
  ext->obj_blynk_setting->signal_cb(blynk_setting_signal_cb);
}


/**********************************************
 * static functions
 **********************************************/
 static lv_res_t blynk_setting_signal_cb(lv_obj_t *obj, lv_signal_t sign, void* param)
{
  lv_res_t res;
  res = ancestor_blynk_setting_signal_cb( obj, sign, param);
  if(res != LV_RES_OK ) return res;
  if(sign == LV_SIGNAL_GET_TYPE) 
    return lv_obj_handle_get_type_signal((lv_obj_type_t*)param, "");

  /* หลัง del() หรือ deconstrutor ถูกเรียก และภายในได้ลบ child ไปแล้ว */
  if(sign == LV_SIGNAL_CLEANUP) {
    Serial.println("[Blynk Setting] signal cleanup");
    GObject* obj_blynk_setting = (GObject*) lv_obj_get_user_data(obj);
    BlynkGO_Manager* pManager = (BlynkGO_Manager*) obj_blynk_setting->_par;
    blynkgo_manager_ext_t* ext = (blynkgo_manager_ext_t*)  pManager->ext_attr();

  
    ext->ta_blynk_auth->del();          free_widget(ext->ta_blynk_auth);
    ext->ta_blynk_port->del();          free_widget(ext->ta_blynk_port);//(obj_blynk_setting);
    ext->ta_blynk_host->del();          free_widget(ext->ta_blynk_host);//(obj_blynk_setting);
    ext->lb_blynk_auth_info->del();     free_widget(ext->lb_blynk_auth_info);//("Blynk Auth", obj_blynk_setting);
    ext->lb_blynk_port_info->del();     free_widget(ext->lb_blynk_port_info);//("Blynk Port", obj_blynk_setting);
    ext->lb_blynk_host_info->del();     free_widget(ext->lb_blynk_host_info);//("Blynk Host", obj_blynk_setting);
    ext->page_blynk_setting_body->del();free_widget(ext->page_blynk_setting_body);
    ext->lb_blynk_setting_title->del(); free_widget(ext->lb_blynk_setting_title);//("ตั้งค่า Blynk", obj_blynk_setting);
    ext->lb_blynk_setting_back->del();  free_widget(ext->lb_blynk_setting_back);//("  " SYMBOL_ARROW_LEFT "  ", obj_blynk_setting );


    if(ext->obj_blynk_setting->hasLocalStyle()) {ext->obj_blynk_setting->freeLocalStyle(); } //pIcon->_has_localstyle = false; } // 
    ext->obj_blynk_setting->_created = false;

    MEM_CHECK;
  }
  return LV_RES_OK;
}


#endif //BLYNKGO_USE_BLYNK