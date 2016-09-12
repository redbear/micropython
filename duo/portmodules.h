/*
 * portmodules.h
 *
 *  Created on: 2016��9��9��
 *      Author: Go
 */

#ifndef DUO_PORTMODULES_H_
#define DUO_PORTMODULES_H_

extern const mp_obj_module_t pyb_module;
extern const mp_obj_module_t mp_module_time;

MP_DECLARE_CONST_FUN_OBJ(time_delay_ms_obj);
MP_DECLARE_CONST_FUN_OBJ(time_delay_us_obj);

#endif /* DUO_PORTMODULES_H_ */
