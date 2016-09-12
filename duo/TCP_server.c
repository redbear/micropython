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
#include "TCP_server.h"
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

typedef struct _pyb_TCP_server_obj_t {
    mp_obj_base_t base;
    pyb_TCP_server_t server_id : 8;
}pyb_TCP_server_obj_t;

static tcp_server_t servers[MAX_SERVER_SOCKETS];
static tcp_client_t clients[MAX_CLIENT_SOCKETS];

static uint32_t server_id = 0;
static uint32_t client_id = 0;

STATIC mp_obj_t pyb_TCP_server_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    if(n_args < 0 || n_args > 1)
    	nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_TypeError, "argument num/types mismatch."));

    server_id++;
    if (server_id < 1 || server_id > 3) {
    	nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
    			"Up to create three servers"));
        }

    servers[server_id].socket_id = server_id;
    servers[server_id].server = (tcp_server *)(TCPServer_newTCPServer(mp_obj_get_int(args[1])));
    servers[server_id].socket_state = SOCKET_STATE_UNUSED;

	pyb_TCP_server_obj_t *self = m_new0(pyb_TCP_server_obj_t, 1);
    self->base.type = &pyb_TCP_server_type;
    self->server_id = server_id;

    return self;
}

void TCP_server_init0(void) {
  uint8_t i;
  for(i=0; i<MAX_SERVER_SOCKETS; i++) {
    servers[i].server = NULL;
    servers[i].socket_state = SOCKET_STATE_UNUSED;
  }

  for(i=0; i<MAX_CLIENT_SOCKETS; i++) {
    clients[i].client = NULL;
    clients[i].socket_state = SOCKET_STATE_UNUSED;
  }

  return;
}

STATIC mp_obj_t TCP_server_begin(mp_obj_t self) {
	pyb_TCP_server_obj_t *server = self;
	uint32_t server_num = server->server_id;
	if(server_num < MAX_SERVER_SOCKETS ) {

	} else {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "Parameter range is 0 to 2£¡"));
	}

	if(servers[server_num].server == NULL) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The server %d does not exist!", server_num));
	}

	if(servers[server_num].socket_state == SOCKET_STATE_USED) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The server %d already used!", server_num));
	}

	tcp_server *TCP_server = servers[server_num].server;

	if (TCPServer_begin(TCP_server)) {
		servers[server_num].socket_state = SOCKET_STATE_USED;
		return mp_const_true;
	} else {
		return mp_const_false;
	}
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(TCP_server_begin_obj, TCP_server_begin);

STATIC mp_obj_t TCP_server_stop(mp_obj_t self) {
	pyb_TCP_server_obj_t *server = self;
	uint32_t server_num = server->server_id;
	if(server_num < MAX_SERVER_SOCKETS ) {

	} else {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "Parameter range is 0 to 2£¡"));
	}

	if(servers[server_num].server == NULL) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The server %d does not exist!", server_num));
	}

	if(servers[server_num].socket_state == SOCKET_STATE_USED) {

	} else {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The server %d already unused!", server_num));
	}

	tcp_server *TCP_server = servers[server_num].server;
	TCPServer_stop(TCP_server);

	servers[server_num].socket_state = SOCKET_STATE_UNUSED;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(TCP_server_stop_obj, TCP_server_stop);

STATIC mp_obj_t delete_TCP_server(mp_obj_t self) {
	pyb_TCP_server_obj_t *server = self;
	uint32_t server_num = server->server_id;
	if(server_num < MAX_SERVER_SOCKETS ) {

	} else {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "Parameter range is 0 to 2£¡"));
	}

	if(servers[server_num].server == NULL) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The server %d does not exist!", server_num));
	}

	TCPServer_deleteTCPServer(servers[server_num].server);
	servers[server_num].server = NULL;
	servers[server_num].socket_state = SOCKET_STATE_UNUSED;

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(delete_TCP_server_obj, delete_TCP_server);

STATIC mp_obj_t TCP_server_accept(mp_obj_t self) {
	pyb_TCP_server_obj_t *server = self;
	uint32_t server_num = server->server_id;
	if(server_num < MAX_SERVER_SOCKETS ) {

	} else {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "Parameter range is 0 to 2£¡"));
	}

	if(servers[server_num].server == NULL) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The server %d does not exist!", server_num));
	}

	if(servers[server_num].socket_state == SOCKET_STATE_USED) {

	} else {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The server %d already unused!", server_num));
	}

	if(client_id < MAX_CLIENT_SOCKETS) {
		clients[client_id].client = TCPServer_available(servers[server_num].server);
		if(NULL == clients[client_id].client){
			return MP_OBJ_NEW_SMALL_INT(-1);
		}
		clients[client_id].socket_id = client_id;
		clients[client_id].socket_state == SOCKET_STATE_USED;
	} else {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "The biggest support 10 clients!"));
	}

	return MP_OBJ_NEW_SMALL_INT(clients[client_id++].socket_id);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(TCP_server_accept_obj, TCP_server_accept);

STATIC mp_obj_t TCP_server_available(mp_obj_t self, mp_obj_t client_id_in) {
	uint32_t client_id = mp_obj_get_int(client_id_in);
	int size = 0;
	if(client_id < MAX_CLIENT_SOCKETS ) {

	} else {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "Parameter range is 0 to 9£¡"));
	}

	return MP_OBJ_NEW_SMALL_INT(TCPClient_available(clients[client_id].client));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(TCP_server_available_obj, TCP_server_available);

STATIC mp_obj_t TCP_server_write_data(mp_obj_t self, mp_obj_t client_id_in, mp_obj_t buf_in) {
    uint32_t client_id = mp_obj_get_int(client_id_in);
	int size = 0;
	if(client_id < MAX_CLIENT_SOCKETS ) {

	} else {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "Parameter range is 0 to 9£¡"));
	}

	 return TCP_client_write_data(clients[client_id].client, buf_in);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(TCP_server_write_data_obj, TCP_server_write_data);

STATIC mp_obj_t TCP_server_read_data(mp_uint_t n_args, const mp_obj_t *args) {
    uint32_t client_id = mp_obj_get_int(args[1]);
	int size = 0;
	if(client_id < MAX_CLIENT_SOCKETS ) {

	} else {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_IndentationError, "Parameter range is 0 to 9£¡"));
	}

	 return TCP_client_read_data(clients[client_id].client, args[2], args[3]);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(TCP_server_read_data_obj, 4, 4, TCP_server_read_data);

STATIC const mp_map_elem_t TCP_server_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_begin), (mp_obj_t)&TCP_server_begin_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_stop), (mp_obj_t)&TCP_server_stop_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_delete), (mp_obj_t)&delete_TCP_server_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_accept), (mp_obj_t)&TCP_server_accept_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_available), (mp_obj_t)&TCP_server_available_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_write), (mp_obj_t)&TCP_server_write_data_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&TCP_server_read_data_obj},
};

STATIC MP_DEFINE_CONST_DICT(TCP_server_locals_dict, TCP_server_locals_dict_table);

const mp_obj_type_t pyb_TCP_server_type = {
    { &mp_type_type },
    .name = MP_QSTR_TCP_server,
	.make_new = pyb_TCP_server_make_new,
    .locals_dict = (mp_obj_t)&TCP_server_locals_dict,
};
