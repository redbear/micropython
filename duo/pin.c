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
#include "py/mphal.h"
#include "pin.h"
#include "wiring.h"

/// \moduleref pyb
/// \class Pin - control I/O pins
///

STATIC const uint16_t board_pin_mapping_tableA[] = {A7, A6, A1, A0, A2, A3, A4, A5, 0, TX, RX, 0, 0, D7, D6, D5};
STATIC const uint16_t board_pin_mapping_tableB[] = {0, 0, 0, D4, D3, D2, D1, D0};
STATIC const uint16_t no_pwm_pin_list[] = {A2, A3, D5, D6, D7};
STATIC const uint16_t analog_pin_table[] = {A0, A1, A2, A3, A4, A5, A6};

const uint16_t pin_mapping(const pin_obj_t *self){
	if(self->gpio == GPIOA){
		return board_pin_mapping_tableA[self->pin];
	} else {
		return board_pin_mapping_tableB[self->pin];
	}
}

STATIC mp_obj_t pin_obj_init_helper(const pin_obj_t *self, mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // get io mode
    uint mode = mp_obj_get_int(pos_args[0]);
    if (mode > 7) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "invalid pin mode"));
    }

    pinMode(pin_mapping(self), mode);
    return mp_const_none;
}

/// \classmethod \constructor(id, ...)
/// Create a new Pin object associated with the id.  If additional arguments are given,
/// they are used to initialise the pin.  See `init`.
STATIC mp_obj_t pin_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    const pin_obj_t *pin = NULL;

    if(n_args > 2)
    {
    	nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_TypeError, "argument num/types mismatch"));
    }

    // Run an argument through the mapper and return the result.
    if(MP_OBJ_IS_STR(args[0]))
    {
    	pin = pin_find_named_pin(&pin_board_pins_locals_dict, args[0]);
    } else {
    	pin = (pin_obj_t *)args[0];
    }

    pinMode(pin_mapping(pin), mp_obj_get_int(args[1]));

    if (n_args > 1 || n_kw > 0) {
        // pin mode given, so configure this GPIO
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        pin_obj_init_helper(pin, n_args - 1, args + 1, &kw_args);
    }

    return (mp_obj_t)pin;
}

/// \classmethod af_list()
/// Returns an array of alternate functions available for this pin.
STATIC mp_obj_t pin_afList(mp_obj_t self_in) {
    pin_obj_t *self = self_in;
    mp_obj_t result = mp_obj_new_list(0, NULL);

    const pin_af_obj_t *af = self->af;
    for (mp_uint_t i = 0; i < self->num_af; i++, af++) {
        mp_obj_list_append(result, (mp_obj_t)af);
    }
    return result;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pin_afList_obj, pin_afList);

STATIC mp_obj_t pin_obj_init(mp_uint_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return pin_obj_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
MP_DEFINE_CONST_FUN_OBJ_KW(pin_init_obj, 1, pin_obj_init);

/// \method value([value])
/// Get or set the digital logic level of the pin:
///
///   - With no argument, return 0 or 1 depending on the logic level of the pin.
///   - With `value` given, set the logic level of the pin.  `value` can be
///   anything that converts to a boolean.  If it converts to `True`, the pin
///   is set high, otherwise it is set low.
STATIC mp_obj_t pin_value(mp_obj_t self_in, mp_obj_t value) {
    pin_obj_t *self = self_in;
	const uint16_t pin = pin_mapping(self);
        // set pin
    if (mp_obj_is_true(value)) {
    	pinMode(pin, OUTPUT);
        digitalWrite(pin, 1);
    } else {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, 0);
    }
    return mp_const_none;

}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pin_value_obj, pin_value);

STATIC mp_obj_t pin_digitalRead(mp_obj_t self_in) {
	pin_obj_t *self = self_in;
	uint16_t pin = pin_mapping(self);
	pinMode(pin, INPUT_PULLUP);
    return MP_OBJ_NEW_SMALL_INT(digitalRead(pin));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pin_digitalRead_obj, pin_digitalRead);

