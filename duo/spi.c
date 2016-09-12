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
#include "spi.h"
#include "wiring.h"

typedef struct _pyb_spi_obj_t {
    mp_obj_base_t base;
    pyb_spi_t spi_id : 8;
}pyb_spi_obj_t;

/******************************************************************************/
/* Micro Python bindings                                                      */



/// \method init(mode, baudrate=328125, *, polarity=1, phase=0, bits=8, firstbit=SPI.MSB, ti=False, crc=None)
///
/// Initialise the SPI bus with the given parameters:
///
///   - `mode` must be either `SPI.MASTER` or `SPI.SLAVE`.
///   - `baudrate` is the SCK clock rate (only sensible for a master).
STATIC mp_obj_t pyb_spi_init_helper(const pyb_spi_obj_t *self, mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // init the SPI bus
	if(self->spi_id == 1) {
		spi_begin();

		printf("spi begin success!\n");
	} else if(self->spi_id == 2) {
		spi1_begin();
	} else {
		printf("%s\n","Error: The first parameter is only 1 or 2");
	}
    return mp_const_none;
}

/// \classmethod \constructor(bus, ...)
///
/// Construct an SPI object on the given bus.  `bus` can be 1 or 2.
/// With no additional parameters, the SPI object is created but not
/// initialised (it has the settings from the last initialisation of
/// the bus, if any).  If extra arguments are given, the bus is initialised.
/// See `init` for parameters of initialisation.
///
/// The physical pins of the SPI busses are:
///
/// At the moment, the NSS pin is not used by the SPI driver and is free
/// for other use.
STATIC mp_obj_t pyb_spi_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // work out SPI bus
    int spi_id = mp_obj_get_int(args[0]);
    if (spi_id < 1 || spi_id > 2) {
    	nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
    			"SPI(%d) does not exist", spi_id));
        }

	pyb_spi_obj_t *self = m_new0(pyb_spi_obj_t, 1);
    self->base.type = &pyb_spi_type;
    self->spi_id = spi_id;

    if (n_args > 0 || n_kw > 0) {
        // start the peripheral
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        pyb_spi_init_helper(self, n_args - 1, args + 1, &kw_args);
    }

    return self;
}

STATIC mp_obj_t pyb_spi_init(mp_uint_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return pyb_spi_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_spi_init_obj, 1, pyb_spi_init);

/// \method deinit()
/// Turn off the SPI bus.
STATIC mp_obj_t pyb_spi_deinit(mp_obj_t self_in) {
    pyb_spi_obj_t *self = self_in;

    if(self->spi_id == 1) {
    	spi_end();
    } else if(self->spi_id == 2) {
    	spi1_end();
    } else {
    	nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
    			"SPI(%d) does not exist", self->spi_id));
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_spi_deinit_obj, pyb_spi_deinit);

STATIC mp_obj_t pyb_spi_send_char(mp_obj_t self_in, mp_obj_t data) {
    // TODO assumes transmission size is 8-bits wide
    pyb_spi_obj_t *self = self_in;

    if(self->spi_id == 1) {
    	spi_transfer(mp_obj_get_int(data));
    } else if(self->spi_id == 2) {
    	spi1_transfer(mp_obj_get_int(data));
    } else {
    	nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
    			"SPI(%d) does not exist", self->spi_id));
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_spi_send_char_obj, pyb_spi_send_char);

STATIC mp_obj_t pyb_spi_recv_char(mp_obj_t self_in) {
    // TODO assumes transmission size is 8-bits wide
    pyb_spi_obj_t *self = self_in;

    if(self->spi_id == 1) {
    	return MP_OBJ_NEW_SMALL_INT(spi_transfer('\0'));
    } else if(self->spi_id == 2) {
    	return MP_OBJ_NEW_SMALL_INT(spi1_transfer('\0'));
    } else {
    	nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
    			"SPI(%d) does not exist", self->spi_id));
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_spi_recv_char_obj, pyb_spi_recv_char);

