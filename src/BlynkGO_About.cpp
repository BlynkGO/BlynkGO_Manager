#include "BlynkGO_Manager.h"
#include <Arduino.h>

IMG_DECLARE(blynkgo_big_logo);
FONT_DECLARE(prasanmit_60);

/**********************************************
 * static variables
 **********************************************/

static lv_signal_cb_t ancestor_blynkgo_about_signal_cb;
static GAnimation anim;

/**********************************************
 * static functions
 **********************************************/
static lv_res_t blynkgo_about_signal_cb(lv_obj_t *obj, lv_signal_t sign, void* param);

/**********************************************
 * class member functions
 **********************************************/
void BlynkGO_Manager::blynkgo_about(bool standalone){
  this->hidden(false);
  blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) this->ext_attr();
  ext->child_setting_standalone = standalone;
  ext->obj_blynkgo_about = new_GObject(this);
  ext->img_blynkgo_logo  = new_GImage(ext->obj_blynkgo_about);
  ext->lb_blynkgo_text   = new_GLabel(ext->obj_blynkgo_about);
  ext->anim							 = new_GAnimation();

  ext->obj_blynkgo_about->size(GScreen);
  ext->obj_blynkgo_about->color(TFT_WHITE);
  ext->obj_blynkgo_about->user_data(this);
  ext->obj_blynkgo_about->onClicked([](GWidget*w ){
    BlynkGO_Manager* pManager = (BlynkGO_Manager*) w->user_data();
    blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) pManager->ext_attr();
    if(ext->child_setting_standalone) pManager->hidden(true);

    free_widget(ext->obj_blynkgo_about);
    MEM_CHECK;
  });

  ext->lb_blynkgo_text->text("BlynkGO");
  // ext->lb_blynkgo_text->underline(true);
  ext->lb_blynkgo_text->font(prasanmit_60, TFT_BLACK);
  ext->lb_blynkgo_text->hidden(true);

  ext->img_blynkgo_logo->setImage(blynkgo_big_logo);
  ext->img_blynkgo_logo->parent_event(true);

  ext->anim->user_data(ext);
  ext->anim->init(ext->img_blynkgo_logo, [](GWidget* w, int16_t v){
    GImage* pImg = (GImage*) w;
    pImg->opa(v);
  }, 0, 255, 4000, false );
  ext->anim->ready_cb([](GAnimation* anim) {
    anim->init(anim->widget(), [](GWidget* w, int16_t v){
      GImage* pImg = (GImage*) w;
      pImg->zoom(map_f(v,0,1000,100,60));
      // pImg->center( map(v,0,1000, GScreen.width()/2, 50), 
      //               map(v,0,1000, GScreen.height()/2,50));
    }, 0, 1000, 3000, false );

    anim->ready_cb([](GAnimation* anim) {
      blynkgo_manager_ext_t* ext= (blynkgo_manager_ext_t*) anim->user_data();
      ext->lb_blynkgo_text->hidden(false);

      anim->init(anim->widget(), [](GWidget* w, int16_t v){
        GImage* pImg = (GImage*) w;

        pImg->align_center( map_f(v,0,1000,0,-100), 0);

      }, 0, 1000, 2000, false );
      anim->create();
    });
    anim->create();
  });
  ext->anim->create();

  if(ancestor_blynkgo_about_signal_cb == NULL) ancestor_blynkgo_about_signal_cb = ext->obj_blynkgo_about->signal_cb();
  ext->obj_blynkgo_about->signal_cb(blynkgo_about_signal_cb);
}

/**********************************************
 * static functions
 **********************************************/
static lv_res_t blynkgo_about_signal_cb(lv_obj_t *obj, lv_signal_t sign, void* param)
{
  lv_res_t res;
  res = ancestor_blynkgo_about_signal_cb( obj, sign, param);
  if(res != LV_RES_OK ) return res;
  if(sign == LV_SIGNAL_GET_TYPE) 
    return lv_obj_handle_get_type_signal((lv_obj_type_t*)param, "");

  /* หลัง del() หรือ deconstrutor ถูกเรียก และภายในได้ลบ child ไปแล้ว */
  if(sign == LV_SIGNAL_CLEANUP) {
    Serial.println("[BlynkGO About] signal cleanup");
    GObject* obj_blynkgo_about = (GObject*) lv_obj_get_user_data(obj);
    BlynkGO_Manager* pManager = (BlynkGO_Manager*) obj_blynkgo_about->_par;
    blynkgo_manager_ext_t* ext = (blynkgo_manager_ext_t*)  pManager->ext_attr();
    
    ext->anim->del(); free(ext->anim); ext->anim = NULL;
    free_widget(ext->img_blynkgo_logo);
    free_widget(ext->lb_blynkgo_text);

    if(ext->obj_blynkgo_about->hasLocalStyle()) {ext->obj_blynkgo_about->freeLocalStyle(); } //pIcon->_has_localstyle = false; } // 
    ext->obj_blynkgo_about->_created = false;

    MEM_CHECK;
  }
  return LV_RES_OK;
}
