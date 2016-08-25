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
#include <stdint.h>
#include <string.h>

#include "py/nlr.h"
#include "py/runtime.h"
#include "dac.h"
#include "pin.h"
#include "genhdr/pins.h"
#include "gpio_api.h"

/// \moduleref pyb
/// \class DAC - digital to analog conversion
///
/// The DAC is used to output analog values (a specific voltage) on pin X5 or pin X6.
/// The voltage will be between 0 and 3.3V.
///
/// *This module will undergo changes to the API.*
///
/// Example usage:
///
///     from pyb import DAC
///
///     dac = DAC(1)            # create DAC 1 on pin X5
///     dac.write(128)          # write a value to the DAC (makes X5 1.65V)
///
/// To output a continuous sine-wave:
///
///     import math
///     from pyb import DAC
///
///     # create a buffer containing a sine-wave
///     buf = bytearray(100)
///     for i in range(len(buf)):
///         buf[i] = 128 + int(127 * math.sin(2 * math.pi * i / len(buf)))
///
///     # output the sine-wave at 400Hz
///     dac = DAC(1)
///     dac.write_timed(buf, 400 * len(buf), mode=DAC.CIRCULAR)

#if defined(MICROPY_HW_ENABLE_DAC) && MICROPY_HW_ENABLE_DAC

/******************************************************************************/
// Micro Python bindings

typedef struct _pyb_dac_obj_t {
    mp_obj_base_t base;
    uint16_t      dac_pin;
}pyb_dac_obj_t;

STATIC mp_obj_t pyb_dac_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    if(n_args > 1)
    {
    	nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_TypeError, "argument num/types mismatch"));
    }

    pin_obj_t *pin = args[0];
	pyb_dac_obj_t *self = m_new0(pyb_dac_obj_t, 1);
    self->base.type = &pyb_dac_type;
    self->dac_pin = pin_mapping(pin);

    return self;
}

/// \method write(value)
/// Direct access to the DAC output (8 bit only at the moment).
STATIC mp_obj_t pyb_dac_write(mp_obj_t self_in, mp_obj_t val) {
	pyb_dac_obj_t *dac = self_in;
    uint16_t pin = dac->dac_pin;
    if(pin == A2 || pin == A3){
    	pinMode(pin, OUTPUT);
    	wiring_analogWrite(pin, mp_obj_get_int(val));
    } else {
    	printf("Error: Only pin A2 and A3 support DAC function\n");
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_dac_write_obj, pyb_dac_write);

STATIC const mp_map_elem_t pyb_dac_locals_dict_table[] = {
    // instance methods
    { MP_OBJ_NEW_QSTR(MP_QSTR_write), (mp_obj_t)&pyb_dac_write_obj },
};

STATIC MP_DEFINE_CONST_DICT(pyb_dac_locals_dict, pyb_dac_locals_dict_table);

const mp_obj_type_t pyb_dac_type = {
    { &mp_type_type },
    .name = MP_QSTR_DAC,
	.make_new = pyb_dac_make_new,
    .locals_dict = (mp_obj_t)&pyb_dac_locals_dict,
};

#endif // MICROPY_HW_ENABLE_DAC