/// \method send(send, *, timeout=5000)
/// Send data on the bus:
///
///   - `send` is the data to send (an integer to send, or a buffer object).
///
/// Return value: `None`.
STATIC mp_obj_t pyb_spi_send(mp_obj_t self_in, mp_obj_t send_buffer) {
    // TODO assumes transmission size is 8-bits wide
    int i = 0;
	pyb_spi_obj_t *self = self_in;
    if(MP_OBJ_IS_STR(send_buffer)) {
    	const char *buf = mp_obj_str_get_str(send_buffer);

        if(self->spi_id == 1) {
        	spi_transferBytes((void *)buf, NULL, strlen(buf) + 1, NULL);
        } else if(self->spi_id == 2) {
        	spi1_transferBytes((void *)buf, NULL, strlen(buf) + 1, NULL);
        } else {
        	nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
        			"SPI(%d) does not exist", self->spi_id));
        }
    } else {
    	mp_obj_list_t *buffer = MP_OBJ_TO_PTR(send_buffer);
    	char *buf = NULL;
    	buf = (char *)malloc(buffer->len);

    	for(; i < buffer->len; i++) {
    		buf[i] = mp_obj_get_int(buffer->items[i]);
    	}

        if(self->spi_id == 1) {
        	spi_transferBytes(buf, NULL, buffer->len, NULL);
        } else if(self->spi_id == 2) {
        	spi1_transferBytes(buf, NULL, buffer->len, NULL);
        } else {
        	nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
        			"SPI(%d) does not exist", self->spi_id));
        }
        free(buf);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_spi_send_obj, pyb_spi_send);

/// \method recv(recv, *, timeout=5000)
///
/// Receive data on the bus:
///
///   - `recv` can be an integer, which is the number of bytes to receive,
///     or a mutable buffer, which will be filled with received bytes.
///   - `timeout` is the timeout in milliseconds to wait for the receive.
///
/// Return value: if `recv` is an integer then a new buffer of the bytes received,
/// otherwise the same buffer that was passed in to `recv`.
STATIC mp_obj_t pyb_spi_recv(mp_obj_t self_in, mp_obj_t recv_buffer, mp_obj_t length) {
    // TODO assumes transmission size is 8-bits wide
    int i = 0;
	pyb_spi_obj_t *self = self_in;
	uint8_t data = 0;

    if(self->spi_id == 1) {
    	for(; i < mp_obj_get_int(length); i++) {
    		data = spi_transfer('\0');
    		mp_obj_list_append(recv_buffer, MP_OBJ_NEW_SMALL_INT(data));

    		if(data == '\0') {
    			break;
    		}
    	}
    } else if(self->spi_id == 2) {
    	for(; i < mp_obj_get_int(length); i++) {
    		if((data = spi1_transfer('\0')) == '\0') {
    			break;
    		}

    		mp_obj_list_append(recv_buffer, MP_OBJ_NEW_SMALL_INT(data));
    	}
    } else {
    	nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
    			"SPI(%d) does not exist", self->spi_id));
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_spi_recv_obj, pyb_spi_recv);

STATIC mp_obj_t pyb_spi_set_clock_speed(mp_obj_t self_in, mp_obj_t value, mp_obj_t value_scale) {
    // TODO assumes transmission size is 8-bits wide
    pyb_spi_obj_t *self = self_in;

    if(self->spi_id == 1) {
    	spi_setClockSpeed(mp_obj_get_int(value), mp_obj_get_int(value_scale));
    } else if(self->spi_id == 2) {
    	spi1_setClockSpeed(mp_obj_get_int(value), mp_obj_get_int(value_scale));
    } else {
    	nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
    			"SPI(%d) does not exist", self->spi_id));
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_spi_set_clock_speed_obj, pyb_spi_set_clock_speed);

STATIC mp_obj_t pyb_spi_set_bit_order(mp_obj_t self_in, mp_obj_t bit_order) {
    // TODO assumes transmission size is 8-bits wide
    pyb_spi_obj_t *self = self_in;

    if(self->spi_id == 1) {
    	spi_setBitOrder(mp_obj_get_int(bit_order));
    } else if(self->spi_id == 2) {
    	spi1_setBitOrder(mp_obj_get_int(bit_order));
    } else {
    	nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
    			"SPI(%d) does not exist", self->spi_id));
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_spi_set_bit_order_obj, pyb_spi_set_bit_order);

STATIC mp_obj_t pyb_spi_set_clock_divider(mp_obj_t self_in, mp_obj_t divider) {
    // TODO assumes transmission size is 8-bits wide
    pyb_spi_obj_t *self = self_in;

    if(self->spi_id == 1) {
    	spi_setClockDivider(mp_obj_get_int(divider));
    } else if(self->spi_id == 2) {
    	spi1_setClockDivider(mp_obj_get_int(divider));
    } else {
    	nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
    			"SPI(%d) does not exist", self->spi_id));
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_spi_set_clock_divider_obj, pyb_spi_set_clock_divider);