STATIC mp_obj_t pin_pwmWrite(mp_uint_t n_args, const mp_obj_t *args) {
    int i = 0;
	pin_obj_t *self = args[0];
    uint16_t pin = pin_mapping(self);
    for(; i < 5; i++){
    	if(pin == no_pwm_pin_list[i]){
    		printf("Error: The pin does not support PWM function\n");
    		return mp_const_none;
    	}
    }
    pinMode(pin, OUTPUT);
    if(3 == n_args)
    {
    	wiring_analogWriteWithFreq(pin, mp_obj_get_int(args[1]), mp_obj_get_int(args[2]));
    }
    else if(2 == n_args)
    {
    	wiring_analogWrite(pin, mp_obj_get_int(args[1]));
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pin_pwmWrite_obj,2 ,3, pin_pwmWrite);

STATIC mp_obj_t pin_tone(mp_obj_t self_in, mp_obj_t freq, mp_obj_t duration) {
	pin_obj_t *self = self_in;
    uint16_t pin = pin_mapping(self);

    tone(pin, mp_obj_get_int(freq), mp_obj_get_int(duration));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pin_tone_obj, pin_tone);

STATIC mp_obj_t pin_noTone(mp_obj_t self_in) {
	pin_obj_t *self = self_in;
    uint16_t pin = pin_mapping(self);

    noTone(pin);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pin_noTone_obj, pin_noTone);

STATIC mp_obj_t pin_analogWrite(mp_obj_t self_in, mp_obj_t val) {
	pin_obj_t *self = self_in;
	uint16_t pin = pin_mapping(self);
    if(pin == A2 || pin == A3){
    	pinMode(pin, OUTPUT);
    	wiring_analogWrite(pin, mp_obj_get_int(val));
    } else {
    	printf("Error: Only pin A2 and A3 support DAC function\n");
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pin_analogWrite_obj, pin_analogWrite);

STATIC mp_obj_t pin_analogRead(mp_obj_t self_in) {
	int i = 0;
	pin_obj_t *self = self_in;
	uint16_t pin = pin_mapping(self);
    for(i = 0; i < 7; i++){
    	if(analog_pin_table[i] == pin){
    	    uint32_t data = analogRead(pin);
    	    return mp_obj_new_int(data);
    	}
    }
    return mp_obj_new_int(-1);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pin_analogRead_obj, pin_analogRead);

STATIC mp_obj_t pin_shiftIn(mp_obj_t data_in, mp_obj_t clock, mp_obj_t bit_order) {
    pin_obj_t *self_data = data_in;
    pin_obj_t *self_clock = clock;
    uint8_t data_pin = pin_mapping(self_data);
    uint8_t clock_pin = pin_mapping(self_clock);
    pinMode(data_pin, INPUT);
    pinMode(clock_pin, OUTPUT);

    return MP_OBJ_NEW_SMALL_INT(shiftIn(data_pin, clock_pin, mp_obj_get_int(bit_order)));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pin_shiftIn_obj, pin_shiftIn);

STATIC mp_obj_t pin_shiftOut(mp_uint_t n_args, const mp_obj_t *args) {
    pin_obj_t *self_data = args[0];
    pin_obj_t *self_clock = args[1];
    uint8_t data_pin = pin_mapping(self_data);
    uint8_t clock_pin = pin_mapping(self_clock);
    uint8_t  bit_order = mp_obj_get_int(args[2]);
    uint8_t  value = mp_obj_get_int(args[3]);

    pinMode(data_pin, OUTPUT);
    pinMode(clock_pin, OUTPUT);

    shiftOut(data_pin, clock_pin, bit_order, value);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pin_shiftOut_obj, 4, 4, pin_shiftOut);

STATIC mp_obj_t pin_pulseIn(mp_obj_t self_in, const mp_obj_t value) {
    pin_obj_t *self_data = self_in;
    uint8_t data_pin = pin_mapping(self_data);
    pinMode(data_pin, INPUT);
    uint32_t duration = pulseIn(data_pin, mp_obj_get_int(value));

    return MP_OBJ_NEW_SMALL_INT(duration);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pin_pulseIn_obj, pin_pulseIn);

/// \method name()
/// Get the pin name.
STATIC mp_obj_t pin_name(mp_obj_t self_in) {
    pin_obj_t *self = self_in;
    return MP_OBJ_NEW_QSTR(self->name);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pin_name_obj, pin_name);

/// \method names()
/// Returns the cpu and board names for this pin.
STATIC mp_obj_t pin_names(mp_obj_t self_in) {
    pin_obj_t *self = self_in;
    mp_obj_t result = mp_obj_new_list(0, NULL);
    mp_obj_list_append(result, MP_OBJ_NEW_QSTR(self->name));

    mp_map_t *map = mp_obj_dict_get_map((mp_obj_t)&pin_board_pins_locals_dict);
    mp_map_elem_t *elem = map->table;

    for (mp_uint_t i = 0; i < map->used; i++, elem++) {
        if (elem->value == self) {
            mp_obj_list_append(result, elem->key);
        }
    }
    return result;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pin_names_obj, pin_names);

/// \method port()
/// Get the pin port.
STATIC mp_obj_t pin_port(mp_obj_t self_in) {
    pin_obj_t *self = self_in;
    return MP_OBJ_NEW_SMALL_INT(self->port);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pin_port_obj, pin_port);

/// \method pin()
/// Get the pin number.
STATIC mp_obj_t pin_pin(mp_obj_t self_in) {
    pin_obj_t *self = self_in;
    return MP_OBJ_NEW_SMALL_INT(self->pin);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pin_pin_obj, pin_pin);

/// \method mode()
/// Returns the currently configured mode of the pin. The integer returned
/// will match one of the allowed constants for the mode argument to the init
/// function.
STATIC mp_obj_t pin_mode(mp_obj_t self_in) {
	pin_obj_t *self = self_in;
	uint16_t pin = pin_mapping(self);

	switch(getPinMode(pin))
	{
		case 0 :
			return MP_OBJ_NEW_QSTR(qstr_from_str("INPUT"));
			break;
		case 1 :
			return MP_OBJ_NEW_QSTR(qstr_from_str("OUTPUT"));
			break;
		case 2 :
			return MP_OBJ_NEW_QSTR(qstr_from_str("INPUT_PU"));
			break;
		case 3 :
			return MP_OBJ_NEW_QSTR(qstr_from_str("INPUT_PD"));
			break;
		case 4 :
			return MP_OBJ_NEW_QSTR(qstr_from_str("AF_OUTPUT_PP"));
			break;
		case 5 :
			return MP_OBJ_NEW_QSTR(qstr_from_str("AF_OUTPUT_OD"));
			break;
		case 6 :
			return MP_OBJ_NEW_QSTR(qstr_from_str("AN_INPUT"));
			break;
		case 7 :
			return MP_OBJ_NEW_QSTR(qstr_from_str("AN_OUTPUT"));
			break;
		default :
			return MP_OBJ_NEW_QSTR(qstr_from_str("PIN_MODE_NONE"));
			break;

	}
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pin_mode_obj, pin_mode);

STATIC void pin_named_pins_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    pin_named_pins_obj_t *self = self_in;
    mp_printf(print, "<Pin.%q>", self->name);
}

const pin_obj_t *pin_find_named_pin(const mp_obj_dict_t *named_pins, mp_obj_t name) {
    mp_map_t *named_map = mp_obj_dict_get_map((mp_obj_t)named_pins);
    mp_map_elem_t *named_elem = mp_map_lookup(named_map, name, MP_MAP_LOOKUP);
    if (named_elem != NULL && named_elem->value != NULL) {
        return named_elem->value;
    }
    return NULL;
}

const pin_af_obj_t *pin_find_af_by_index(const pin_obj_t *pin, mp_uint_t af_idx) {
    const pin_af_obj_t *af = pin->af;
    for (mp_uint_t i = 0; i < pin->num_af; i++, af++) {
        if (af->idx == af_idx) {
            return af;
        }
    }
    return NULL;
}

const mp_obj_type_t pin_cpu_pins_obj_type = {
    { &mp_type_type },
    .name = MP_QSTR_cpu,
    .print = pin_named_pins_obj_print,
    .locals_dict = (mp_obj_t)&pin_cpu_pins_locals_dict,
};

const mp_obj_type_t pin_board_pins_obj_type = {
    { &mp_type_type },
    .name = MP_QSTR_board,
    .print = pin_named_pins_obj_print,
    .locals_dict = (mp_obj_t)&pin_board_pins_locals_dict,
};

STATIC const mp_map_elem_t pin_locals_dict_table[] = {
    // instance methods
    { MP_OBJ_NEW_QSTR(MP_QSTR_pinMode),       	 (mp_obj_t)&pin_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_digitalWrite),     (mp_obj_t)&pin_value_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_digitalRead),      (mp_obj_t)&pin_digitalRead_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_pwmWrite),     	 (mp_obj_t)&pin_pwmWrite_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_tone),     	 	 (mp_obj_t)&pin_tone_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_noTone),     	 	 (mp_obj_t)&pin_noTone_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_analogWrite),      (mp_obj_t)&pin_analogWrite_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_analogRead),       (mp_obj_t)&pin_analogRead_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_name),       		 (mp_obj_t)&pin_name_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_names),      		 (mp_obj_t)&pin_names_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_afList),    		 (mp_obj_t)&pin_afList_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_port),       		 (mp_obj_t)&pin_port_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin),        		 (mp_obj_t)&pin_pin_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_mode),       		 (mp_obj_t)&pin_mode_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_shiftIn),   		 (mp_obj_t)&pin_shiftIn_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_shiftOut),  		 (mp_obj_t)&pin_shiftOut_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_pulseIn),   		 (mp_obj_t)&pin_pulseIn_obj },

    // class attributes
    { MP_OBJ_NEW_QSTR(MP_QSTR_board),   (mp_obj_t)&pin_board_pins_obj_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_cpu),     (mp_obj_t)&pin_cpu_pins_obj_type },

    // class constants
	{ MP_OBJ_NEW_QSTR(MP_QSTR_HIGH),        		MP_OBJ_NEW_SMALL_INT(VALUE_HIGH) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_LOW),        			MP_OBJ_NEW_SMALL_INT(VALUE_LOW) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_INPUT),        		MP_OBJ_NEW_SMALL_INT(MODE_IN) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_OUTPUT),       		MP_OBJ_NEW_SMALL_INT(MODE_OUT) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_INPUT_PU), 			MP_OBJ_NEW_SMALL_INT(MODE_INPUT_PU) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_INPUT_PD),    		MP_OBJ_NEW_SMALL_INT(MODE_INPUT_PD) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_AN_INPUT),   			MP_OBJ_NEW_SMALL_INT(MODE_AN_INPUT) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_AN_OUTPUT), 			MP_OBJ_NEW_SMALL_INT(MODE_AN_OUTPUT) },

