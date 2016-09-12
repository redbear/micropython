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

STATIC const mp_map_elem_t pyb_timer_locals_dict_table[] = {
    // instance methods
};

STATIC MP_DEFINE_CONST_DICT(pyb_timer_locals_dict, pyb_timer_locals_dict_table);

const mp_obj_type_t pyb_timer_type = {
    { &mp_type_type },
    .name = MP_QSTR_TIMER,
    .locals_dict = (mp_obj_t)&pyb_timer_locals_dict,
};
