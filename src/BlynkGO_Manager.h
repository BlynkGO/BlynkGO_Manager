#ifndef __BLYNKGO_MANAGER_H__
#define __BLYNKGO_MANAGER_H__

#include <BlynkGO.h>
#include <WiFi.h>

#define KEYUNLOCK_TIMEOUT     30000

extern uint8_t SETTING_BIT_GROUP;
#define WIFI_SCAN_BIT         1   // BIT 1 ของ SETTING_BIT_GROUP
#define WIFI_INFO_BIT         2   // BIT 2 ของ SETTING_BIT_GROUP
#define KEYUNLOCK_BIT         4   // BIT 3 ของ SETTING_BIT_GROUP
#define TOUCHCALIBRATE_BIT    5   // BIT 4 ของ SETTING_BIT_GROUP

extern bool     unlock_state;
extern uint32_t unlock_timer;

typedef struct {
  /* Main Setting */
  GLabel  * lb_setting_back;  // "  " SYMBOL_ARROW_LEFT "  "
  GLabel  * lb_setting_title; // "ตั้งค่า BlynkGO"
  GList   * list_setting;

  GKeyUnlocker* keyunlocker;
  bool      child_setting_standalone;

  /* Admin Setting */
  GObject   * obj_admin_setting;
  GLabel    * lb_admin_setting_back;
  GLabel    * lb_admin_setting_title;
  GList     * list_admin_setting_body;
  GContainer* cont_admin_lock_mode;
  GLabel    * lb_admin_lock_mode;
  GSwitch   * sw_admin_lock_mode;

  /* WiFi Setting */
  GObject   * obj_wifi_setting;
  GLabel    * lb_wifi_setting_back;
  GLabel    * lb_wifi_setting_title;
  GPage     * page_wifi_setting_body;
  GLabel    * lb_ssid_info;
  GLabel    * lb_pass_info;
  GTextArea * ta_ssid;
  GTextArea * ta_password;
  GButton   * btn_wifiscan;
  GContainer* cont_buttons;
  GButton   * btn_wifi_qrcode;
  GButton   * btn_wifi_info;
  GQRCode   * qrcode;
  /* WiFi Scan */
  GObject   * obj_wifiscan;
  GList     * list_wifiscan;
  GLabel    * lb_wifiscan;
  /* WiFi Info */

#if BLYNKGO_USE_BLYNK
  /* Blynk Setting */
  GObject   * obj_blynk_setting;
  GLabel    * lb_blynk_setting_back;
  GLabel    * lb_blynk_setting_title;
  GPage     * page_blynk_setting_body;
  GLabel    * lb_blynk_host_info;
  GLabel    * lb_blynk_port_info;
  GLabel    * lb_blynk_auth_info;
  GTextArea * ta_blynk_host;
  GTextArea * ta_blynk_port;
  GTextArea * ta_blynk_auth;
#endif //BLYNKGO_USE_BLYNK

  /* LINE notify Setting */
  GObject   * obj_line_setting;
  GLabel    * lb_line_setting_back;
  GLabel    * lb_line_setting_title;
  GLabel    * lb_line_token_info;
  GTextArea * ta_line_token;

  /* NTP Setting */
  GObject   * obj_ntp_setting;
  GLabel    * lb_ntp_setting_back;
  GLabel    * lb_ntp_setting_title;
  GPage     * page_ntp_seting_body;
  GLabel    * lb_ntp_server_info;
  GLabel    * lb_ntp_timezone_info;
  GLabel    * lb_ntp_daylight_info;
  GTextArea * ta_ntp_server;
  GTextArea * ta_ntp_timezone;
  GTextArea * ta_ntp_daylight;

  /* GPS Setting */
  GObject   * obj_gps_setting;
  GLabel    * lb_gps_setting_back;
  GLabel    * lb_gps_setting_title;
  GPage     * page_gps_setting_body;
  GLabel    * lb_gps_lat_info;
  GLabel    * lb_gps_long_info;
  GTextArea * ta_gps_lat;
  GTextArea * ta_gps_long;

  /* BlynkGO About */
  GObject   * obj_blynkgo_about;
  GImage    * img_blynkgo_logo;
  GLabel    * lb_blynkgo_text;
  GAnimation* anim;

} blynkgo_manager_ext_t;

class BlynkGO_Manager: public GObject {
  public:
    BlynkGO_Manager(GWidget& parent=GScreen);
    BlynkGO_Manager(GWidget* parent);
    virtual ~BlynkGO_Manager();

    virtual void create();
            void create(GWidget& parent);
            void create(GWidget* parent);

    void      run();
    void      hidden(bool enable);
    bool      hidden();

    void      admin_setting(bool standalone =false);
    void      wifi_setting(bool standalone =false);
#if BLYNKGO_USE_BLYNK
    void      blynk_setting(bool standalone =false);
#endif
    void      line_setting(bool standalone =false);
    void      ntp_setting(bool standalone =false);
    void      gps_setting(bool standalone =false);
    void      blynkgo_about(bool standalone =false);

    // wifi
    String    ssid();
    void      ssid(String ssid);
    String    password();
    void      password(String password);

#if BLYNKGO_USE_BLYNK    
    // blynk
    void      blynk_server(String server);
    String    blynk_server();
    void      blynk_port(uint16_t port);
    uint16_t  blynk_port();
    void      blynk_auth(String auth);
    String    blynk_auth();
#endif // BLYNKGO_USE_BLYNK

    // line notify
    void      line_token(String token);
    String    line_token();

    void      wifi_setting_clickable(bool enable);
  private:
    
};

#define KEY_WIFI_SSID     "SSID"
#define KEY_WIFI_PASS     "PASS"

#define KEY_BLYNK_HOST    "BLYNK_HOST"
#define KEY_BLYNK_PORT    "BLYNK_PORT"
#define KEY_BLYNK_AUTH    "BLYNK_AUTH"

#define KEY_LINE_TOKEN    "LINE_TOKEN"


extern BlynkGO_Manager blynkgo_manager;
extern BlynkGO_Manager* pManager;

#endif//__BLYNKGO_MANAGER_H__
