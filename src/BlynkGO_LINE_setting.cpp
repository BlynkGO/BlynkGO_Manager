#include "BlynkGO_Manager.h"

/**********************************************
 * static variables
 **********************************************/

static lv_signal_cb_t ancestor_linenotify_setting_signal_cb;


/**********************************************
 * static functions
 **********************************************/
static lv_res_t linenotify_setting_signal_cb(lv_obj_t *obj, lv_signal_t sign, void* param);

/**********************************************
 * class member functions
 **********************************************/

void BlynkGO_Manager::line_setting(bool standalone){
  this->hidden(false);
  blynkgo_manager_ext_t* ext  = (blynkgo_manager_ext_t*) this->ext_attr();

  ext->child_setting_standalone = standalone;
  ext->obj_line_setting       = new_GObject(this);
    ext->lb_line_setting_back   = new_GLabel(ext->obj_line_setting);//("  " SYMBOL_ARROW_LEFT "  ", obj_line_setting );
    ext->lb_line_setting_title  = new_GLabel(ext->obj_line_setting);//("ตั้งค่า Line Notify", obj_line_setting);
    ext->lb_line_token_info     = new_GLabel(ext->obj_line_setting);//("Line Token", obj_line_setting);
    ext->ta_line_token          = new_GTextArea(ext->obj_line_setting);//(obj_line_setting);

  ext->obj_line_setting->size(GScreen);
  ext->obj_line_setting->color(TFT_BLACK);

  ext->lb_line_setting_back->text("  " SYMBOL_ARROW_LEFT "  ");
  ext->lb_line_setting_back->position(3,0);
  ext->lb_line_setting_back->clickable(true);
  ext->lb_line_setting_back->user_data(this);
  ext->lb_line_setting_back->onClicked([](GWidget* w){  
    BlynkGO_Manager* pManager = (BlynkGO_Manager*) w->user_data();
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) pManager->ext_attr();
    if(ext->child_setting_standalone) pManager->hidden(true);

    free_widget(ext->obj_line_setting);

    // ขยายเวลาให้ unlock หาก system keylock เป็นโหมด lock
    if( NVS.getInt("KeyLock") ){
      unlock_state = true;
      unlock_timer = millis() + KEYUNLOCK_TIMEOUT;
    }
    MEM_CHECK; 
  });

  ext->lb_line_setting_title->text("ตั้งค่า Line Notify");
  ext->lb_line_setting_title->align(ext->lb_line_setting_back, ALIGN_RIGHT);

  ext->lb_line_token_info->text("Line Token");
  ext->lb_line_token_info->position( 20, ext->lb_line_setting_title->bottomY() + 10);

  ext->ta_line_token->width(BlynkGO.width()-40);
  ext->ta_line_token->placeholder("LINE Token");
  ext->ta_line_token->cursor_type(CURSOR_NONE);
  ext->ta_line_token->align(ext->lb_line_token_info, ALIGN_BOTTOM_LEFT);

  ext->ta_line_token->user_data(ext);
  ext->ta_line_token->onValueChanged([](GWidget* w){
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();
    String token = ext->ta_line_token->text();
    Serial.printf("LINE Token : %s\n",token.c_str());
    NVS.setString(KEY_LINE_TOKEN, token);
  });

  
  if(NVS.exists(KEY_LINE_TOKEN)) {  ext->ta_line_token->text(NVS.getString(KEY_LINE_TOKEN)); }
  
  if(ancestor_linenotify_setting_signal_cb == NULL) ancestor_linenotify_setting_signal_cb = ext->obj_line_setting->signal_cb();
  ext->obj_line_setting->signal_cb(linenotify_setting_signal_cb);
}


/**********************************************
 * static functions
 **********************************************/
static lv_res_t linenotify_setting_signal_cb(lv_obj_t *obj, lv_signal_t sign, void* param)
{
  lv_res_t res;
  res = ancestor_linenotify_setting_signal_cb( obj, sign, param);
  if(res != LV_RES_OK ) return res;
  if(sign == LV_SIGNAL_GET_TYPE) 
    return lv_obj_handle_get_type_signal((lv_obj_type_t*)param, "");

  /* หลัง del() หรือ deconstrutor ถูกเรียก และภายในได้ลบ child ไปแล้ว */
  if(sign == LV_SIGNAL_CLEANUP) {
    Serial.println("[Line Setting] signal cleanup");
    GObject* obj_line_setting = (GObject*) lv_obj_get_user_data(obj);
    BlynkGO_Manager* pManager = (BlynkGO_Manager*) obj_line_setting->_par;
    blynkgo_manager_ext_t* ext = (blynkgo_manager_ext_t*)  pManager->ext_attr();

    free_widget(ext->ta_line_token);
    free_widget(ext->lb_line_token_info);
    free_widget(ext->lb_line_setting_title);
    free_widget(ext->lb_line_setting_back);

    if(ext->obj_line_setting->hasLocalStyle()) {ext->obj_line_setting->freeLocalStyle(); } //pIcon->_has_localstyle = false; } // 
    ext->obj_line_setting->_created = false;

    MEM_CHECK;
  }
  return LV_RES_OK;
}
