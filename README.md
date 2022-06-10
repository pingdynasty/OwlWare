# OwlWare
Firmware for the OWL programmable effects pedal.

All code licensed under the Gnu GPL unless otherwise stated.

The OwlWare firmware in this repository has been superceded by [OpenWare](https://github.com/pingdynasty/OpenWare). For more details see [www.rebeltech.org](https://www.rebeltech.org), the [community forum](community.rebeltech.org), or refer to [www.openwarelab.org](https://www.openwarelab.org/) for documentation.


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

If you want to install it elsewhere, edit `common.mk` in the __OwlWare__ directory to point to the right `TOOLROOT` directory.


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

### Build Options
The default configuration builds an OWL Pedal debug build. To build the release version (no debug information, apprx 2x performance) add `CONFIG=Release`. To build the OWL Modular version, add `PLATFORM=Modular`. Make sure to do a `make clean` after changing build options.

## Deploy
In the __OwlWare__ directory, type in:
* `make dfu` to build the bin file and upload to an OWL device in DFU mode, connected by USB
* `make debug` to build the elf file and upload to an OWL device connected with `st-util` using an stlink programmer


[gcc-arm]: https://launchpad.net/gcc-arm-embedded
[dfu-util]: http://dfu-util.gnumonks.org
[stlink]: https://github.com/texane/stlink
[gnu-make]: http://www.gnu.org/software/make/
[owlware-zip]: https://github.com/pingdynasty/OwlWare/archive/master.zip
[owlpatches-zip]: https://github.com/pingdynasty/OwlPatches/archive/master.zip
[cygwin]: http://www.cygwin.com
[owl-nest]: https://github.com/pingdynasty/OwlNest
