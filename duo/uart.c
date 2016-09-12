#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "py/nlr.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "uart.h"
#include "wiring.h"

//TODO: Add UART7/8 support for MCU_SERIES_F7

/// \moduleref pyb
/// \class UART - duplex serial communication bus
///

struct _pyb_uart_obj_t {
    mp_obj_base_t base;
    pyb_uart_t uart_id : 8;
};

STATIC mp_obj_t pyb_uart_deinit(mp_obj_t self_in);

void uart_init0(void) {
    for (int i = 0; i < MP_ARRAY_SIZE(MP_STATE_PORT(pyb_uart_obj_all)); i++) {
        MP_STATE_PORT(pyb_uart_obj_all)[i] = NULL;
    }
}

// unregister all interrupt sources
void uart_deinit(void) {
    for (int i = 0; i < MP_ARRAY_SIZE(MP_STATE_PORT(pyb_uart_obj_all)); i++) {
        pyb_uart_obj_t *uart_obj = MP_STATE_PORT(pyb_uart_obj_all)[i];
        if (uart_obj != NULL) {
            pyb_uart_deinit(uart_obj);
        }
    }
}

/******************************************************************************/
/* Micro Python bindings                                                      */

STATIC mp_obj_t pyb_uart_init_helper(pyb_uart_obj_t *self, mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
	if(self->uart_id == 1) {
		usartserial1_begin(mp_obj_get_int(pos_args[0]));
	} else if(self->uart_id == 2) {
		usartserial2_begin(mp_obj_get_int(pos_args[0]));
	} else {
		printf("%s\n","Error: The first parameter is only 1 or 2");
	}
    return mp_const_none;
}

/// \classmethod \constructor(bus, ...)
STATIC mp_obj_t pyb_uart_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // work out port
    int uart_id = 0;
    uart_id = mp_obj_get_int(args[0]);
    if (uart_id < 1 || uart_id > MP_ARRAY_SIZE(MP_STATE_PORT(pyb_uart_obj_all))) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "UART(%d) does not exist", uart_id));
    }

    pyb_uart_obj_t *self;
    if (MP_STATE_PORT(pyb_uart_obj_all)[uart_id - 1] == NULL) {
        // create new UART object
        self = m_new0(pyb_uart_obj_t, 1);
        self->base.type = &pyb_uart_type;
        self->uart_id = uart_id;
        MP_STATE_PORT(pyb_uart_obj_all)[uart_id - 1] = self;
    } else {
        // reference existing UART object
        self = MP_STATE_PORT(pyb_uart_obj_all)[uart_id - 1];
    }

    if (n_args > 1 || n_kw > 0) {
        // start the peripheral
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        pyb_uart_init_helper(self, n_args - 1, args + 1, &kw_args);
    }

    return self;
}

STATIC mp_obj_t pyb_uart_init(mp_uint_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return pyb_uart_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_uart_init_obj, 1, pyb_uart_init);

