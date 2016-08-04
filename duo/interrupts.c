#include "interrupts.h"
#include "py/obj.h"
#include "extint.h"

static void exti0_isr(void) {
	Handle_EXTI_Irq(0);
}

static void exti1_isr(void) {
	Handle_EXTI_Irq(1);
}

static void exti2_isr(void) {
	Handle_EXTI_Irq(2);
}

static void exti3_isr(void) {
	Handle_EXTI_Irq(2);
}

static void exti4_isr(void) {
	Handle_EXTI_Irq(4);
}

static void exti5_isr(void) {
	Handle_EXTI_Irq(5);
}

static void exti6_isr(void) {
	Handle_EXTI_Irq(6);
}

static void exti7_isr(void) {
	Handle_EXTI_Irq(7);
}

static void exti8_isr(void) {
	Handle_EXTI_Irq(8);
}

static void exti9_isr(void) {
	Handle_EXTI_Irq(9);
}

static void exti10_isr(void) {
	Handle_EXTI_Irq(10);
}

static void exti11_isr(void) {
	Handle_EXTI_Irq(11);
}

static void exti12_isr(void) {
	Handle_EXTI_Irq(12);
}

static void exti13_isr(void) {
	Handle_EXTI_Irq(13);
}

static void exti14_isr(void) {
	Handle_EXTI_Irq(14);
}

static void exti15_isr(void) {
	Handle_EXTI_Irq(15);
}

static isr_fn_t exti_isr[16] = {
    exti0_isr,
    exti1_isr,
    exti2_isr,
    exti3_isr,
    exti4_isr,
    exti5_isr,
    exti6_isr,
    exti7_isr,
    exti8_isr,
    exti9_isr,
    exti10_isr,
    exti11_isr,
    exti12_isr,
    exti13_isr,
    exti14_isr,
    exti15_isr,
};

isr_fn_t get_exti_isr(uint8_t pin_source)
{
    return exti_isr[pin_source];
}