#include "genhdr/pins_af_const.h"
};

STATIC MP_DEFINE_CONST_DICT(pin_locals_dict, pin_locals_dict_table);

const mp_obj_type_t pin_type = {
    { &mp_type_type },
    .name = MP_QSTR_Pin,
    .make_new = pin_make_new,
    .locals_dict = (mp_obj_t)&pin_locals_dict,
};

/// \moduleref pyb
/// \class PinAF - Pin Alternate Functions
///
/// A Pin represents a physical pin on the microcprocessor. Each pin
/// can have a variety of functions (GPIO, I2C SDA, etc). Each PinAF
/// object represents a particular function for a pin.
///
/// Usage Model:
///
///     x3 = pyb.Pin.board.X3
///     x3_af = x3.af_list()
///
/// x3_af will now contain an array of PinAF objects which are availble on
/// pin X3.
///
/// For the pyboard, x3_af would contain:
///     [Pin.AF1_TIM2, Pin.AF2_TIM5, Pin.AF3_TIM9, Pin.AF7_USART2]
///
/// Normally, each peripheral would configure the af automatically, but sometimes
/// the same function is available on multiple pins, and having more control
/// is desired.
///
/// To configure X3 to expose TIM2_CH3, you could use:
///    pin = pyb.Pin(pyb.Pin.board.X3, mode=pyb.Pin.AF_PP, af=pyb.Pin.AF1_TIM2)
/// or:
///    pin = pyb.Pin(pyb.Pin.board.X3, mode=pyb.Pin.AF_PP, af=1)

