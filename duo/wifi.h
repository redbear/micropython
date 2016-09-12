#ifndef DUO_WIFI_H_
#define DUO_WIFI_H_

#define      Mode_WLAN_SEC_UNSEC        (0)
#define      Mode_WLAN_SEC_WEP	        (1)
#define      Mode_WLAN_SEC_WPA	        (2)
#define      Mode_WLAN_SEC_WPA2	        (3)
#define      Mode_WLAN_SEC_NOT_SET      (0xff)

#define      Mode_WLAN_CIPHER_NOT_SET   (0)
#define      Mode_WLAN_CIPHER_AES       (1)
#define      Mode_WLAN_CIPHER_TKIP      (2)
#define      Mode_WLAN_CIPHER_AES_TKIP  (3)

extern const mp_obj_type_t pyb_wifi_type;


#endif /* DUO_WIFI_H_ */
