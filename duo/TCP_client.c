#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "py/nlr.h"
#include "py/runtime.h"
#include "py/binary.h"
#include "py/mphal.h"
#include "pin.h"
#include "genhdr/pins.h"
#include "wiring.h"
#include "TCP_client.h"

typedef enum {
  SOCKET_STATE_UNUSED,
  SOCKET_STATE_USED,
}socket_state_t;

typedef struct {
  tcp_server *server;
  sock_handle_t socket_id;
  socket_state_t socket_state;
}tcp_server_t;

typedef struct {
  tcp_client *client;
  sock_handle_t socket_id;
  socket_state_t socket_state;
}tcp_client_t;

static tcp_server_t TCP_server = {
		.server = NULL,
		.socket_id = 0,
		.socket_state = SOCKET_STATE_UNUSED,
};
static tcp_client_t TCP_client = {
		.client = NULL,
		.socket_id = 0,
		.socket_state = SOCKET_STATE_UNUSED,
};

typedef struct _pyb_TCP_client_obj_t {
    mp_obj_base_t base;
}pyb_TCP_client_obj_t;

STATIC mp_obj_t pyb_TCP_client_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);

	pyb_TCP_client_obj_t *self = m_new0(pyb_TCP_client_obj_t, 1);
    self->base.type = &pyb_TCP_client_type;

	TCP_client.client = TCPClient_newTCPClient();
	TCP_client.socket_state = SOCKET_STATE_UNUSED;

	if(TCP_client.client != NULL)
		printf("true\n");
	else
		printf("false\n");

    return self;
}

STATIC uint32_t conversion_IP(char * ip_string) {
	uint32_t ip = 0;
	uint32_t ip_temp[4];
	int i = 0;
	char *result = strtok(ip_string, ".");
	ip_temp[i++] = (uint32_t)atoi(result);
	while(result != NULL) {
		result = strtok(NULL, ".");
		ip_temp[i++] = (uint32_t)atoi(result);
	}

	ip = ip_temp[0] << 24 | ip_temp[1] << 16 | ip_temp[2] << 8 | ip_temp[3];

	return ip;
}

STATIC mp_obj_t TCP_client_connect_by_IP(mp_obj_t self, mp_obj_t ip_in, mp_obj_t port_in) {
	const char * const ip_string = mp_obj_str_get_str(ip_in);
	uint16_t port = mp_obj_get_int(port_in);
	uint32_t ip = conversion_IP((char *)ip_string);

	if(TCP_client.client == NULL) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The client does not exist!"));
	}

	if(TCP_client.socket_state == SOCKET_STATE_USED) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The client already used!"));
	}

	if (TCPClient_connectByIP(TCP_client.client, ip, port)) {
		TCP_client.socket_state = SOCKET_STATE_USED;
		return mp_const_true;
	} else {
		return mp_const_false;
	}
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(TCP_client_connect_by_IP_obj, TCP_client_connect_by_IP);

STATIC mp_obj_t TCP_client_connect_by_host_name(mp_obj_t self, mp_obj_t host_in, mp_obj_t port_in) {
	const char *host = mp_obj_str_get_str(host_in);
	uint16_t port = mp_obj_get_int(port_in);

	if(TCP_client.client == NULL) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The client does not exist!"));
	}

	if(TCP_client.socket_state == SOCKET_STATE_USED) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The client already used!"));
	}

	if (TCPClient_connectByHostName(TCP_client.client, host, port)) {
		TCP_client.socket_state = SOCKET_STATE_USED;
		return mp_const_true;
	} else {
		return mp_const_false;
	}
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(TCP_client_connect_by_host_name_obj, TCP_client_connect_by_host_name);

STATIC mp_obj_t TCP_client_stop(mp_obj_t self) {

	if(TCP_client.client == NULL) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The client does not exist!"));
	}

	if(TCP_client.socket_state == SOCKET_STATE_USED) {

	} else {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The client already unused!"));
	}

	TCPClient_stop(TCP_client.client);

	TCP_client.socket_state = SOCKET_STATE_UNUSED;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(TCP_client_stop_obj, TCP_client_stop);

STATIC mp_obj_t delete_TCP_client(mp_obj_t self) {

	TCPClient_deleteTCPClient(TCP_client.client);
	TCP_client.client = NULL;
	TCP_client.socket_state = SOCKET_STATE_UNUSED;

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(delete_TCP_client_obj, delete_TCP_client);

STATIC mp_obj_t TCP_client_available(mp_obj_t self) {
	if(TCP_client.client == NULL) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The client does not exist!"));
	}

	if(TCP_client.socket_state == SOCKET_STATE_USED) {

	} else {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The client already unused!"));
	}

	return MP_OBJ_NEW_SMALL_INT(TCPClient_available(TCP_client.client));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(TCP_client_available_obj, TCP_client_available);

mp_obj_t TCP_client_write_data(tcp_client * client_in, mp_obj_t buf_in) {
	tcp_client_t client = {
				.client = client_in,
				.socket_id = 0,
				.socket_state = SOCKET_STATE_UNUSED,
		};

	if(client.client == NULL) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The client does not exist!"));
	}

	int size = 0;

    if(MP_OBJ_IS_STR(buf_in)) {
    	const char *buf = mp_obj_str_get_str(buf_in);
    	size = TCPClient_write(client.client, (uint8_t *)buf, (size_t)strlen(buf));
    } else {
    	int i = 0;
    	mp_obj_list_t *buffer = MP_OBJ_TO_PTR(buf_in);
    	uint8_t buf[buffer->len];

    	for(; i < buffer->len; i++) {
    		buf[i] = mp_obj_get_int(buffer->items[i]);
    		printf("buf[%d] = %d\n", i, buf[i]);
    	}
    	size = TCPClient_write(client.client, buf, buffer->len);
    }

    return MP_OBJ_NEW_SMALL_INT(size);
}

