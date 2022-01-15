
## atlas430

library of HAL functions that provide a glue layer between the upstream [Texas Instruments mspdriverlib](https://www.ti.com/tool/MSPDRIVERLIB) and projects that need quick prototyping while also allowing easier code migration from one msp430 microcontroller to another. on one hand it has a number of generic functions and configurable compile-time defines that will set up the hardware abstraction layer allowing the user to focus on the higher level code and on the other hand it can be out-of-the-way and allow direct access to the registers or to the TI driverlib. strives to also provide limited device-specific pin initialization for about 320 microcontrollers from the F5xx, F6xx, FR2xx, FR4xx, FR5xx and FR6xx families via code generated by scripted datasheet parsing.


![Lib Logo](./doc/img/target_devboard.png)


```
 source:    https://github.com/rodan/atlas430
 author:    Petre Rodan <2b4eda@subdimension.ro>
 license:   BSD
```

### components

* TI MSP430 DriverLib
* HAL glue for
  * port init
  * system and auxiliary clocks
  * uart with optional interrupt driven tx
  * interrupt driven i2c
  * spi functions
  * interrupt-based event handling
  * scheduling (to be added soon)
* sample drivers
  * *Honeywell High Accuracy Ceramic* (HSC) and *Standard Accuracy Ceramic* (SSC) piezoresistive silicon pressure sensors
  * *Cypress FM24xxx* i2c FRAM IC
  * *Texas Intruments ADS1110* 16bit ADC
  * *Texas Instruments TCA6408* IO expander 
  * *Maxim DS3231* rtc IC
  * *Maxim DS3234* rtc IC
  * *Microchip MCP41xxx*, *MCP42xxx* Single/Dual Digital Potentiometer IC
  * *Texas Instruments PGA2311* stereo audio volume control IC
  * *Analog Devices AD7789* low power 16/24bit sigma-delta ADC IC
* Makefile-based build, tag file and document generation, code static scan
* shell scripts for checking the build environment, flashing uCs, version incrementation, RTC initialization
* a Makefile driven collection of unit tests for the drivers above

### software requirements

the entire development is done on a Gentoo Linux using makefiles, vim, the TI toolchain and gdb.
while none of those are a requirement to making the library part of any msp430 project, it happens to be the beaten path. the library can be used in the following ways:

#### via a special project makefile (in a linux environment)

see [this Makefile](https://github.com/rodan/sigdup/blob/master/firmware/Makefile) for the perfect example. **REFLIB_ROOT** defines the path to where this atlas430 repository has been cloned, **TARGET** represents the target microcontroller and [config.h](https://github.com/rodan/sigdup/blob/master/firmware/config.h) will be automatically expanded into compilation macros (-DFOO arguments to be sent to gcc). if **TARGET** is not hardcoded in the Makefile, then the user needs to provide it as an argument to make:

```
make TARGET=MSP430FXXXX
```

the makefile supports the following options:

```
# check if everything is installed and paths are correct
make envcheck

# remove the build/DEVICE/ directory
make TARGET=MSP430FXXXX clean

# compile the project and library
# create dependency tree and source code tags
make TARGET=MSP430FXXXX

# burn the firmware onto the target microcontroller
make TARGET=MSP430FXXXX install

# perform an optional static scan of the source code 
make TARGET=MSP430FXXXX cppcheck    # needs dev-util/cppcheck
make TARGET=MSP430FXXXX scan-build  # needs sys-devel/clang +static-analyzer
```

in order to use gdb to debug the project make sure to enable the **CONFIG_DEBUG** macro in config.h and run in a terminal

```
LD_PRELOAD='/opt/atlas430/lib/libmsp430.so' mspdebug --allow-fw-update tilib gdb
```

and then start gdb from within the project directory:

```
make TARGET=MSP430FXXXX debug
```

commonly used commands from within gdb provided as example for the unit tests:

```
target remote localhost:2000
monitor reset
monitor erase
load build/MSP430FXXXX/main.elf
b
disassemble
nexti
info registers
continue
tui enable
```

the included [Makefile.env](https://github.com/rodan/atlas430/blob/master/Makefile.env) contains the paths for the excelent [TI msp430 toolchain](https://www.ti.com/tool/MSP430-GCC-OPENSOURCE) which is a requirement in this scenario. a [shell script](./tools/check_setup.sh) for checking the build environment can be run on a non-priviledged account and will provide pointers of what packages are needed for building using this library. it also helps in installing the TI toolchain and support files.

all the [unit tests](https://github.com/rodan/atlas430/tree/master/tests) can be compiled with this method.

#### via *Code Composer Studio for Linux*

import [this project](https://github.com/rodan/sigdup/tree/master/firmware) as an example. the atlas430 repo is expected to be symlinked or cloned into */opt/atlas430/* while the compilation macros, the memory model, compile includes are all baked into the project's xml files - one needs to tweak them via *project properties* since this scenario is not using any of the provided Makefiles or scripts. compiling and debug is done via CCS.

#### via *Code Composer Studio for Windows*

theoretically like above, but I have never tested it.

### testing

HAL functions are expected to work on any of the supported microcontrollers and for each family there are a few devboards on which the code is manually tested. the current compatibility table can be consulted [here](doc/tests/README.md).

### references 

the following projects are already using this library:

project | microcontroller | short description
------- | --------------- | -----------------
[sigdup](https://github.com/rodan/sigdup) | msp430fr5994 | software that takes PulseView digital signal captures as input and re-plays them
[solar-charger rev4.1](https://github.com/rodan/solar-charger) | msp430f5510 | off-the-grid 1 cell lipo and 12v lead battery charger for low efficiency 15-36V photovoltaic cells
[ucurrent_ctrl](https://github.com/rodan/ucurrent_ctrl) | msp430f5510 | power controller for Dave L. Jones's µCurrent


