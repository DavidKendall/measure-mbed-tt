#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <cmsis.h>
#include <scheduler.h>

static volatile uint32_t tickCount = 0;
static schTCB_t schTasks[TT_SCHED_MAX_TASKS];

void SysTick_Handler(void) {
  tickCount += 1;
}

void schInit(void) {   // initialise the scheduler
  
  for (uint8_t i = 0; i < TT_SCHED_MAX_TASKS; i+=1) {
    schTasks[i].task = (pVoidFunc_t)0;
    schTasks[i].delay = 0;
    schTasks[i].period = 0;
  }
}

void schAddTask(               // add a task to the task set
  pVoidFunc_t task,              // the task to add
  uint32_t delay,                // the delay in ms
  uint32_t period) {             // the period
  
  uint8_t i = 0;
  
  while (i < TT_SCHED_MAX_TASKS && schTasks[i].task != (pVoidFunc_t)0) {
    i += 1;
  }
  assert(i < TT_SCHED_MAX_TASKS);
  schTasks[i].task = task;
  schTasks[i].delay = delay + 1;
  schTasks[i].period = period;
}

void schStart(void) {           // start ticking
  SysTick_Config((SystemCoreClock / TT_SCHED_TICK_HZ) - 1);
}

void schDispatch(void) {          // update after a tick -- ISR
  __disable_irq();
  bool isUpdateNeeded = (tickCount > 0);
  __enable_irq();

  while (isUpdateNeeded) {
    for (uint8_t i = 0; i < TT_SCHED_MAX_TASKS; i+=1) {
      if (schTasks[i].task) {
        if (--schTasks[i].delay == 0) {
          (*(schTasks[i].task))();
          schTasks[i].delay = schTasks[i].period;
        } 
      }
    }
    __disable_irq();
    tickCount -= 1;
    isUpdateNeeded = (tickCount > 0);
    __enable_irq();
    // assert(!isUpdateNeeded);
  }
  schSleep();
}

void schSleep(void) {         // go to sleep, if possible, to save power
  __WFI();
}

