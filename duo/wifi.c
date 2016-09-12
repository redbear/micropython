#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "py/obj.h"
#include "py/nlr.h"
#include "py/runtime.h"
#include "py/binary.h"
#include "py/mphal.h"
#include "wifi.h"
#include "pin.h"
#include "genhdr/pins.h"
#include "wiring.h"

#define bool  _Bool
#define true  1
#define false 0

typedef enum {
	WIFI_STATE_OFF,
	WIFI_STATE_ON,
	WIFI_STATE_CONNECTING,
	WIFI_STATE_CONNECTED,
} WiFi_State_t;

static WiFi_State_t wifi_state = WIFI_STATE_OFF;
static bool connect_failed = false;

STATIC mp_obj_t pyb_wifi_on() {
	if(!connect_failed)
		wifi_on();
	if(wifi_state < WIFI_STATE_ON)
		wifi_state = WIFI_STATE_ON;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_on_obj, pyb_wifi_on);

STATIC mp_obj_t pyb_wifi_off() {

	wifi_off();
	wifi_state = WIFI_STATE_OFF;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_off_obj, pyb_wifi_off);

STATIC mp_obj_t pyb_wifi_connect() {
	uint32_t cur_time = millis();

	if(wifi_hasCredentials()) {
		wifi_connect();
		wifi_state = WIFI_STATE_CONNECTING;
		while(!wifi_isReady() && (millis() - cur_time) < 5000) {
		}

		if(wifi_isReady()) {
			wifi_state = WIFI_STATE_CONNECTED;
			connect_failed = false;
		}
		else {
			wifi_off();
			connect_failed = true;
			wifi_state = WIFI_STATE_OFF;
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "Connect to AP failed! Please try again!"));
		}
	} else {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "No WiFi credentials available in Duo.\n \
            Use WiFi.setCredential({\"ssid\":\"YOUR_SSID\", \"password\":\"YOUR_PASS_WORD\", \"sec\":\"SECUTITY_TYPE\", \"cipher\":\"CIPHER_TYPE\"})\n \
            to set a valid credential first."));
	}

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_connect_obj, pyb_wifi_connect);

STATIC mp_obj_t pyb_wifi_disconnect() {
	wifi_disconnect();
	if(wifi_state > WIFI_STATE_ON)
		wifi_state = WIFI_STATE_ON;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_disconnect_obj, pyb_wifi_disconnect);

STATIC mp_obj_t pyb_wifi_connecting() {

	if (mp_obj_is_true(MP_OBJ_NEW_SMALL_INT(wifi_isConnecting()))) {
		return mp_const_true;
	} else {
		return mp_const_false;
	}
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_connecting_obj, pyb_wifi_connecting);

STATIC mp_obj_t pyb_wifi_ready() {

	if (wifi_isReady()) {
		return mp_const_true;
	} else {
		return mp_const_false;
	}
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_ready_obj, pyb_wifi_ready);

STATIC mp_obj_t pyb_wifi_listen(mp_uint_t n_args, const mp_obj_t *args) {

		if(n_args == 0) {
			wifi_startListen();
		} else {
			if (mp_obj_is_true(args[0])) {
				wifi_startListen();
			} else {
				wifi_stopListen();
			}
		}

		while(wifi_isListening());

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_wifi_listen_obj, 0, 1, pyb_wifi_listen);

STATIC mp_obj_t pyb_wifi_listening() {

	if (mp_obj_is_true(MP_OBJ_NEW_SMALL_INT(wifi_isListening()))) {
		return mp_const_true;
	} else {
		return mp_const_false;
	}
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_listening_obj, pyb_wifi_listening);

