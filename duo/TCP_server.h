#ifndef DUO_TCP_SERVER_H_
#define DUO_TCP_SERVER_H_

#define MAX_SERVER_SOCKETS  3
#define MAX_CLIENT_SOCKETS  20

typedef enum {
    TCP_SERVER_0 = 0,
    TCP_SERVER_1 = 1,
	TCP_SERVER_2 = 2,
} pyb_TCP_server_t;

void TCP_server_init0(void);

extern const mp_obj_type_t pyb_TCP_server_type;

#endif /* DUO_TCP_SERVER_H_ */