STATIC mp_obj_t TCP_client_write(mp_obj_t self, mp_obj_t buf_in) {


    return TCP_client_write_data(TCP_client.client, buf_in);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(TCP_client_write_obj, TCP_client_write);

mp_obj_t TCP_client_read_data(tcp_client * client_in, mp_obj_t buf_in, mp_obj_t size_in) {
	tcp_client_t client = {
			.client = client_in,
			.socket_id = 0,
			.socket_state = SOCKET_STATE_UNUSED,
	};

	if(client.client == NULL) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The client does not exist!"));
	}

	int size = 0;
	int i = 0, num = 0;
	mp_obj_list_t *buffer = MP_OBJ_TO_PTR(buf_in);
	uint8_t *buf = NULL;
	buf = (uint8_t *)malloc(buffer->len);
	size = TCPClient_read(client.client, buf, (size_t)mp_obj_get_int(size_in));

	num = size > mp_obj_get_int(size_in) ? mp_obj_get_int(size_in) : size;

	for(; i < num; i++) {

	  	mp_obj_list_append(buf_in, MP_OBJ_NEW_SMALL_INT(buf[i]));
	}

	free(buf);

    return MP_OBJ_NEW_SMALL_INT(num);
}

STATIC mp_obj_t TCP_client_read(mp_obj_t self, mp_obj_t buf_in, mp_obj_t size_in) {

    return TCP_client_read_data(TCP_client.client, buf_in,size_in);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(TCP_client_read_obj, TCP_client_read);

STATIC mp_obj_t TCP_client_flush(mp_obj_t self) {
	if(TCP_client.client == NULL) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The client does not exist!"));
	}

	if(TCP_client.socket_state == SOCKET_STATE_USED) {

	} else {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The client already unused!"));
	}

	TCPClient_flush(TCP_client.client);

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(TCP_client_flush_obj, TCP_client_flush);

STATIC mp_obj_t TCP_client_status(mp_obj_t self) {
	if(TCP_client.client == NULL) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The client does not exist!"));
	}

	if (TCPClient_status(TCP_client.client)) {
		return mp_const_true;
	} else {
		return mp_const_false;
	}

}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(TCP_client_status_obj, TCP_client_status);

STATIC mp_obj_t TCP_client_connected(mp_obj_t self) {
	if(TCP_client.client == NULL) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The client does not exist!"));
	}

	if(TCP_client.socket_state == SOCKET_STATE_USED) {

	} else {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The client already unused!"));
	}

	if (TCPClient_connected(TCP_client.client)) {
		return mp_const_true;
	} else {
		return mp_const_false;
	}
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(TCP_client_connected_obj, TCP_client_connected);

STATIC mp_obj_t TCP_client_peek(mp_obj_t self) {
	if(TCP_client.client == NULL) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The client does not exist!"));
	}

	if(TCP_client.socket_state == SOCKET_STATE_USED) {

	} else {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The client already unused!"));
	}

	return MP_OBJ_NEW_SMALL_INT(TCPClient_peek(TCP_client.client));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(TCP_client_peek_obj, TCP_client_peek);

STATIC const mp_map_elem_t TCP_client_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_connectByIP), (mp_obj_t)&TCP_client_connect_by_IP_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_connectByHost), (mp_obj_t)&TCP_client_connect_by_host_name_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_stop), (mp_obj_t)&TCP_client_stop_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_delete), (mp_obj_t)&delete_TCP_client_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_available), (mp_obj_t)&TCP_client_available_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&TCP_client_read_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_write), (mp_obj_t)&TCP_client_write_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_flush), (mp_obj_t)&TCP_client_flush_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_status), (mp_obj_t)&TCP_client_status_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_connected), (mp_obj_t)&TCP_client_connected_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_peek), (mp_obj_t)&TCP_client_peek_obj},
};

STATIC MP_DEFINE_CONST_DICT(TCP_client_locals_dict, TCP_client_locals_dict_table);

const mp_obj_type_t pyb_TCP_client_type = {
    { &mp_type_type },
    .name = MP_QSTR_TCP_client,
	.make_new = pyb_TCP_client_make_new,
    .locals_dict = (mp_obj_t)&TCP_client_locals_dict,
};


