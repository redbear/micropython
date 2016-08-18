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
#include <stddef.h>
#include <string.h>

#include "py/nlr.h"
#include "py/runtime.h"
#include "py/gc.h"
#include "py/mphal.h"
#include "pin.h"
#include "extint.h"
#include "wiring.h"
#include "py/objfun.h"
#include "interrupts.h"

/// \moduleref pyb
/// \class ExtInt - configure I/O pins to interrupt on external events
///
/// There are a total of 22 interrupt lines. 16 of these can come from GPIO pins
/// and the remaining 6 are from internal sources.
///
/// For lines 0 thru 15, a given line can map to the corresponding line from an
/// arbitrary port. So line 0 can map to Px0 where x is A, B, C, ... and
/// line 1 can map to Px1 where x is A, B, C, ...
///
///     def callback(line):
///         print("line =", line)
///
/// Note: ExtInt will automatically configure the gpio line as an input.
///
///     extint = pyb.ExtInt(pin, pyb.ExtInt.IRQ_FALLING, pyb.Pin.PULL_UP, callback)
///
/// Now every time a falling edge is seen on the X1 pin, the callback will be
/// called. Caution: mechanical pushbuttons have "bounce" and pushing or
/// releasing a switch will often generate multiple edges.
/// See: http://www.eng.utah.edu/~cs5780/debouncing.pdf for a detailed
/// explanation, along with various techniques for debouncing.
///
/// Trying to register 2 callbacks onto the same pin will throw an exception.
///
/// If pin is passed as an integer, then it is assumed to map to one of the
/// internal interrupt sources, and must be in the range 16 thru 22.
///
/// All other pin objects go through the pin mapper to come up with one of the
/// gpio pins.
///
///     extint = pyb.ExtInt(pin, mode, pull, callback)
///
/// Valid modes are pyb.ExtInt.IRQ_RISING, pyb.ExtInt.IRQ_FALLING,
/// pyb.ExtInt.IRQ_RISING_FALLING, pyb.ExtInt.EVT_RISING,
/// pyb.ExtInt.EVT_FALLING, and pyb.ExtInt.EVT_RISING_FALLING.
///
/// Only the IRQ_xxx modes have been tested. The EVT_xxx modes have
/// something to do with sleep mode and the WFE instruction.
///
/// Valid pull values are pyb.Pin.PULL_UP, pyb.Pin.PULL_DOWN, pyb.Pin.PULL_NONE.
///
/// There is also a C API, so that drivers which require EXTI interrupt lines
/// can also use this code. See extint.h for the available functions and
/// usrsw.h for an example of using this.

// TODO Add python method to change callback object.



// Macro used to set/clear the bit corresponding to the line in the IMR/EMR
// register in an atomic fashion by using bitband addressing.

typedef struct {
    mp_obj_base_t base;
    mp_int_t line;
} extint_obj_t;

uint32_t pyb_extint_callback[EXTI_NUM_VECTORS];

// Set override_callback_obj to true if you want to unconditionally set the

uint extint_register(mp_obj_t pin_obj, uint32_t mode, uint32_t pull, mp_obj_t callback_obj, bool override_callback_obj) {
    const pin_obj_t *pin = NULL;
    uint8_t v_line;


    pin = pin_find(pin_obj);
    v_line = pin->pin;

    mp_obj_t *cb = &MP_STATE_PORT(pyb_extint_callback)[v_line];

    if (!override_callback_obj && *cb != mp_const_none && callback_obj != mp_const_none) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "ExtInt vector %d is already in use", v_line));
    }

    Interrupt_disableAllInterrupts();
    *cb = callback_obj;
    pinMode(pin_mapping(pin), pull);
    Interrupt_attachInterrupt(pin_mapping(pin), get_exti_isr(v_line), mode);
    Interrupt_enableAllInterrupts();
    return v_line;
}


