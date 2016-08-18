#ifndef DUO_TCP_CLIENT_H_
#define DUO_TCP_CLIENT_H_

#include "wiring.h"

extern const mp_obj_type_t pyb_TCP_client_type;

mp_obj_t TCP_client_write_data(tcp_client * client_in, mp_obj_t buf_in);
mp_obj_t TCP_client_read_data(tcp_client * client_in, mp_obj_t buf_in, mp_obj_t size_in);

#endif /* DUO_TCP_CLIENT_H_ */
