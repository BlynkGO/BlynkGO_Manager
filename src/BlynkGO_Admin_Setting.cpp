#include "BlynkGO_Manager.h"

/**********************************************
 * static variables
 **********************************************/

static lv_signal_cb_t ancestor_admin_setting_signal_cb;


/**********************************************
 * static functions
 **********************************************/
static lv_res_t admin_setting_signal_cb(lv_obj_t *obj, lv_signal_t sign, void* param);
static void lb_admin_setting_back_onClicked(GWidget* w);
static void sw_admin_lock_mode_onValueChanged(GWidget*w);

/**********************************************
 * class member functions
 **********************************************/

void BlynkGO_Manager::admin_setting(bool standalone){
  this->hidden(false);
  blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) this->ext_attr();
  ext->child_setting_standalone = standalone;
  ext->obj_admin_setting          = new_GObject(this);
    ext->lb_admin_setting_back    = new_GLabel(ext->obj_admin_setting);
    ext->lb_admin_setting_title   = new_GLabel(ext->obj_admin_setting);
    ext->list_admin_setting_body  = new_GList(ext->obj_admin_setting);
      ext->cont_admin_lock_mode   = new_GContainer(ext->list_admin_setting_body);
        ext->lb_admin_lock_mode   = new_GLabel(ext->cont_admin_lock_mode);
        ext->sw_admin_lock_mode   = new_GSwitch(ext->cont_admin_lock_mode);


  ext->lb_admin_setting_back  ->user_data(ext);
  ext->sw_admin_lock_mode     ->user_data(ext);
  ext->list_admin_setting_body->user_data(ext);


  ext->obj_admin_setting->size(GScreen);
  ext->obj_admin_setting->color(TFT_BLACK);

  ext->lb_admin_setting_back->text("  " SYMBOL_ARROW_LEFT "  ");
  ext->lb_admin_setting_back->position(3,0);
  ext->lb_admin_setting_back->clickable(true);
  ext->lb_admin_setting_back->user_data(this);
  ext->lb_admin_setting_back->onClicked(lb_admin_setting_back_onClicked);

  ext->lb_admin_setting_title->text("ตั้งค่า Admin");
  ext->lb_admin_setting_title->align(ext->lb_admin_setting_back, ALIGN_RIGHT);
  
  ext->list_admin_setting_body->size(ext->obj_admin_setting->width(),ext->obj_admin_setting->height()-40);
  ext->list_admin_setting_body->align(ALIGN_BOTTOM);
  
  ext->list_admin_setting_body->padding_top(7);                   // ระยะช่องไฟด้านบนของ item
  ext->list_admin_setting_body->padding_bottom(7);                // ระยะช่องไฟด้านล่างของ item
  ext->list_admin_setting_body->padding_left(20);                 // ระยะช่องไฟด้านซ้ายของ item

  ext->list_admin_setting_body->font((BlynkGO.width()==480)? prasanmit_35 : prasanmit_25, TFT_WHITE);

  ext->list_admin_setting_body->style[RELEASED].color(TFT_BLACK);
  ext->list_admin_setting_body->style[PRESSED].color(TFT_DARKGRAY);
  ext->list_admin_setting_body->padding_left(30);

  ext->cont_admin_lock_mode->fit2(FIT_FLOOD, FIT_TIGHT);
  ext->cont_admin_lock_mode->layout(LAYOUT_OFF);
  ext->cont_admin_lock_mode->page_glue(true);
  ext->cont_admin_lock_mode->color(TFT_BLACK);
  ext->cont_admin_lock_mode->font((BlynkGO.width()==480)? prasanmit_35 : prasanmit_25, TFT_WHITE);
  
  ext->lb_admin_lock_mode->text("เปิดใช้งาน Admin-Lock");
  ext->lb_admin_lock_mode->align(ALIGN_LEFT,30);
  
  ext->sw_admin_lock_mode->align(ALIGN_RIGHT,-10);
  ext->sw_admin_lock_mode->state(NVS.getInt("KeyLock"));

  ext->sw_admin_lock_mode->onValueChanged(sw_admin_lock_mode_onValueChanged);

  ext->list_admin_setting_body->addItem("เปลี่ยนรหัส Admin-Lock ใหม่");
  ext->list_admin_setting_body->onClicked([](GWidget*w){
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();
    
    int16_t item_selected_id = ext->list_admin_setting_body->item_selected_id();
    Serial.println( item_selected_id);
    if( item_selected_id==0 ) {
      ext->keyunlocker = new_GKeyUnlocker();
      ext->keyunlocker->size(GScreen);
      ext->keyunlocker->user_data(ext);
      ext->keyunlocker->mode( GKEYUNLOCKER_MODE_SETTING);
      ext->keyunlocker->onApply([](GWidget* w){
        Serial.println("GKeyUnlocker Apply");
        blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();

        bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 1 ) ;
      });
      ext->keyunlocker->onCancel([](GWidget* w){
        Serial.println("GKeyUnlocker Cancel");
        blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();

        bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 1 ) ;
      });
    }
  });
  
  if(ancestor_admin_setting_signal_cb == NULL) ancestor_admin_setting_signal_cb = ext->obj_admin_setting->signal_cb();
  ext->obj_admin_setting->signal_cb(admin_setting_signal_cb);
  
}

