extern void setup();
extern void run();

int main(void){
  /*   SRAM_Init(); called in system_hsx.c before interrupts are enabled */

  setup();	

  run();

  return 0;
}