STATIC mp_obj_t extint_obj_detach_interrupt(mp_obj_t self_in) {
    pin_obj_t *self = self_in;
    Interrupt_detachInterrupt(pin_mapping(self));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(extint_obj_detach_interrupt_obj, extint_obj_detach_interrupt);

STATIC mp_obj_t extint_obj_enable_all_interrupt() {

	Interrupt_enableAllInterrupts();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(extint_obj_enable_all_interrupt_obj, extint_obj_enable_all_interrupt);

STATIC mp_obj_t extint_obj_disable_all_interrupt() {

	Interrupt_disableAllInterrupts();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(extint_obj_disable_all_interrupt_obj, extint_obj_disable_all_interrupt);

/// \classmethod \constructor(pin, mode, pull, callback)
/// Create an ExtInt object:
///
///   - `pin` is the pin on which to enable the interrupt (can be a pin object or any valid pin name).
///   - `mode` can be one of:
///     - `ExtInt.IRQ_RISING` - trigger on a rising edge;
///     - `ExtInt.IRQ_FALLING` - trigger on a falling edge;
///     - `ExtInt.IRQ_RISING_FALLING` - trigger on a rising or falling edge.
///   - `pull` can be one of:
///     - `pyb.Pin.PULL_NONE` - no pull up or down resistors;
///     - `pyb.Pin.PULL_UP` - enable the pull-up resistor;
///     - `pyb.Pin.PULL_DOWN` - enable the pull-down resistor.
///   - `callback` is the function to call when the interrupt triggers.  The
///   callback function must accept exactly 1 argument, which is the line that
///   triggered the interrupt.

STATIC const mp_arg_t pyb_extint_make_new_args[] = {
    { MP_QSTR_pin,      MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    { MP_QSTR_mode,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    { MP_QSTR_pull,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    { MP_QSTR_callback, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
};
#define PYB_EXTINT_MAKE_NEW_NUM_ARGS MP_ARRAY_SIZE(pyb_extint_make_new_args)

STATIC mp_obj_t extint_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    // type_in == extint_obj_type

    // parse args
    mp_arg_val_t vals[PYB_EXTINT_MAKE_NEW_NUM_ARGS];
    mp_arg_parse_all_kw_array(n_args, n_kw, args, PYB_EXTINT_MAKE_NEW_NUM_ARGS, pyb_extint_make_new_args, vals);

    extint_obj_t *self = m_new_obj(extint_obj_t);
    self->base.type = type;
    self->line = extint_register(vals[0].u_obj, vals[1].u_int, vals[2].u_int, vals[3].u_obj, false);

    return self;
}

STATIC const mp_map_elem_t extint_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_detach_interrupt),    (mp_obj_t)&extint_obj_detach_interrupt_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_enable_all_interrupt),  (mp_obj_t)&extint_obj_enable_all_interrupt_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_disable_all_interrupt), (mp_obj_t)&extint_obj_disable_all_interrupt_obj },
};

STATIC MP_DEFINE_CONST_DICT(extint_locals_dict, extint_locals_dict_table);

const mp_obj_type_t pyb_extint_type = {
    { &mp_type_type },
    .name = MP_QSTR_ExtInt,
    .make_new = extint_make_new,
    .locals_dict = (mp_obj_t)&extint_locals_dict,
};

void extint_init0(void) {
    for (int i = 0; i < PYB_EXTI_NUM_VECTORS; i++) {
        MP_STATE_PORT(pyb_extint_callback)[i] = mp_const_none;
   }
}

// Interrupt handler
void Handle_EXTI_Irq(uint32_t line) {
    if (line < EXTI_NUM_VECTORS) {
        mp_obj_t *cb = &MP_STATE_PORT(pyb_extint_callback)[line];
        if (*cb != mp_const_none) {
            // When executing code within a handler we must lock the GC to prevent
            // any memory allocations.  We must also catch any exceptions.
            gc_lock();
            nlr_buf_t nlr;
            if (nlr_push(&nlr) == 0) {
                mp_call_function_0(*cb);
                nlr_pop();
            } else {
                // Uncaught exception; disable the callback so it doesn't run again.
                *cb = mp_const_none;
                printf("Uncaught exception in ExtInt interrupt handler line %lu\n", line);
                mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
            }
            gc_unlock();
        }
    }
}

