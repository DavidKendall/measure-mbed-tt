/*
 * Simple program for the application board
 *
 * Implemented using a time-triggered scheduler.
 *
 * DK - 08-June-2018
 */


#include <stdbool.h>
#include <stdint.h>
#include <scheduler.h>
#include <mbed.h>
#include "C12832.h"
#include "MMA7660.h"
#include "LM75B.h"
#include <stdio.h>
#include <assert.h>
#include <MK64F12.h>

static DigitalOut red(LED_RED);
static DigitalOut green(LED_GREEN);
static DigitalOut led_app_red(D5);
static DigitalOut led_app_green(D9);
C12832 lcd(D11, D13, D12, D7, D10);
AnalogIn pot1(A0); // Pot 1 - Left
AnalogIn pot2(A1); // Pot 2 - Right
float pot1Val;
float pot2Val;
DigitalIn joystick[5] = {D4, A2, A3, A4, A5};
char symbols[] = {'C', 'U', 'D', 'L', 'R'};
char joystickVal;
MMA7660 accel(D14, D15);
float accelVal[3];
LM75B temp(D14, D15);
float tempVal;
uint32_t timeElapsed;

void samplePot(void);
void sampleJoystick(void);
void sampleAccel(void);
void sampleTemp(void);
void led1ToggleTask(void);
void led2ToggleTask(void);
void updatePot(void);
void updateJoystick(void);
void updateAccel(void);
void updateTemp(void);
void counterInit(void);
void counterStart(void);
uint32_t counterStop(void);

extern "C" {
  void PIT1_IRQHandler(void);
}

int main () {
  counterInit();
  red = 0;
  green = 1;
  led_app_red = 1;
  timeElapsed = 0;
  wait(0.5);
  counterStart();
  pot1Val = pot1.read();
  //lcd.locate(2,8);
  //lcd.printf("Hello world!\n", 0.66);
  //wait(0.5);
  timeElapsed = counterStop();
  lcd.cls();
  lcd.locate(86, 8);
  lcd.printf("%u", timeElapsed);

  schInit();
  schAddTask(samplePot, 0, 20);
  schAddTask(sampleJoystick, 0, 10);
  schAddTask(sampleAccel, 0, 20);
  schAddTask(sampleTemp, 0, 100);
  schAddTask(led1ToggleTask, 11, 50);
  schAddTask(led2ToggleTask, 61, 50);
  schAddTask(updatePot, 3, 20);
  schAddTask(updateJoystick, 7, 10);
  schAddTask(updateAccel, 11, 20);
  schAddTask(updateTemp, 13, 100);

  schStart();
  
  while (true) {
    schDispatch();
  }
}

void samplePot(void) {
  pot1Val = pot1.read();
  pot2Val = pot2.read();
}

void sampleJoystick(void) {
  int i = 0;

  joystickVal = '-';
  for (i = 0; i < 5; i+=1) {
      if (joystick[i] == 1) {
          joystickVal = symbols[i];
          break;
      }
   }
}

void sampleAccel(void) {
  accelVal[0] = accel.x();
  accelVal[1] = accel.y();
  accelVal[2] = accel.z();
}

void sampleTemp(void) {
  tempVal = temp.read();
}

void led1ToggleTask(void) {
  red = 1 - red;
}

void led2ToggleTask(void) {
  led_app_green = 1 - led_app_green;
}

void updatePot(void) {
  lcd.locate(0, 0);
  lcd.printf("L: %0.2f", pot1Val);
  lcd.locate(0, 8);
  lcd.printf("R: %0.2f", pot2Val);
}

void updateJoystick(void) {
  lcd.locate(0, 16);
   lcd.printf("J: %c", joystickVal);
}

void updateAccel(void) {
  lcd.locate(43, 0);
  lcd.printf("X: %0.2f", accelVal[0]);
  lcd.locate(43, 8);
  lcd.printf("Y: %0.2f", accelVal[1]);
  lcd.locate(43, 16);
  lcd.printf("Z: %0.2f", accelVal[2]);
}

void updateTemp(void) {
  lcd.locate(86, 0);
  lcd.printf("T: %02.2f", tempVal);
}

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
  
  counter = 0XFFFFFFFF - PIT->CHANNEL[2].CVAL;  // get the value of the timer counter
  /* Disable the timer */
  PIT->CHANNEL[2].TCTRL &= ~PIT_TCTRL_TEN_MASK;
  return counter;
}

extern "C" {
void PIT1_IRQHandler(void) {
    /* Clear the timer interrupt flag to allow further timer interrupts */
    PIT->CHANNEL[2].TFLG |= PIT_TFLG_TIF_MASK;
    /* We should never get here - timer overflow */
    assert(false);
}
}