/// \method __str__()
/// Return a string describing the alternate function.
STATIC void pin_af_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    pin_af_obj_t *self = self_in;
    mp_printf(print, "Pin.%q", self->name);
}

/// \method index()
/// Return the alternate function index.
STATIC mp_obj_t pin_af_index(mp_obj_t self_in) {
    pin_af_obj_t *af = self_in;
    return MP_OBJ_NEW_SMALL_INT(af->idx);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pin_af_index_obj, pin_af_index);

/// \method name()
/// Return the name of the alternate function.
STATIC mp_obj_t pin_af_name(mp_obj_t self_in) {
    pin_af_obj_t *af = self_in;
    return MP_OBJ_NEW_QSTR(af->name);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pin_af_name_obj, pin_af_name);

/// \method reg()
/// Return the base register associated with the peripheral assigned to this
/// alternate function. For example, if the alternate function were TIM2_CH3
/// this would return stm.TIM2
STATIC mp_obj_t pin_af_reg(mp_obj_t self_in) {
    pin_af_obj_t *af = self_in;
    return MP_OBJ_NEW_SMALL_INT((mp_uint_t)af->reg);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pin_af_reg_obj, pin_af_reg);

STATIC const mp_map_elem_t pin_af_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_index),   (mp_obj_t)&pin_af_index_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_name),    (mp_obj_t)&pin_af_name_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_reg),     (mp_obj_t)&pin_af_reg_obj },
};
STATIC MP_DEFINE_CONST_DICT(pin_af_locals_dict, pin_af_locals_dict_table);

const mp_obj_type_t pin_af_type = {
    { &mp_type_type },
    .name = MP_QSTR_PinAF,
    .print = pin_af_obj_print,
    .locals_dict = (mp_obj_t)&pin_af_locals_dict,
};