/// \method deinit()
/// Turn off the UART bus.
STATIC mp_obj_t pyb_uart_deinit(mp_obj_t self_in) {
    pyb_uart_obj_t *self = self_in;
    if(self->uart_id == 1) {
    	usartserial1_end();
    } else if(self->uart_id == 2) {
    	usartserial2_end();
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_uart_deinit_obj, pyb_uart_deinit);

/// \method any()
/// Return `True` if any characters waiting, else `False`.
STATIC mp_obj_t pyb_uart_any(mp_obj_t self_in) {
    pyb_uart_obj_t *self = self_in;
    if(self->uart_id == 1) {
    	return MP_OBJ_NEW_SMALL_INT(usartserial1_available());
    } else if(self->uart_id == 2) {
    	return MP_OBJ_NEW_SMALL_INT(usartserial2_available());
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_uart_any_obj, pyb_uart_any);

STATIC mp_obj_t pyb_uart_flush(mp_obj_t self_in) {
    pyb_uart_obj_t *self = self_in;
    if(self->uart_id == 1) {
    	usartserial1_flush();
    	printf("flush success!\n");
    } else if(self->uart_id == 2) {
    	usartserial2_flush();
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_uart_flush_obj, pyb_uart_flush);

STATIC mp_obj_t pyb_uart_peek(mp_obj_t self_in) {
    pyb_uart_obj_t *self = self_in;
    if(self->uart_id == 1) {
    	return MP_OBJ_NEW_SMALL_INT(usartserial1_peek());
    } else if(self->uart_id == 2) {
    	return MP_OBJ_NEW_SMALL_INT(usartserial2_peek());
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_uart_peek_obj, pyb_uart_peek);

STATIC mp_obj_t pyb_uart_isenable(mp_obj_t self_in) {
    pyb_uart_obj_t *self = self_in;
    if(self->uart_id == 1) {
    	if (mp_obj_is_true(MP_OBJ_NEW_SMALL_INT(usartserial1_isenable()))) {
    		return mp_const_true;
    	} else {
    		return mp_const_false;
    	}
    } else if(self->uart_id == 2) {
    	if (mp_obj_is_true(MP_OBJ_NEW_SMALL_INT(usartserial2_isenable()))) {
    		return mp_const_true;
    	} else {
    		return mp_const_false;
    	}
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_uart_isenable_obj, pyb_uart_isenable);

/// \method writechar(char)
/// Write a single character on the bus.  `char` is an integer to write.
/// Return value: `None`.
STATIC mp_obj_t pyb_uart_writechar(mp_obj_t self_in, mp_obj_t char_in) {
    pyb_uart_obj_t *self = self_in;

    // get the character to write (might be 9 bits)
    uint16_t data = mp_obj_get_int(char_in);

    // write the character
    if(self->uart_id == 1) {
    	usartserial1_putc((unsigned char)data);
     } else if(self->uart_id == 2) {
    	usartserial2_putc((unsigned char)data);
     }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_uart_writechar_obj, pyb_uart_writechar);

/// \method readchar()
/// Receive a single character on the bus.
/// Return value: The character read, as an integer.  Returns -1 on timeout.
STATIC mp_obj_t pyb_uart_readchar(mp_obj_t self_in) {
    pyb_uart_obj_t *self = self_in;
    if(self->uart_id == 1) {
    	return MP_OBJ_NEW_SMALL_INT(usartserial1_read());
     } else if(self->uart_id == 2) {
    	return MP_OBJ_NEW_SMALL_INT(usartserial2_read());
     }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_uart_readchar_obj, pyb_uart_readchar);

STATIC mp_obj_t pyb_uart_read(mp_obj_t self_in, mp_obj_t buf_in, mp_obj_t size) {
    int i = 0;
	pyb_uart_obj_t *self = self_in;
	unsigned char data = 0;

    // read the data
    if(self->uart_id == 1) {
    	for(; i < mp_obj_get_int(size); i++) {
    		data = usartserial1_read();
    		mp_obj_list_append(buf_in, MP_OBJ_NEW_SMALL_INT(data));
    		if(data == 0) {
    			break;
    		}
//    		printf("buf[%d] = %c\n", i, data);
    	}
    } else if (self->uart_id == 2) {
       	for(; i < mp_obj_get_int(size); i++) {
    		data = usartserial2_read();
    		mp_obj_list_append(buf_in, MP_OBJ_NEW_SMALL_INT(data));
    		if(data == 0) {
    			break;
    		}
        }
    }

    return MP_OBJ_NEW_SMALL_INT(i);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_uart_read_obj, pyb_uart_read);

STATIC mp_obj_t pyb_uart_write(mp_obj_t self_in, mp_obj_t buf_in) {
    int i = 0;
	pyb_uart_obj_t *self = self_in;
	const char* buf = mp_obj_str_get_str(buf_in);

    if(self->uart_id == 1) {
    	for(; i < strlen(buf); i++) {
    		usartserial1_putc(buf[i]);
    	}
    } else if (self->uart_id == 2) {
    	for(; i < strlen(buf); i++) {
        	usartserial2_putc(buf[i]);
        }
    }

    return MP_OBJ_NEW_SMALL_INT(i);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_uart_write_obj, pyb_uart_write);

STATIC const mp_map_elem_t pyb_uart_locals_dict_table[] = {
    // instance methods

    { MP_OBJ_NEW_QSTR(MP_QSTR_init), (mp_obj_t)&pyb_uart_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_deInit), (mp_obj_t)&pyb_uart_deinit_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_any), (mp_obj_t)&pyb_uart_any_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_flush), (mp_obj_t)&pyb_uart_flush_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_peek), (mp_obj_t)&pyb_uart_peek_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_isEnable), (mp_obj_t)&pyb_uart_isenable_obj },

    /// \method read([nbytes])
    { MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&pyb_uart_read_obj },
    /// \method write(buf)
    { MP_OBJ_NEW_QSTR(MP_QSTR_write), (mp_obj_t)&pyb_uart_write_obj },

    { MP_OBJ_NEW_QSTR(MP_QSTR_writeChar), (mp_obj_t)&pyb_uart_writechar_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_readChar), (mp_obj_t)&pyb_uart_readchar_obj },

	{ MP_OBJ_NEW_QSTR(MP_QSTR_BAUDRATE_9600), MP_OBJ_NEW_SMALL_INT(BAUDRATE_9600) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_BAUDRATE_115200), MP_OBJ_NEW_SMALL_INT(BAUDRATE_115200) },
};

STATIC MP_DEFINE_CONST_DICT(pyb_uart_locals_dict, pyb_uart_locals_dict_table);

const mp_obj_type_t pyb_uart_type = {
    { &mp_type_type },
    .name = MP_QSTR_UART,
    .make_new = pyb_uart_make_new,
    .getiter = mp_identity,
    .locals_dict = (mp_obj_t)&pyb_uart_locals_dict,
};
