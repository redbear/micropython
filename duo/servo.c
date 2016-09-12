#include <stdio.h>
#include <string.h>

#include "py/nlr.h"
#include "py/runtime.h"
#include "py/binary.h"
#include "py/mphal.h"
#include "servo.h"
#include "pin.h"
#include "genhdr/pins.h"
#include "wiring.h"

typedef struct _pyb_servo_obj_t {
    mp_obj_base_t base;
    uint16_t      servo_pin;
}pyb_servo_obj_t;

STATIC mp_obj_t pyb_servo_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    pin_obj_t *pin = args[0];
	pyb_servo_obj_t *self = m_new0(pyb_servo_obj_t, 1);
    self->base.type = &pyb_servo_type;
    self->servo_pin = pin_mapping(pin);

    return self;
}

STATIC mp_obj_t pyb_servo_attach(mp_obj_t self_in) {

	pyb_servo_obj_t *servo = self_in;
	if(servo_attach(servo->servo_pin)) {
		return mp_const_true;
	} else {
		return mp_const_false;
	}

}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_servo_attach_obj, pyb_servo_attach);

STATIC mp_obj_t pyb_servo_write(mp_obj_t self_in, mp_obj_t angle) {
	pyb_servo_obj_t *servo = self_in;
	servo_write(servo->servo_pin, mp_obj_get_int(angle));

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_servo_write_obj, pyb_servo_write);

STATIC mp_obj_t pyb_servo_write_microseconds(mp_obj_t self_in, mp_obj_t pulseWidth) {
	pyb_servo_obj_t *servo = self_in;
	servo_writeMicroseconds(servo->servo_pin, mp_obj_get_int(pulseWidth));

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_servo_write_microseconds_obj, pyb_servo_write_microseconds);

STATIC mp_obj_t pyb_servo_read_microseconds(mp_obj_t self_in) {
	pyb_servo_obj_t *servo = self_in;

    return MP_OBJ_NEW_SMALL_INT(servo_readMicroseconds(servo->servo_pin));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_servo_read_microseconds_obj, pyb_servo_read_microseconds);

STATIC mp_obj_t pyb_servo_read(mp_obj_t self_in) {
	pyb_servo_obj_t *servo = self_in;

    return MP_OBJ_NEW_SMALL_INT(servo_read(servo->servo_pin));

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_servo_read_obj, pyb_servo_read);

STATIC mp_obj_t pyb_servo_attached(mp_obj_t self_in) {
	pyb_servo_obj_t *servo = self_in;
	if(servo_attached(servo->servo_pin)) {
		return mp_const_true;
	} else {
		return mp_const_false;
	}

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_servo_attached_obj, pyb_servo_attached);

STATIC mp_obj_t pyb_servo_detach(mp_obj_t self_in) {
	pyb_servo_obj_t *servo = self_in;
	if(servo_detach(servo->servo_pin)) {
		return mp_const_true;
	} else {
		return mp_const_false;
	}

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_servo_detach_obj, pyb_servo_detach);

STATIC mp_obj_t pyb_servo_set_trim(mp_obj_t self_in, mp_obj_t trim) {
	pyb_servo_obj_t *servo = self_in;
	servo_setTrim(servo->servo_pin, mp_obj_get_int(trim));

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_servo_set_trim_obj, pyb_servo_set_trim);

STATIC const mp_map_elem_t servo_locals_dict_table[] = {
	{ MP_OBJ_NEW_QSTR(MP_QSTR_attach), (mp_obj_t)&pyb_servo_attach_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_attached), (mp_obj_t)&pyb_servo_attached_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_detach), (mp_obj_t)&pyb_servo_detach_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&pyb_servo_read_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_write), (mp_obj_t)&pyb_servo_write_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_writeMicroseconds), (mp_obj_t)&pyb_servo_write_microseconds_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_readMicroseconds), (mp_obj_t)&pyb_servo_read_microseconds_obj},
	{ MP_OBJ_NEW_QSTR(MP_QSTR_setTrim), (mp_obj_t)&pyb_servo_set_trim_obj},
};

STATIC MP_DEFINE_CONST_DICT(servo_locals_dict, servo_locals_dict_table);

const mp_obj_type_t pyb_servo_type = {
    { &mp_type_type },
    .name = MP_QSTR_SERVO,
	.make_new = pyb_servo_make_new,
    .locals_dict = (mp_obj_t)&servo_locals_dict,
};

