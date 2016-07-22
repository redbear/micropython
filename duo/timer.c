#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "py/nlr.h"
#include "py/runtime.h"
#include "py/gc.h"
#include "timer.h"
#include "pin.h"
#include "wiring.h"
#include "tone_api.h"

STATIC const uint16_t no_pwm_pin_list[] = {A2, A3, D5, D6, D7};

STATIC mp_obj_t pyb_timer_pwm(mp_obj_t self_in, mp_obj_t val) {
    int i = 0;
	pin_obj_t *self = self_in;
    uint16_t pin = pin_mapping(self);
    for(; i < 5; i++){
    	if(pin == no_pwm_pin_list[i]){
    		printf("Error: The pin does not support PWM function\n");
    		return mp_const_none;
    	}
    }
    pinMode(pin, OUTPUT);
    wiring_analogWrite(pin, mp_obj_get_int(val));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_timer_pwm_obj, pyb_timer_pwm);

STATIC mp_obj_t pyb_timer_tone(mp_obj_t self_in, mp_obj_t freq, mp_obj_t duration) {
    int i = 0;
	pin_obj_t *self = self_in;
    uint16_t pin = pin_mapping(self);

    tone(pin, mp_obj_get_int(freq), mp_obj_get_int(duration));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_timer_tone_obj, pyb_timer_tone);

STATIC mp_obj_t pyb_timer_no_tone(mp_obj_t self_in) {
    int i = 0;
	pin_obj_t *self = self_in;
    uint16_t pin = pin_mapping(self);

    noTone(pin);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_timer_no_tone_obj, pyb_timer_no_tone);

STATIC const mp_map_elem_t pyb_timer_locals_dict_table[] = {
    // instance methods
    { MP_OBJ_NEW_QSTR(MP_QSTR_pwm), (mp_obj_t)&pyb_timer_pwm_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_tone), (mp_obj_t)&pyb_timer_tone_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_no_tone), (mp_obj_t)&pyb_timer_no_tone_obj },
};

STATIC MP_DEFINE_CONST_DICT(pyb_timer_locals_dict, pyb_timer_locals_dict_table);

const mp_obj_type_t pyb_timer_type = {
    { &mp_type_type },
    .name = MP_QSTR_TIMER,
    .locals_dict = (mp_obj_t)&pyb_timer_locals_dict,
};