STATIC mp_obj_t pyb_wifi_set_credentials(mp_uint_t n_args, const mp_obj_t *args) {

	const char* ssid = mp_obj_str_get_str(args[0]);
	const char* password;
	uint32_t security;
	uint32_t cipher;

	if(wifi_state == WIFI_STATE_OFF)
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "WiFi is turned off. Use WiFi.on() to enable WiFi first."));
	if(ssid[0] == '\0')
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The length of SSID is equal to 0."));

	if(n_args == 4)
	{
		password = mp_obj_str_get_str(args[1]);
		security = mp_obj_get_int(args[2]);
		cipher = mp_obj_get_int(args[3]);

		printf("ssid : %s\npassword : %s \nsecurity : %lu\ncipher : %lu\n", ssid, password, security, cipher);
	} else if(n_args == 2)
	{
		password = "00000000";
		security = mp_obj_get_int(args[1]);
		cipher = WLAN_CIPHER_NOT_SET;
		printf("ssid : %s\nsecurity : %lu\n", ssid, security);
	} else
	{
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_TypeError, "argument num/types mismatch"));
	}

	if(connect_failed)
		wifi_on();
	wifi_setCredentials(ssid, password, security, cipher);
	if(connect_failed)
		wifi_off();

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_wifi_set_credentials_obj, 2, 4, pyb_wifi_set_credentials);

STATIC char *get_security(int num) {
	static char security[16];
	memset(security, '\0', 16);

	switch(num) {
	case 0 :
		strcpy(security, "WLAN_SEC_UNSEC");
		break;
	case 1 :
		strcpy(security, "WLAN_SEC_WEP");
		break;
	case 2 :
		strcpy(security, "WLAN_SEC_WPA");
		break;
	case 3 :
		strcpy(security, "WLAN_SEC_WPA2");
		break;
	case 0xff :
		strcpy(security, "WLAN_SEC_NOT_SET");
		break;
	default :
		strcpy(security, "WLAN_SEC_UNKNOWN");
		break;
	}

	return security;
}

STATIC char *get_cipher(int num) {
	static char cipher[16];
	memset(cipher, '\0', 16);

	switch(num) {
	case 0 :
		strcpy(cipher, "WLAN_CIPHER_NOT_SET");
		break;
	case 1 :
		strcpy(cipher, "WLAN_CIPHER_AES");
		break;
	case 2 :
		strcpy(cipher, "WLAN_CIPHER_TKIP");
		break;
	case 3 :
		strcpy(cipher, "WLAN_CIPHER_AES_TKIP");
		break;
	default :
		break;
	}

	return cipher;
}

STATIC mp_obj_t pyb_wifi_get_credentials(mp_obj_t count) {
	size_t result_count = mp_obj_get_int(count);
	WiFiAccessPoint results[result_count];
	int found = wifi_getCredentials(results, result_count);
	char buffer[128];
	int i = 0, num = 0;
	if(found < result_count)
		num = found;
	else
		num = result_count;

	for(; i < num ; i++) {
		uint8_t ssidLen = results[i].ssidLength;
		results[i].ssid[ssidLen] = '\0';
		memset(buffer, '\0', 128);
		int j = 0;
		j = sprintf(buffer, "ssid%d : %s    ", i, results[i].ssid);
		printf("ssid : %s\n", results[i].ssid);

		j += sprintf(buffer + j, "security%d : %s    ", i, get_security(results[i].security));
		printf("security : %s\n", get_security(results[i].security));

		j += sprintf(buffer + j, "cipher%d : %s    ", i, get_cipher(results[i].cipher));
		printf("cipher : %s\n\n", get_cipher(results[i].cipher));

	}

	return MP_OBJ_NEW_SMALL_INT(num);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_wifi_get_credentials_obj, pyb_wifi_get_credentials);

STATIC mp_obj_t pyb_wifi_has_credentials() {

	if (wifi_hasCredentials()) {
		return mp_const_true;
	} else {
		return mp_const_false;
	}
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_has_credentials_obj, pyb_wifi_has_credentials);

STATIC mp_obj_t pyb_wifi_clear_credentials() {

	if (mp_obj_is_true(MP_OBJ_NEW_SMALL_INT(wifi_clearCredentials()))) {
		return mp_const_true;
	} else {
		return mp_const_false;
	}
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_clear_credentials_obj, pyb_wifi_clear_credentials);

