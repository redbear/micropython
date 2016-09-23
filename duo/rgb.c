#include <stdio.h>

#include "py/nlr.h"
#include "py/runtime.h"
#include "rgb.h"
#include "pin.h"
#include "genhdr/pins.h"
#include "gpio_api.h"
#include "wiring.h"

STATIC mp_obj_t pyb_rgb_control(mp_obj_t override) {

	if(mp_const_true == mp_obj_new_bool(mp_obj_get_int(override))) {
		rgb_controll(1);
	} else {
		rgb_controll(0);
	}

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_rgb_control_obj, pyb_rgb_control);

STATIC mp_obj_t pyb_rgb_controlled() {

	if(rgb_controlled()) {
		return mp_const_true;
	} else {
		return mp_const_false;
	}

}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_rgb_controlled_obj, pyb_rgb_controlled);

STATIC mp_obj_t pyb_rgb_color(mp_obj_t red, mp_obj_t green, mp_obj_t blue) {

	rgb_color(mp_obj_get_int(red), mp_obj_get_int(green), mp_obj_get_int(blue));

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_rgb_color_obj, pyb_rgb_color);

STATIC mp_obj_t pyb_rgb_brightness(mp_obj_t brightness) {

	rgb_setBrightness(mp_obj_get_int(brightness),1);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_rgb_brightness_obj, pyb_rgb_brightness);


STATIC const mp_map_elem_t pyb_rgb_locals_dict_table[] = {
    // instance methods
    { MP_OBJ_NEW_QSTR(MP_QSTR_control), (mp_obj_t)&pyb_rgb_control_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_controlled), (mp_obj_t)&pyb_rgb_controlled_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_color), (mp_obj_t)&pyb_rgb_color_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_brightness), (mp_obj_t)&pyb_rgb_brightness_obj },
};

STATIC MP_DEFINE_CONST_DICT(pyb_rgb_locals_dict, pyb_rgb_locals_dict_table);

const mp_obj_type_t pyb_rgb_type = {
    { &mp_type_type },
    .name = MP_QSTR_RGB,
    .locals_dict = (mp_obj_t)&pyb_rgb_locals_dict,
};