STATIC mp_obj_t pyb_spi_set_data_mode(mp_obj_t self_in, mp_obj_t mode) {
    // TODO assumes transmission size is 8-bits wide
    pyb_spi_obj_t *self = self_in;

    if(self->spi_id == 1) {
    	spi_setDataMode(mp_obj_get_int(mode));
    } else if(self->spi_id == 2) {
    	spi1_setDataMode(mp_obj_get_int(mode));
    } else {
    	nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
    			"SPI(%d) does not exist", self->spi_id));
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_spi_set_data_mode_obj, pyb_spi_set_data_mode);

STATIC mp_obj_t pyb_spi_isenable(mp_obj_t self_in) {
    pyb_spi_obj_t *self = self_in;
    if(self->spi_id == 1) {
    	if (mp_obj_is_true(MP_OBJ_NEW_SMALL_INT(spi_isEnabled()))) {
    		return mp_const_true;
    	} else {
    		return mp_const_false;
    	}
    } else if(self->spi_id == 2) {
    	if (mp_obj_is_true(MP_OBJ_NEW_SMALL_INT(spi1_isEnabled()))) {
    		return mp_const_true;
    	} else {
    		return mp_const_false;
    	}
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_spi_isenable_obj, pyb_spi_isenable);

STATIC const mp_map_elem_t pyb_spi_locals_dict_table[] = {
    // instance methods
    { MP_OBJ_NEW_QSTR(MP_QSTR_init), 				(mp_obj_t)&pyb_spi_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_deInit), 				(mp_obj_t)&pyb_spi_deinit_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_sendChar), 			(mp_obj_t)&pyb_spi_send_char_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_recvChar), 			(mp_obj_t)&pyb_spi_recv_char_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_send), 				(mp_obj_t)&pyb_spi_send_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_recv), 				(mp_obj_t)&pyb_spi_recv_obj },

	{ MP_OBJ_NEW_QSTR(MP_QSTR_setClockSpeed), 		(mp_obj_t)&pyb_spi_set_clock_speed_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_setBitOrder), 		(mp_obj_t)&pyb_spi_set_bit_order_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_setClockDivider), 	(mp_obj_t)&pyb_spi_set_clock_divider_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_setDataMode), 		(mp_obj_t)&pyb_spi_set_data_mode_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_isEnable), 			(mp_obj_t)&pyb_spi_isenable_obj },

	// class constants
	{ MP_OBJ_NEW_QSTR(MP_QSTR_LSBFIRST),        	MP_OBJ_NEW_SMALL_INT(LSBFIRST) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_MSBFIRST),        	MP_OBJ_NEW_SMALL_INT(MSBFIRST) },

	{ MP_OBJ_NEW_QSTR(MP_QSTR_SPI_CLOCK_DIV2),      MP_OBJ_NEW_SMALL_INT(SPI_CLOCK_DIV2) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_SPI_CLOCK_DIV4),      MP_OBJ_NEW_SMALL_INT(SPI_CLOCK_DIV4) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_SPI_CLOCK_DIV8),      MP_OBJ_NEW_SMALL_INT(SPI_CLOCK_DIV8) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_SPI_CLOCK_DIV16),     MP_OBJ_NEW_SMALL_INT(SPI_CLOCK_DIV16) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_SPI_CLOCK_DIV32),     MP_OBJ_NEW_SMALL_INT(SPI_CLOCK_DIV32) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_SPI_CLOCK_DIV64),     MP_OBJ_NEW_SMALL_INT(SPI_CLOCK_DIV64) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_SPI_CLOCK_DIV128),    MP_OBJ_NEW_SMALL_INT(SPI_CLOCK_DIV128) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_SPI_CLOCK_DIV256),    MP_OBJ_NEW_SMALL_INT(SPI_CLOCK_DIV256) },

	{ MP_OBJ_NEW_QSTR(MP_QSTR_SPI_MODE0),       	MP_OBJ_NEW_SMALL_INT(SPI_MODE0) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_SPI_MODE1),       	MP_OBJ_NEW_SMALL_INT(SPI_MODE1) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_SPI_MODE2),        	MP_OBJ_NEW_SMALL_INT(SPI_MODE2) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_SPI_MODE3),        	MP_OBJ_NEW_SMALL_INT(SPI_MODE3) },
};

STATIC MP_DEFINE_CONST_DICT(pyb_spi_locals_dict, pyb_spi_locals_dict_table);

const mp_obj_type_t pyb_spi_type = {
    { &mp_type_type },
    .name = MP_QSTR_SPI,
    .make_new = pyb_spi_make_new,
    .locals_dict = (mp_obj_t)&pyb_spi_locals_dict,
};