STATIC mp_obj_t pyb_wifi_mac_address() {
	uint8_t mac[6];
	int i = 0;

	wifi_macAddress(mac);

	printf("%d:%d:%d:%d:%d:%d\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_mac_address_obj, pyb_wifi_mac_address);

STATIC mp_obj_t pyb_wifi_SSID() {
	const char* ssid = NULL;
	ssid = wifi_SSID();

    return mp_obj_new_str(ssid, strlen(ssid), true);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_SSID_obj, pyb_wifi_SSID);

STATIC mp_obj_t pyb_wifi_BSSID() {
	uint8_t mac[6];
	int i = 0;

	if(wifi_state < WIFI_STATE_CONNECTED) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "WiFi hasn't connected to AP yet.."));
	}

	wifi_BSSID(mac);

	printf("%d:%d:%d:%d:%d:%d\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_BSSID_obj, pyb_wifi_BSSID);

STATIC mp_obj_t pyb_wifi_RSSI() {

    return MP_OBJ_NEW_SMALL_INT(wifi_RSSI());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_RSSI_obj, pyb_wifi_RSSI);

STATIC void conversion_IP(char *ip_string, uint8_t *ip) {
	int i = 0;
	char *result = strtok(ip_string, ".");

		ip[i++] = (uint8_t)atoi(result);
		while(result != NULL) {
			result = strtok(NULL, ".");
			ip[i++] = (uint8_t)atoi(result);
		}

		return;
}

STATIC mp_obj_t pyb_wifi_ping(mp_obj_t buf_in, mp_obj_t nTries) {
	int i = 0;
	const char *ip_string = mp_obj_str_get_str(buf_in);
	uint8_t ip[4];

	if(wifi_state < WIFI_STATE_CONNECTED) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "WiFi hasn't connected to AP yet.."));
	}
	conversion_IP((char *)ip_string, ip);

    return MP_OBJ_NEW_SMALL_INT(wifi_ping(ip, mp_obj_get_int(nTries)));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_wifi_ping_obj, pyb_wifi_ping);

STATIC mp_obj_t pyb_wifi_scan(mp_obj_t count) {
	size_t result_count = mp_obj_get_int(count);
	WiFiAccessPoint results[result_count];
	char buffer[128];

	if(wifi_state == WIFI_STATE_OFF)
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "WiFi is turned off. Use WiFi.on() to enable WiFi first."));

	int found = wifi_scan(results, result_count);
	int i = 0, num = 0;
	if(found < result_count)
		num = found;
	else
		num = result_count;

	for(; i < num ; i++) {
		uint8_t ssidLen = results[i].ssidLength;
		results[i].ssid[ssidLen] = '\0';
		memset(buffer, '\0', 128);
		int j = 0;

		j = sprintf(buffer, "ssid%d : %s    ", i, results[i].ssid);
		printf("ssid : %s\n", results[i].ssid);

		j += sprintf(buffer + j, "security%d : %s    ", i, get_security(results[i].security));
		printf("security : %s\n", get_security(results[i].security));

		j += sprintf(buffer + j, "cipher%d : %s    ", i, get_cipher(results[i].cipher));
		printf("cipher : %s\n", get_cipher(results[i].cipher));

		j += sprintf(buffer + j, "RSSI%d : %d    ", i, results[i].rssi);
		printf("RSSI : %d\n\n", results[i].rssi);
	}

	return MP_OBJ_NEW_SMALL_INT(num);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_wifi_scan_obj, pyb_wifi_scan);

STATIC mp_obj_t pyb_wifi_resolve(mp_obj_t buf_in, mp_obj_t data_in) {
	uint8_t ip[4];
	char buffer[8];

	if(wifi_state < WIFI_STATE_CONNECTED) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "WiFi hasn't connected to AP yet.."));
	}
	const char* name = mp_obj_str_get_str(buf_in);
	wifi_resolve(name, ip);

	printf("%d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
	sprintf(buffer, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	mp_obj_list_append(data_in,mp_obj_new_str(buffer, strlen(buffer), true));

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_wifi_resolve_obj, pyb_wifi_resolve);

STATIC mp_obj_t pyb_wifi_local_IP() {
	uint8_t local_ip[4];
	char buffer[8];

	if(wifi_state < WIFI_STATE_CONNECTED) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "WiFi hasn't connected to AP yet.."));
	}

	wifi_localIP(local_ip);

	printf("%d.%d.%d.%d\n", local_ip[0], local_ip[1], local_ip[2], local_ip[3]);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_local_IP_obj, pyb_wifi_local_IP);