/**********************************************
 * static functions
 **********************************************/
 
static lv_res_t admin_setting_signal_cb(lv_obj_t *obj, lv_signal_t sign, void* param)
{
  lv_res_t res;
  res = ancestor_admin_setting_signal_cb( obj, sign, param);
  if(res != LV_RES_OK ) return res;
  if(sign == LV_SIGNAL_GET_TYPE) 
    return lv_obj_handle_get_type_signal((lv_obj_type_t*)param, "");

  /* หลัง del() หรือ deconstrutor ถูกเรียก และภายในได้ลบ child ไปแล้ว */
  if(sign == LV_SIGNAL_CLEANUP) {
    Serial.println("[Admin Setting] signal cleanup");
    GObject* obj_admin_setting = (GObject*) lv_obj_get_user_data(obj);
    BlynkGO_Manager* pManager = (BlynkGO_Manager*) obj_admin_setting->_par;
    blynkgo_manager_ext_t* ext = (blynkgo_manager_ext_t*)  pManager->ext_attr();

    ext->sw_admin_lock_mode->del();       free_widget(ext->sw_admin_lock_mode);
    ext->lb_admin_lock_mode->del();       free_widget(ext->lb_admin_lock_mode);
    ext->cont_admin_lock_mode->del();     free_widget(ext->cont_admin_lock_mode);
    ext->list_admin_setting_body->del();  free_widget(ext->list_admin_setting_body);
    ext->lb_admin_setting_title->del();   free_widget(ext->lb_admin_setting_title);
    ext->lb_admin_setting_back->del();    free_widget(ext->lb_admin_setting_back);

    if(ext->obj_admin_setting->hasLocalStyle()) {ext->obj_admin_setting->freeLocalStyle(); } //pIcon->_has_localstyle = false; } // 
    ext->obj_admin_setting->_created = false;

    MEM_CHECK;
  }
  return LV_RES_OK;
}


/* Admin Setting */
static void lb_admin_setting_back_onClicked(GWidget* w){
  BlynkGO_Manager* pManager = (BlynkGO_Manager*) w->user_data();
  blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) pManager->ext_attr();
  if(ext->child_setting_standalone) pManager->hidden(true);

  ext->obj_admin_setting->del();    free_widget(ext->obj_admin_setting);

  MEM_CHECK;
}

static void sw_admin_lock_mode_onValueChanged(GWidget*w){
  Serial.println("SW Value Changed");
  blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();
  
  Serial.printf("Switch : %d\n", ext->sw_admin_lock_mode->state());

  if(ext->sw_admin_lock_mode->state()){
    if(NVS.getInt("SecretKey") < 1000){ 
      ext->keyunlocker = new_GKeyUnlocker();
      ext->keyunlocker->size(GScreen);
      ext->keyunlocker->user_data(ext);
      ext->keyunlocker->mode( GKEYUNLOCKER_MODE_SETTING);
      ext->keyunlocker->onApply([](GWidget* w){
        Serial.println("GKeyUnlocker Apply 11");
        blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();

        bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 1 ) ;
      });
      ext->keyunlocker->onCancel([](GWidget* w){
        Serial.println("GKeyUnlocker Cancel 1111");
        blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();

        bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 1 ) ;
      });
    }else{
      NVS.setInt("KeyLock", (uint8_t) true );
      unlock_state = false;
    }
  }else{
    ext->keyunlocker = new_GKeyUnlocker();
    ext->keyunlocker->size(GScreen);
    ext->keyunlocker->user_data(ext);
    ext->keyunlocker->mode( GKEYUNLOCKER_MODE_LOCK);
    
    ext->keyunlocker->onApply([](GWidget* w){
      blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();
      Serial.println("GKeyUnlocker Apply 222");
      NVS.setInt("KeyLock", (uint8_t) false );

      bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 1 ) ;
    });
    ext->keyunlocker->onCancel([](GWidget* w){
      blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) w->user_data();
      
      Serial.println("GKeyUnlocker Cancel 2222");
      NVS.setInt("KeyLock", (uint8_t) true );
      ext->sw_admin_lock_mode->ON();

      bitWrite(SETTING_BIT_GROUP, KEYUNLOCK_BIT, 1 ) ;
    });
  }
}
