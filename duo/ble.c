#include <stdio.h>
#include <string.h>

#include "py/nlr.h"
#include "py/runtime.h"
#include "py/binary.h"
#include "py/mphal.h"
#include "pin.h"
#include "genhdr/pins.h"
#include "wiring.h"
#include "ble.h"

#define MIN_CONN_INTERVAL          0x0028 // 50ms.
#define MAX_CONN_INTERVAL          0x0190 // 500ms.
#define SLAVE_LATENCY              0x0000 // No slave latency.
#define CONN_SUPERVISION_TIMEOUT   0x03E8 // 10s.

// Learn about appearance: http://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.gap.appearance.xml
#define BLE_PERIPHERAL_APPEARANCE  BLE_APPEARANCE_UNKNOWN

#define BLE_DEVICE_NAME            "Duo-Python"

// Length of characteristic value.
#define CHARACTERISTIC1_MAX_LEN    256
#define CHARACTERISTIC2_MAX_LEN    20

/******************************************************
 *               Variable Definitions
 ******************************************************/
// Primary service 128-bits UUID
static uint8_t service1_uuid[16] = { 0x71,0x3d,0x00,0x00,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };
// Characteristics 128-bits UUID
static uint8_t char1_uuid[16]    = { 0x71,0x3d,0x00,0x02,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };
static uint8_t char2_uuid[16]    = { 0x71,0x3d,0x00,0x03,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };

// GAP and GATT characteristics value
static uint8_t  appearance[2] = {
  LOW_BYTE(BLE_PERIPHERAL_APPEARANCE),
  HIGH_BYTE(BLE_PERIPHERAL_APPEARANCE)
};

static uint8_t  change[4] = {
  0x00, 0x00, 0xFF, 0xFF
};

static uint8_t  conn_param[8] = {
  LOW_BYTE(MIN_CONN_INTERVAL), HIGH_BYTE(MIN_CONN_INTERVAL),
  LOW_BYTE(MAX_CONN_INTERVAL), HIGH_BYTE(MAX_CONN_INTERVAL),
  LOW_BYTE(SLAVE_LATENCY), HIGH_BYTE(SLAVE_LATENCY),
  LOW_BYTE(CONN_SUPERVISION_TIMEOUT), HIGH_BYTE(CONN_SUPERVISION_TIMEOUT)
};

/*
 * BLE peripheral advertising parameters:
 *     - advertising_interval_min: [0x0020, 0x4000], default: 0x0800, unit: 0.625 msec
 *     - advertising_interval_max: [0x0020, 0x4000], default: 0x0800, unit: 0.625 msec
 *     - advertising_type:
 *           BLE_GAP_ADV_TYPE_ADV_IND
 *           BLE_GAP_ADV_TYPE_ADV_DIRECT_IND
 *           BLE_GAP_ADV_TYPE_ADV_SCAN_IND
 *           BLE_GAP_ADV_TYPE_ADV_NONCONN_IND
 *     - own_address_type:
 *           BLE_GAP_ADDR_TYPE_PUBLIC
 *           BLE_GAP_ADDR_TYPE_RANDOM
 *     - advertising_channel_map:
 *           BLE_GAP_ADV_CHANNEL_MAP_37
 *           BLE_GAP_ADV_CHANNEL_MAP_38
 *           BLE_GAP_ADV_CHANNEL_MAP_39
 *           BLE_GAP_ADV_CHANNEL_MAP_ALL
 *     - filter policies:
 *           BLE_GAP_ADV_FP_ANY
 *           BLE_GAP_ADV_FP_FILTER_SCANREQ
 *           BLE_GAP_ADV_FP_FILTER_CONNREQ
 *           BLE_GAP_ADV_FP_FILTER_BOTH
 *
 * Note:  If the advertising_type is set to BLE_GAP_ADV_TYPE_ADV_SCAN_IND or BLE_GAP_ADV_TYPE_ADV_NONCONN_IND,
 *        the advertising_interval_min and advertising_interval_max should not be set to less than 0x00A0.
 */
static advParams_t adv_params = {
  .adv_int_min   = 0x0030,
  .adv_int_max   = 0x0030,
  .adv_type      = BLE_GAP_ADV_TYPE_ADV_IND,
  .dir_addr_type = BLE_GAP_ADDR_TYPE_PUBLIC,
  .dir_addr      = {0,0,0,0,0,0},
  .channel_map   = BLE_GAP_ADV_CHANNEL_MAP_ALL,
  .filter_policy = BLE_GAP_ADV_FP_ANY
};

