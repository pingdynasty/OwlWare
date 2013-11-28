# OwlWare

Firmware for the OWL programmable effects pedal.

All code licensed under the Gnu GPL unless otherwise stated.

See http://hoxtonowl.com for more details.

# Prerequisites

## GNU Tools for ARM Embedded Processors

Download the GNU ARM toolchain from here:
https://launchpad.net/gcc-arm-embedded

Unzip in the Tools directory and (if necessary) edit Makefile.common to point to the right TOOLROOT directory.

## STM32F4DISCOVERY Board Firmware Package
STSW-STM32068
Download the file stsw-stm32068.zip from here:
http://www.st.com/web/catalog/tools/FM147/CL1794/SC961/SS1743/PF257904
and unzip in the Libraries directory.

If you have installed the software elsewhere, edit Makefile.f4 to point to the right LIBROOT directory.
