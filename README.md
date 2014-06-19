# OwlWare
Firmware for the OWL programmable effects pedal.

All code licensed under the Gnu GPL unless otherwise stated.

See http://hoxtonowl.com for more details.


# Prerequisites
First you need to get the OwlWare source code into a directory of your choice. We will refer to this as the __OwlWare__ directory.

If you have or create a github.com account then you can always get the latest version of the software with git.

* Clone this github repository with `git clone git@github.com:pingdynasty/OwlWare.git`
* Open a terminal or command window in the cloned repository OwlWare (the __OwlWare__ directory)
* Do a `git submodule init` and `git submodule update` to get the required submodules

If you are not familiar with git, you can also download the source code as a [zip file][owlware-zip].

You will then also have to manually download [OwlPatches][owlpatches-zip] and unzip in the __OwlWare/Libraries__ directory.


## GNU Tools for ARM Embedded Processors
Download the [GNU ARM toolchain][gcc-arm] for your operating system. Unzip in the __OwlWare/Tools__ directory.

If you want to install it elsewhere, edit `Makefile.common` in the __OwlWare__ directory to point to the right `TOOLROOT` directory.


## STM32F4DISCOVERY Firmware Package
Download [the STM32F4-Discovery Firmware Package][STM32068] and unzip the file `stsw-stm32068.zip` in the __OwlWare/Libraries__ directory.

If you have installed the software elsewhere, edit `Makefile.f4` in the __OwlWare__ directory to point to the right `LIBROOT` directory.


## DFU Util
To upload firmware over USB, you will also need `dfu-util` which you can download from [here][dfu-util].
Unzip the binaries (or compile them) in the __OwlWare/Tools/dfu-util__ directory.

Recommended for most users. [OwlNest][owl-nest] can also be used.


## ST Util
For those who want to upload firmware using an STLINKv1 or STLINKv2 programmer (such as those on the ST Discovery boards).
Download `stlink` from [here][stlink] and install it in the __OwlWare/Tools/stlink__ directory..

For most users, this will not be required.


## Make
To build and deploy the project using the provided Makefiles you will need some version of the make utility, for example [GNU make][gnu-make].

For Windows users, we recommend installing [Cygwin][cygwin].


# Instructions

## Build
Open a terminal or command window in the __OwlWare__ directory and type in:
* `make` to build `Builds/OwlWare.elf`
* `make bin` to build `Builds/OwlWare.bin`
* `make clean all` to rebuild all intermediary files


## Deploy
In the __OwlWare__ directory, type in:
* `make dfu` to build the bin file and upload to an OWL device in DFU mode, connected by USB
* `make debug` to build the elf file and upload to an OWL device connected with `st-util` using an stlink programmer


[STM32068]: http://www.st.com/web/catalog/tools/FM147/CL1794/SC961/SS1743/PF257904
[gcc-arm]: https://launchpad.net/gcc-arm-embedded
[dfu-util]: http://dfu-util.gnumonks.org
[stlink]: https://github.com/texane/stlink
[gnu-make]: http://www.gnu.org/software/make/
[owlware-zip]: https://github.com/pingdynasty/OwlWare/archive/master.zip
[owlpatches-zip]: https://github.com/pingdynasty/OwlPatches/archive/master.zip
[cygwin]: http://www.cygwin.com
[owl-nest]: http://hoxtonowl.com/software/owlnest
