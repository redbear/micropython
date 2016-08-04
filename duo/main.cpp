
#include "application.h"
#include "wiring.h"
#include "extint.h"

SYSTEM_MODE(SEMI_AUTOMATIC);

uint32_t pyb_extint_callback[EXTI_NUM_VECTORS];


void setup()
{
	extint_init0();
}

void loop()
{
	mp_setup();
	mp_loop();
}