STATIC mp_obj_t pyb_wifi_subnet_mask() {
	uint8_t subnet_mask[4];
	char buffer[8];

	if(wifi_state < WIFI_STATE_CONNECTED) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "WiFi hasn't connected to AP yet.."));
	}

	wifi_subnetMask(subnet_mask);

	printf("%d.%d.%d.%d\n", subnet_mask[0], subnet_mask[1], subnet_mask[2], subnet_mask[3]);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_subnet_mask_obj, pyb_wifi_subnet_mask);

STATIC mp_obj_t pyb_wifi_gateway_IP() {
	uint8_t gateway_ip[4];
	char buffer[8];

	if(wifi_state < WIFI_STATE_CONNECTED) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "WiFi hasn't connected to AP yet.."));
	}

	wifi_gatewayIP(gateway_ip);

	printf("%d.%d.%d.%d\n", gateway_ip[0], gateway_ip[1], gateway_ip[2], gateway_ip[3]);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_gateway_IP_obj, pyb_wifi_gateway_IP);

STATIC mp_obj_t pyb_wifi_dns_server_IP() {
	uint8_t dns_server_ip[4];
	char buffer[8];

	if(wifi_state < WIFI_STATE_CONNECTED) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "WiFi hasn't connected to AP yet.."));
	}

	wifi_dnsServerIP(dns_server_ip);

	printf("%d.%d.%d.%d\n", dns_server_ip[0], dns_server_ip[1], dns_server_ip[2], dns_server_ip[3]);

    return mp_const_none;

}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_dns_server_IP_obj, pyb_wifi_dns_server_IP);

STATIC mp_obj_t pyb_wifi_dhcp_server_IP() {
	uint8_t dhcp_server_ip[4];
	char buffer[8];

	if(wifi_state < WIFI_STATE_CONNECTED) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "WiFi hasn't connected to AP yet.."));
	}

	wifi_dhcpServerIP(dhcp_server_ip);

	printf("%d.%d.%d.%d\n", dhcp_server_ip[0], dhcp_server_ip[1], dhcp_server_ip[2], dhcp_server_ip[3]);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_dhcp_server_IP_obj, pyb_wifi_dhcp_server_IP);

STATIC mp_obj_t pyb_wifi_set_static_IP(mp_uint_t n_args, const mp_obj_t *args) {
	int i = 0;
	const char* buffer1 = mp_obj_str_get_str(args[0]);
	uint8_t host_ip[4];
	conversion_IP((char *)buffer1, host_ip);

	const char* buffer2 = mp_obj_str_get_str(args[1]);
	uint8_t netmask_ip[4];
	conversion_IP((char *)buffer2, netmask_ip);

	const char* buffer3 = mp_obj_str_get_str(args[2]);
	uint8_t gateway_ip[4];
	conversion_IP((char *)buffer3, gateway_ip);

	const char* buffer4 = mp_obj_str_get_str(args[3]);
	uint8_t dns_ip[4];
	conversion_IP((char *)buffer4, dns_ip);

	wifi_setStaticIP(host_ip, netmask_ip, gateway_ip, dns_ip);

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_wifi_set_static_IP_obj, 4, 4, pyb_wifi_set_static_IP);

STATIC mp_obj_t pyb_wifi_use_static_IP() {

	wifi_useStaticIP();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_use_static_IP_obj, pyb_wifi_use_static_IP);

STATIC mp_obj_t pyb_wifi_use_dynamic_IP() {

	wifi_useDynamicIP();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_wifi_use_dynamic_IP_obj, pyb_wifi_use_dynamic_IP);