// BLE peripheral advertising data
static uint8_t adv_data[] = {
  0x02,
  BLE_GAP_AD_TYPE_FLAGS,
  BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE,

  0x11,
  BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE,
  0x1e, 0x94, 0x8d, 0xf1, 0x48, 0x31, 0x94, 0xba, 0x75, 0x4c, 0x3e, 0x50, 0x00, 0x00, 0x3d, 0x71
};

// BLE peripheral scan respond data
static uint8_t scan_response[] = {
	0x0b,
	BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
	'D', 'u',  'o', '-', 'P', 'y', 't', 'h', 'o', 'n'
};

// Characteristic value handle
static uint16_t character1_handle = 0x0000;
static uint16_t character2_handle = 0x0000;

// Buffer of characterisitc value.
static uint8_t characteristic1_data[CHARACTERISTIC1_MAX_LEN] = { 0x00 };
static uint8_t characteristic2_data[CHARACTERISTIC2_MAX_LEN] = { 0x00 };

static uint16_t characteristic2_data_put = 0;
static uint16_t characteristic1_data_get = 0;
static uint16_t characteristic1_data_sum = 0;
static uint16_t characteristic2_data_sum = 0;

static uint8_t isConnected = 0;
static uint16_t conn_handle = 0;

STATIC int addring(int i) {
	return (i + 1) == CHARACTERISTIC1_MAX_LEN ? 0 : i + 1;
}

STATIC int get_data() {
	int pos;
	if(characteristic1_data_sum > 0) {
		pos = characteristic1_data_get;
		characteristic1_data_get = addring(characteristic1_data_get);
		characteristic1_data_sum--;

		return characteristic1_data[pos];
	} else {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "Receive data buffer is empty!"));
	}
}

STATIC void put_data(uint8_t data) {
	if(characteristic2_data_sum < CHARACTERISTIC2_MAX_LEN) {
		characteristic2_data[characteristic2_data_put] = data;
		characteristic2_data_put = addring(characteristic2_data_put);
		characteristic2_data_sum++;
	} else {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "Receive data buffer is full!"));
	}
}

STATIC void deviceConnectedCallback(BLEStatus_t status, uint16_t handle) {
  switch (status) {
    case BLE_STATUS_OK:
      isConnected = 1;
      printf("Device connected!\n");
      conn_handle = handle;
      break;
    default: break;
  }
}

STATIC void deviceDisconnectedCallback(uint16_t handle) {
  isConnected = 0;
  printf("Disconnected!\n");
}

STATIC int gattWriteCallback(uint16_t value_handle, uint8_t *buffer, uint16_t size) {
  printf("Write value handler: ");
  printf("0x%x\n",value_handle);

  if (character1_handle == value_handle) {
	  for(int i = 0; i < size; i++) {
		  put_data(buffer[i]);
	  }
  }
  else if (character2_handle+1 == value_handle) {
    printf("Characteristic1 cccd write value: ");
    for (uint8_t index = 0; index < size; index++) {
      printf("0x%x",buffer[index]);
      printf(" ");
    }
    printf(" \n");
  }
  return 0;
}

STATIC void characteristic2_notify(uint8_t *buf, uint16_t size) {
  printf("characteristic2_notify\n");

  ble_sendNotify(character2_handle, buf, size);
}

void ble_init0(void) {
	ble_init();

	// Register BLE callback functions.
	ble_onConnectedCallback(deviceConnectedCallback);
	ble_onDisconnectedCallback(deviceDisconnectedCallback);
	ble_onDataWriteCallback(gattWriteCallback);

	// Add GAP service and characteristics
	ble_addServiceUUID16(BLE_UUID_GAP);
	ble_addCharacteristicUUID16(BLE_UUID_GAP_CHARACTERISTIC_DEVICE_NAME, ATT_PROPERTY_READ|ATT_PROPERTY_WRITE, (uint8_t*)BLE_DEVICE_NAME, sizeof(BLE_DEVICE_NAME));
	ble_addCharacteristicUUID16(BLE_UUID_GAP_CHARACTERISTIC_APPEARANCE, ATT_PROPERTY_READ, appearance, sizeof(appearance));
	ble_addCharacteristicUUID16(BLE_UUID_GAP_CHARACTERISTIC_PPCP, ATT_PROPERTY_READ, conn_param, sizeof(conn_param));

	// Add GATT service and characteristics
	ble_addServiceUUID16(BLE_UUID_GATT);
	ble_addCharacteristicUUID16(BLE_UUID_GATT_CHARACTERISTIC_SERVICE_CHANGED, ATT_PROPERTY_INDICATE, change, sizeof(change));

	// Add primary service1.
	ble_addServiceUUID128(service1_uuid);
	// Add characteristic to service1, return value handle of characteristic.
	character1_handle = ble_addCharacteristicDynamicUUID128(char1_uuid, ATT_PROPERTY_WRITE_WITHOUT_RESPONSE, characteristic1_data, CHARACTERISTIC1_MAX_LEN);
	character2_handle = ble_addCharacteristicDynamicUUID128(char2_uuid, ATT_PROPERTY_NOTIFY, characteristic2_data, CHARACTERISTIC2_MAX_LEN);

	// Set BLE advertising parameters
	ble_setAdvParams(&adv_params);

	// Set BLE advertising and scan respond data
	ble_setAdvData(sizeof(adv_data), adv_data);
	ble_setScanRspData(sizeof(scan_response), scan_response);
}

