/* Linker script for STM32F407 */

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
/* _estack = 0x2000c000;    /\* end of loader RAM *\/ */
_estack = 0x10004000;    /* end of CCM RAM */

/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size  = 0x000; /* required amount of heap  */
_Min_Stack_Size = 0x800; /* required amount of stack */

/* Specify the memory areas */
MEMORY
{
  BOOTLOADER (r)   : ORIGIN = 0x08000000, LENGTH = 16K    /* Sector 0 */
  EEPROM (r)       : ORIGIN = 0x08004000, LENGTH = 16K    /* Sector 1 */
  FLASH (rx)       : ORIGIN = 0x08008000, LENGTH = 352K   /* Sector 2-6 */
  PATCHFLASH (rx)  : ORIGIN = 0x08060000, LENGTH = 128K   /* Sector 7 */
  PROGRAM3 (r)     : ORIGIN = 0x08080000, LENGTH = 128K   /* Sector 8 */
  PROGRAM2 (r)     : ORIGIN = 0x080a0000, LENGTH = 128K   /* Sector 9 */
  PROGRAM1 (r)     : ORIGIN = 0x080c0000, LENGTH = 128K   /* Sector 10 */
  PROGRAM0 (r)     : ORIGIN = 0x080e0000, LENGTH = 128K   /* Sector 11 */ /* total Flash memory is 1Mb */
  CCMRAM (rw)      : ORIGIN = 0x10000000, LENGTH = 16K    /* Closely Coupled Memory */
  RAM (rwx)        : ORIGIN = 0x20000000, LENGTH = 48K
  PATCHRAM (rwx)   : ORIGIN = 0x2000c000, LENGTH = 64K    /* total RAM is 112kb */
  BANK1_SRAM1 (rwx): ORIGIN = 0x60000000, LENGTH = 0K
  BANK1_SRAM2 (rwx): ORIGIN = 0x64000000, LENGTH = 0K
  BANK1_SRAM3 (rwx): ORIGIN = 0x68000000, LENGTH = 1M
  BANK1_SRAM4 (rwx): ORIGIN = 0x6c000000, LENGTH = 0K
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
	*(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

.fastcode : /* AT(LOADADDR(.startup) + SIZEOF(.startup)) */
{
    . = ALIGN(8);
   _fastcode_start = .;
/* functions with __attribute__ ((section (".coderam")))*/
   *(.coderam)
    . = ALIGN(8);
   _fastcode_end = .;
  } >RAM AT>FLASH
_fastcode_load = LOADADDR (.fastcode);
_fastcode_size = _fastcode_end - _fastcode_start;

   .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
    .ARM : {
    __exidx_start = .;
      *(.ARM.exidx*)
      __exidx_end = .;
    } >FLASH

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(.fini_array*))
    KEEP (*(SORT(.fini_array.*)))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

/*   .eeprom (NOLOAD) : */
/*     { */
/*     . = ALIGN(4); */
/*         *(.eeprom) */
/*     . = ALIGN(4); */
/*     } >EEPROM */

  /* used by the startup to initialize data */
  _sidata = .;

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data : AT ( _sidata )
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM

  /* CCM section, vars must be located here explicitly */
  /* Example: int foo __attribute__ ((section (".ccmdata"))); */
  .ccmdata (NOLOAD) :
  {
    . = ALIGN(4);
    *(.ccmdata)
    . = ALIGN(4);
  } >CCMRAM

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(4);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(4);
  } >CCMRAM

/* /\* Shared RAM section, vars must be located here explicitly *\/ */
/* /\* Example: int foo __attribute__ ((section (".sharedram"))); *\/ */
/* .sharedram (NOLOAD) : */
/* { */
/*   . = ALIGN(4); */
/*   *(.sharedram) */
/*   . = ALIGN(4); */
/* } >SHAREDRAM */

  /* External SRAM, vars must be located here explicitly */
  /* Example: int foo __attribute__ ((section (".extdata"))); */
/* .extdata (NOLOAD) : */
/* { */
/*   . = ALIGN(4); */
/*   *(.extdata) */
/*   . = ALIGN(4); */
/* } >BANK1_SRAM3 */

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
