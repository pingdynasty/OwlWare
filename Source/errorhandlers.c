#include "device.h"
#include "owlcontrol.h"
#include <inttypes.h>

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line){ 
  volatile uint32_t delayCounter;
  /* Blink LEDs */
  setLed(RED);
  for(delayCounter = 0; delayCounter < 4000000; delayCounter++);
  setLed(NONE);
  for(delayCounter = 0; delayCounter < 4000000; delayCounter++);
  setLed(RED);
  for(;;);
  /* NVIC_SystemReset(); */
}

/* exception handlers - so we know what's failing */
void NMI_Handler(void){
  assert_failed(0, 0);
}

void MemManage_Handler(void){ 
  assert_failed(0, 0);
}

void BusFault_Handler(void){ 
  assert_failed(0, 0);
}

void UsageFault_Handler(void){ 
  for(;;);
}

void DebugMon_Handler(void){ 
  for(;;);
}

/**
 * @see http://www.freertos.org/Debugging-Hard-Faults-On-Cortex-M-Microcontrollers.html
 */
void prvGetRegistersFromStack(uint32_t *pulFaultStackAddress) {
/* These are volatile to try and prevent the compiler/linker optimising them
away as the variables never actually get used. If the debugger won't show the
values of the variables, make them global my moving their declaration outside
of this function. */
  volatile uint32_t r0;
  volatile uint32_t r1;
  volatile uint32_t r2;
  volatile uint32_t r3;
  volatile uint32_t r12;
  volatile uint32_t lr; /* Link register. */
  volatile uint32_t pc; /* Program counter. */
  volatile uint32_t psr;/* Program status register. */

  r0 = pulFaultStackAddress[0];
  r1 = pulFaultStackAddress[1];
  r2 = pulFaultStackAddress[2];
  r3 = pulFaultStackAddress[3];

  r12 = pulFaultStackAddress[4];
  lr = pulFaultStackAddress[5];
  pc = pulFaultStackAddress[6];
  psr = pulFaultStackAddress[7];

  /* When the following line is hit, the variables contain the register values. */
  for (;;);
}

/* The fault handler implementation calls a function called
   prvGetRegistersFromStack(). */
static void HardFault_Handler(void) {
  __asm volatile (
     " tst lr, #4                                                \n"
     " ite eq                                                    \n"
     " mrseq r0, msp                                             \n"
     " mrsne r0, psp                                             \n"
     " ldr r1, [r0, #24]                                         \n"
     " ldr r2, handler2_address_const                            \n"
     " bx r2                                                     \n"
     " handler2_address_const: .word prvGetRegistersFromStack    \n"
     );
}

void WWDG_IRQHandler(void) {
  for(;;);
}
void PVD_IRQHandler(void) {
  for(;;);
}

/* void HardFault_Handler(void){  */
/*   assert_failed(0, 0); */
/* } */

/*
void SVC_Handler(void){ 
  for(;;);
}

void PendSV_Handler(void){ 
  for(;;);
}
*/