STATIC mp_obj_t ble_begin() {
	ble_startAdvertising();
	printf("BLE start advertising.\n");

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ble_begin_obj, ble_begin);

STATIC mp_obj_t ble_stop() {
	ble_stopAdvertising();
	printf("BLE stop advertising.\n");

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ble_stop_obj, ble_stop);

STATIC mp_obj_t ble_write_data(uint8_t buf[]) {
	uint8_t buf_temp[CHARACTERISTIC2_MAX_LEN];
	int i = 0;
	int count = 0;
	uint16_t size = 0;

	while(1) {
		if((buf_temp[i] = buf[count]) == '\0') {
			break;
		}
		i++;
		count++;

		if(i == CHARACTERISTIC2_MAX_LEN) {
			characteristic2_notify(buf_temp, CHARACTERISTIC2_MAX_LEN);
			memset(buf_temp, '\0', CHARACTERISTIC2_MAX_LEN);
			i = 0;
		}
	}
	characteristic2_notify(buf_temp, i);
	size = count;

    return MP_OBJ_NEW_SMALL_INT(size);
}

STATIC mp_obj_t ble_write(mp_obj_t buf_in) {
	int i =0;

    if(MP_OBJ_IS_STR(buf_in)) {
    	const char *buf = mp_obj_str_get_str(buf_in);
    	return ble_write_data((uint8_t *)buf);

    } else {
    	mp_obj_list_t *buffer = MP_OBJ_TO_PTR(buf_in);
    	uint8_t buf[buffer->len + 1];
    	buf[buffer->len] = '\0';

    	for(i = 0; i < buffer->len; i++) {
    		buf[i] = mp_obj_get_int(buffer->items[i]);
    	}

    	return ble_write_data(buf);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ble_write_obj, ble_write);

STATIC mp_obj_t ble_read(mp_obj_t buf_in, mp_obj_t size_in) {
	int i = 0, num = 0;
	char data = 0;

	num = characteristic1_data_sum > mp_obj_get_int(size_in) ? mp_obj_get_int(size_in) : characteristic1_data_sum;

	for(; i < num; i++) {
	  	mp_obj_list_append(buf_in, MP_OBJ_NEW_SMALL_INT(get_data()));
	}

    return MP_OBJ_NEW_SMALL_INT(num);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(ble_read_obj, ble_read);

STATIC mp_obj_t ble_available() {

	return MP_OBJ_NEW_SMALL_INT(characteristic1_data_sum);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ble_available_obj, ble_available);

STATIC mp_obj_t ble_connected() {
	if(isConnected)
		return mp_const_true;
	else
		return mp_const_false;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ble_connected_obj, ble_connected);

STATIC mp_obj_t pyb_ble_disconnect() {
	ble_disconnect(conn_handle);

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_ble_disconnect_obj, pyb_ble_disconnect);

STATIC const mp_map_elem_t ble_locals_dict_table[] = {
	{ MP_OBJ_NEW_QSTR(MP_QSTR_begin), (mp_obj_t)&ble_begin_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_stop), (mp_obj_t)&ble_stop_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&ble_read_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_write), (mp_obj_t)&ble_write_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_available), (mp_obj_t)&ble_available_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_connected), (mp_obj_t)&ble_connected_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_disconnect), (mp_obj_t)&pyb_ble_disconnect_obj},
};

STATIC MP_DEFINE_CONST_DICT(ble_locals_dict, ble_locals_dict_table);

const mp_obj_type_t pyb_ble_type = {
    { &mp_type_type },
    .name = MP_QSTR_BLE,
    .locals_dict = (mp_obj_t)&ble_locals_dict,
};



