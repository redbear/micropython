/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "py/nlr.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "pin.h"
#include "genhdr/pins.h"
#include "i2c.h"
#include "wiring.h"

typedef struct _pyb_i2c_obj_t {
    mp_obj_base_t base;
}pyb_i2c_obj_t;

void transfer_status(uint8_t num) {
	switch(num) {
	case 0 :
		printf("Success!\n");
		break;
	case 1 :
		printf("Error: Transfer data is too long, a buffer overflow!\n");
		break;
	case 2 :
		printf("Error: Send the address to receive NACK!\n");
		break;
	case 3 :
		printf("Error: Send the data to receive NACK!\n");
		break;
	case 4 :
		printf("Other error!\n");
		break;
	default :
		break;
	}

	return;
}

/******************************************************************************/
/* Micro Python bindings                                                      */

/// \classmethod \constructor(bus, ...)
STATIC mp_obj_t pyb_i2c_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {

    // work out i2c bus
	pyb_i2c_obj_t *self = m_new0(pyb_i2c_obj_t, 1);
    self->base.type = &pyb_i2c_type;

    return self;
}


STATIC mp_obj_t pyb_i2c_init(mp_uint_t n_args, const mp_obj_t *args) {
	if(2 == n_args)
	{
		i2c_setSpeed(mp_obj_get_int(args[1]));
	}
	else
	{
		i2c_setSpeed(CLOCK_SPEED_100KHZ);
	}
	i2c_begin();

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_i2c_init_obj, 1, 2, pyb_i2c_init);

/// \method deinit()
/// Turn off the I2C bus.
STATIC mp_obj_t pyb_i2c_deinit(mp_obj_t self_in) {
    i2c_end();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_i2c_deinit_obj, pyb_i2c_deinit);

STATIC mp_obj_t pyb_i2c_send_char(mp_obj_t self_in, mp_obj_t data, mp_obj_t addr) {

    i2c_beginTransmission((uint8_t)mp_obj_get_int(addr));
    i2c_writeOneByte((uint8_t)(mp_obj_get_int(data)));
    i2c_beginTransmission((uint8_t)mp_obj_get_int(addr));

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_i2c_send_char_obj, pyb_i2c_send_char);

STATIC mp_obj_t pyb_i2c_recv_char(mp_obj_t self_in, mp_obj_t addr) {

    i2c_requestFrom((uint8_t)(mp_obj_get_int(addr)), 1, 1);
    return MP_OBJ_NEW_SMALL_INT(i2c_read());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_i2c_recv_char_obj, pyb_i2c_recv_char);

/// method send(send, addr=0x00, timeout=5000)
STATIC mp_obj_t pyb_i2c_send(mp_obj_t self_in, mp_obj_t send_buffer, mp_obj_t addr) {

	i2c_beginTransmission((uint8_t)mp_obj_get_int(addr));

    if(MP_OBJ_IS_STR(send_buffer)) {
    	const char *buf = mp_obj_str_get_str(send_buffer);

        i2c_writeBytes((uint8_t *)buf, strlen(buf));
        transfer_status(i2c_endTransmission(1));

        return MP_OBJ_NEW_SMALL_INT(strlen(buf));

    } else {
    	int i = 0;
    	mp_obj_list_t *buffer = MP_OBJ_TO_PTR(send_buffer);
    	uint8_t buf[buffer->len];

    	for(; i < buffer->len; i++) {
    		buf[i] = mp_obj_get_int(buffer->items[i]);
    	}

    	i2c_writeBytes(buf, i);
    	transfer_status(i2c_endTransmission(1));

    	return MP_OBJ_NEW_SMALL_INT(i);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_i2c_send_obj, pyb_i2c_send);

/// \method recv(recv, addr=0x00, timeout=5000)
///
/// Receive data on the bus:
///
///   - `recv` can be an integer, which is the number of bytes to receive,
///     or a mutable buffer, which will be filled with received bytes
///   - `addr` is the address to receive from (only required in master mode)
///   - `timeout` is the timeout in milliseconds to wait for the receive
///
/// Return value: if `recv` is an integer then a new buffer of the bytes received,
/// otherwise the same buffer that was passed in to `recv`.
STATIC mp_obj_t pyb_i2c_recv(mp_uint_t n_args, const mp_obj_t *args) {
    int i = 0;
	char data = 0;
	i2c_requestFrom(((uint8_t)mp_obj_get_int(args[2])), (uint8_t)(mp_obj_get_int(args[3])), 1);

    for(; i < mp_obj_get_int(args[3]); i++) {
    	data = (char)i2c_read();
    	mp_obj_list_append(args[1], MP_OBJ_NEW_SMALL_INT(data));

    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_i2c_recv_obj, 4, 4, pyb_i2c_recv);

STATIC mp_obj_t pyb_i2c_isenable(mp_obj_t self_in) {
    if (mp_obj_is_true(MP_OBJ_NEW_SMALL_INT(i2c_isEnabled()))) {
   		return mp_const_true;
   	} else {
   		return mp_const_false;
   	}

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_i2c_isenable_obj, pyb_i2c_isenable);

STATIC const mp_map_elem_t pyb_i2c_locals_dict_table[] = {
    // instance methods
    { MP_OBJ_NEW_QSTR(MP_QSTR_init), (mp_obj_t)&pyb_i2c_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_deInit), (mp_obj_t)&pyb_i2c_deinit_obj },

    { MP_OBJ_NEW_QSTR(MP_QSTR_sendChar), (mp_obj_t)&pyb_i2c_send_char_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_recvChar), (mp_obj_t)&pyb_i2c_recv_char_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_send), (mp_obj_t)&pyb_i2c_send_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_recv), (mp_obj_t)&pyb_i2c_recv_obj },

	{ MP_OBJ_NEW_QSTR(MP_QSTR_isEnable), (mp_obj_t)&pyb_i2c_isenable_obj },

	{ MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK_SPEED_100KHZ), MP_OBJ_NEW_SMALL_INT(CLOCK_SPEED_100KHZ) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK_SPEED_400KHZ), MP_OBJ_NEW_SMALL_INT(CLOCK_SPEED_400KHZ) },
};

STATIC MP_DEFINE_CONST_DICT(pyb_i2c_locals_dict, pyb_i2c_locals_dict_table);

const mp_obj_type_t pyb_i2c_type = {
    { &mp_type_type },
    .name = MP_QSTR_I2C,
    .make_new = pyb_i2c_make_new,
    .locals_dict = (mp_obj_t)&pyb_i2c_locals_dict,
};
