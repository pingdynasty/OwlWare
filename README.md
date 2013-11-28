# OwlWare

Firmware for the OWL programmable effects pedal.

All code licensed under the Gnu GPL unless otherwise stated.

See http://hoxtonowl.com for more details.


# Prerequisites

## GNU Tools for ARM Embedded Processors

Download the [GNU ARM toolchain][gcc-arm] for your operating system. Unzip in the Tools directory and (if necessary) edit Makefile.common to point to the right TOOLROOT directory.

## STM32F4DISCOVERY Firmware Package
Download [the STM32F4-Discovery Firmware Package][STM32068] and unzip the file stsw-stm32068.zip in the Libraries directory.

If you have installed the software elsewhere, edit Makefile.f4 to point to the right LIBROOT directory.

## DFU Util
To upload firmware over USB using DFU you will also need dfu-util. Download and install from [here][dfu-util].

## ST Util
To upload firmware using an STLINKv1 or STLINKv2 programmer (such as those on the ST Discovery boards) download and install it from [here][stlink].


## Make

To build and deploy the project using the provided Makefiles you will need some version of the make utility, for example GNU make.

# Instructions

## Build
Open a terminal or command window in the project directory and type in:
* `make` to build `Builds/OwlWare.elf`
* `make bin` to build `Builds/OwlWare.bin`
* `make clean all` to rebuild all intermediary files

## Deploy
In the project directory, type in:
* `make dfu` to build the bin file and upload to an OWL device in DFU mode, connected by USB
* `make debug` to build the elf file and upload to an OWL device connected with st-util using an stlink programmer


[STM32068]: http://www.st.com/web/catalog/tools/FM147/CL1794/SC961/SS1743/PF257904
[gcc-arm]: https://launchpad.net/gcc-arm-embedded
[dfu-util]: http://dfu-util.gnumonks.org
[stlink]: https://github.com/texane/stlink