STATIC const mp_map_elem_t wifi_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_on), (mp_obj_t)&pyb_wifi_on_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_off), (mp_obj_t)&pyb_wifi_off_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_connect), (mp_obj_t)&pyb_wifi_connect_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_disconnect), (mp_obj_t)&pyb_wifi_disconnect_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_connecting), (mp_obj_t)&pyb_wifi_connecting_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_ready), (mp_obj_t)&pyb_wifi_ready_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_listen), (mp_obj_t)&pyb_wifi_listen_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_listening), (mp_obj_t)&pyb_wifi_listening_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_setCredentials), (mp_obj_t)&pyb_wifi_set_credentials_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_getCredentials), (mp_obj_t)&pyb_wifi_get_credentials_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_clearCredentials), (mp_obj_t)&pyb_wifi_clear_credentials_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_hasCredentials), (mp_obj_t)&pyb_wifi_has_credentials_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_macAddress), (mp_obj_t)&pyb_wifi_mac_address_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_SSID), (mp_obj_t)&pyb_wifi_SSID_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_BSSID), (mp_obj_t)&pyb_wifi_BSSID_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_RSSI), (mp_obj_t)&pyb_wifi_RSSI_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_ping), (mp_obj_t)&pyb_wifi_ping_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_scan), (mp_obj_t)&pyb_wifi_scan_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_resolve), (mp_obj_t)&pyb_wifi_resolve_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_localIP), (mp_obj_t)&pyb_wifi_local_IP_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_subnetMask), (mp_obj_t)&pyb_wifi_subnet_mask_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_gatewayIP), (mp_obj_t)&pyb_wifi_gateway_IP_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_dnsServerIP), (mp_obj_t)&pyb_wifi_dns_server_IP_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_dhcpServerIP), (mp_obj_t)&pyb_wifi_dhcp_server_IP_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_setStaticIP), (mp_obj_t)&pyb_wifi_set_static_IP_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_useStaticIP), (mp_obj_t)&pyb_wifi_use_static_IP_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_useDynamicIP), (mp_obj_t)&pyb_wifi_use_dynamic_IP_obj},

    // class constants
	{ MP_OBJ_NEW_QSTR(MP_QSTR_WLAN_SEC_UNSEC),      MP_OBJ_NEW_SMALL_INT(Mode_WLAN_SEC_UNSEC) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_WLAN_SEC_WEP),        MP_OBJ_NEW_SMALL_INT(Mode_WLAN_SEC_WEP) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_WLAN_SEC_WPA),        MP_OBJ_NEW_SMALL_INT(Mode_WLAN_SEC_WPA) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_WLAN_SEC_WPA2),       MP_OBJ_NEW_SMALL_INT(Mode_WLAN_SEC_WPA2) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_WLAN_SEC_NOT_SET),    MP_OBJ_NEW_SMALL_INT(Mode_WLAN_SEC_NOT_SET) },

	{ MP_OBJ_NEW_QSTR(MP_QSTR_WLAN_CIPHER_NOT_SET),    MP_OBJ_NEW_SMALL_INT(Mode_WLAN_CIPHER_NOT_SET) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_WLAN_CIPHER_AES),        MP_OBJ_NEW_SMALL_INT(Mode_WLAN_CIPHER_AES) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_WLAN_CIPHER_TKIP),       MP_OBJ_NEW_SMALL_INT(Mode_WLAN_CIPHER_TKIP) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_WLAN_CIPHER_AES_TKIP),   MP_OBJ_NEW_SMALL_INT(Mode_WLAN_CIPHER_AES_TKIP) },
};

STATIC MP_DEFINE_CONST_DICT(wifi_locals_dict, wifi_locals_dict_table);

const mp_obj_type_t pyb_wifi_type = {
    { &mp_type_type },
    .name = MP_QSTR_WIFI,
//    .make_new = wifi_make_new,
    .locals_dict = (mp_obj_t)&wifi_locals_dict,
};
