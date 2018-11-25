#include <counter.h>
#include <stdbool.h>
#include <assert.h>
#include <MK64F12.h>

#ifdef __cplusplus
extern "C" {
#endif

void counterInit(void) {
    /* Open the clock gate to the PIT */
    SIM->SCGC6 |= (1u << 23);
    /* Enable the clock for the PIT timers. Continue to run in debug mode */
    PIT->MCR = 0u;
    /* Disable the timer */
    PIT->CHANNEL[2].TCTRL &= ~PIT_TCTRL_TEN_MASK;
    /* Period p = maximum available, bus clock f = 60 MHz, v = pf - 1 */ 
    PIT->CHANNEL[2].LDVAL = 0xFFFFFFFF;
    /* Enable interrupt on timeout */
    PIT->CHANNEL[2].TCTRL |= PIT_TCTRL_TIE_MASK;
    /* Enable the interrupt in the NVIC */
    NVIC_EnableIRQ(PIT1_IRQn);
}


void counterStart(void) {
    /* Start the timer running */
    PIT->CHANNEL[2].TCTRL |= PIT_TCTRL_TEN_MASK;
}

uint32_t counterStop(void) {
  uint32_t counter = 0;

  /* get the time elapsed since started */
  counter = 0XFFFFFFFF - PIT->CHANNEL[2].CVAL;
  /* Disable the timer */
  PIT->CHANNEL[2].TCTRL &= ~PIT_TCTRL_TEN_MASK;
  return counter;
}

void PIT1_IRQHandler(void) {
    /* Clear the timer interrupt flag to allow further timer interrupts */
    PIT->CHANNEL[2].TFLG |= PIT_TFLG_TIF_MASK;
    /* We should never get here - timer overflow */
    assert(false);
}

#ifdef __cplusplus
}
#endif
