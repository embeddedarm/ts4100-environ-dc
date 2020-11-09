# TS-4100 Environmental Monitor Daughter Card

## Application Information
### TS-4100
### DC interface
#### SPI LCD
#### I2C Sensor
#### PIR
### Software
#### SPI
#### I2C
#### ZPU

## Building
### Buildroot
### PCB
#### Ordering
#### Assembly
### Mounting Frame
#### Ordering
### Final Assembly
#### TS-4100 and TS-4100-ENVIRON-DC
#### PIR Sensor
#### Front Frame
#### Rear Frame

## Using Docker
### Notes on Using Docker


This repository contains the complete sources for our demo daughtercard for the TS-4100. These include PCB source files (KiCad format), mechanical frame/enclosure files (2D SVG), the main software that is run, and a complete Buildroot package.

The project is intended to showcase a using the TS-4100 daughtercard interface, using the TS-4100 ZPU (32-bit stack based microcontroller running inside of the FPGA), as well as using our [buildroot-ts.git](https://github.com/embeddedarm/buildroot-ts) repository to create complete and bootable images for our products using the Buildroot ecosystem.

See the sections below for detailed technical information.

See the [Buildroot](#Buildroot) section below for how to build this project.

## Application Information
### TS-4100
The TS-4100 differs from our SoM products in that it offers a 16-pin 0.1" spaced [pin header](https://docs.embeddedarm.com/TS-4100#HD1_Expansion_Header) as well as 2 Micro USB connectors on the PCB. Power input and serial console are available via USB, with the 16-pin header providing GPIO, an SPI interface, and an I2C interface. These, combined with the on-board WiFi and Bluetooth module, allow for integration of the TS-4100 in smaller form-factor applications.

### Daughtercard Interface
The TS-4100-ENVIRON-DC daughtercard uses: the I2C bus for connecting to an MS8607 temperature/pressure/relative humidity sensor, the SPI bus for connecting to our 128x64 monochrome LCD used on the TS-7553-V2, and 2 of the GPIO for an off the shelf passive infra-red (PIR) motion sensor output and enabling the LCD backlight.

#### SPI LCD
The SPI LCD is a 128x64 px monochrome LCD panel with backlight. It uses an SPI interface that is based on the ST7565P controller. The LCD is powered via 3.3 VDC output from the daughtercard interface. The LCD backlight is powered from the 5 VDC output from the daughtercard interface.

#### I2C Sensor
The I2C interface connects to a single MS8607 temperature/pressure/relative humidity sensor. This chip is actually two separate dies in a single package that appear at two separate I2C addresses. The I2C sensor is powered via 3.3 VDC output from the daughtercard interface.

#### PIR Motion Sensor
The LCD backlight is enabled when there is motion detected by a PIR sensor. The output signal from the PIR sensor is connected to a GPIO pin with the backlight enable connected to a different GPIO pin. The backlight enable signal can be controlled by the CPU, however, the intended operation is to use the motion sensor output run through the ZPU to control the backlight. See the [ZPU](#ZPU) section below for details of the operation. The PIR board is powered via 3.3 VDC output from the daughtercard interface.

### Software
The main application logic is handled by a single shell script. This script pulls local weather data from [wttr.in](https://wttr.in), and reads the local environment information via the I2C temperature/pressure/relative humidity sensor on the daughtercard. The shell script is named `env-collect.sh`.

The output of the collected data is formatted to fit on the screen (25x10 characters with the 4x5 pixel font used), and then the full text buffer is piped to the drawing tool `cairo-display-text`. This tool uses Pango, Cairo, and Fontconfig to take the text buffer and render it for the LCD screen. Formatting includes left shifting output from wttr.in to take up less space, as well as trimming any individual line down to 25 characters. This can result in text that is cut off on the right side of the screen, but most of the information is still viewable.

A customized pixel font was created to cleanly handle all of the UTF-8 glyphs that are used by the wttr.in service.

Buildroot is used to compile all of the utilities and provide a bootable environment. In order to accomplish this, there are a couple of clever applications of Buildroot and this repository.

First, this repository is a GNU/Linux source project using Autotools to configure the compilation of `lcd-helper` and `cairo-display-text` applications. The project is set up to also install the `env-collect.sh` script and ZPU firmware binary directly to the binary directory of the build process.

Second, this repository contains a Buildroot external tree. This is what provides the Buildroot configuration file, project specific Buildroot board files, as well as the Buildroot package to build this project's userspace software. The package `ts4100-environ-dc` is set up to use this repository to build. That is, the Buildroot configuration in this project defines this project as a package. The package is the compilation and installation of the binaries noted above.

Last, the Buildroot external configuration contains, as a submodule, our [buildroot-ts.git](https://github.com/embeddedarm/buildroot-ts) repository. The buildroot-ts.git repository is also set up as a Buildroot external tree. It provides our utilities packages in addition to base configuration files for Buildroot. Inside of that is upstream Buildroot included as a submodule. These two external trees nest together to isolate their specific implementations from each other and from Buildroot itself.

#### SPI LCD
The SPI LCD operation is similar to our TS-7553-V2. A virtual framebuffer device is created to represent the LCD and then a small daemon by the name of `lcd-helper` is started. This daemon is responsible for reading and writing between the virtual framebuffer and the actual LCD device. Applications write to the LCD by writing to the virtual framebuffer device. The actual application text is rendered to the virtual framebuffer via the `cairo-display-text` application. Any text that is given to this application via stdin is then rendered to the LCD with a border around it. This is then displayed on the LCD screen by the `lcd-helper` daemon.

#### I2C Sensor
The I2C temperature/pressure/relative humidity sensor is controlled by kernel drivers. The Buildroot external tree in this repository adds an FDT and kernel configuration fragment. This adds necessary driver as modules and tells the kernel how they are connected on the I2C bus. The kernel driver returns the temperature in milli-C, the pressure in kPa, and the relative humidity in milli-%. The main script simply reads these values from the kernel and normalizes their output to degrees F, inHg, and %RH respectively.

#### ZPU
The PIR sensor used has a relatively short pulse duration when it is activated. It can be increased at the expense of longer startup times. A 7 second output pulse duration incurs a startup time of roughly 4 minutes. In order to easily catch the PIR pulse output and be able to use this to enable the backlight for an arbitrary duration of time, the [ZPU inside of the TS-4100 FPGA](https://docs.embeddedarm.com/TS-4100#ZPU) was used. The ZPU application here is very straightforward, if the output from the PIR sensor goes high, turn on the backlight for ~10 seconds. If the PIR sensor re-triggers before the 10 second timeout, re-start the timer. This means the backlight will turn on if there is motion, and remain on until there is 10 seconds without motion.

## Building
### Buildroot
Buildroot will output a compressed tarball archive that is bootable on the TS-4100 eMMC or microSD card. The instructions below assume an SD card is used as there is some set up needed for U-Boot.

Optionally, Docker can be used to build this project, see [Using Docker](#Using-Docker) below.

* Clone the repository and its submodules
`git clone --recurse-submodules https://github.com/embeddedarm/ts4100-environ-dc`

* Update wpa_supplicant.conf with the WiFi network that the TS-4100 will connect to
`wpa_passphrase "your network SSID" "your network passphrase" > technologic/board/ts4100-environ/wpa_supplicant.conf`
Note that passphrase on the command line is optional, if not provided, it will be prompted for.

* Build the output image (this will take some time)
```
cd ts4100-environ-dc/buildroot/
make ts4100_environ_defconfig clean all
```

* Write the tarball to an SD card. Note that the default output file is `ts4100-environ-dc/buildroot/buildroot-ts/buildroot/output/images/rootfs.tar.xz` The file can be written to an SD card following the instructions in the [TS-4100 manual](https://docs.embeddedarm.com/TS-4100#microSD_Card) replacing the default tarball with the output of this project.

* Insert the SD card in the TS-4100, connect a USB cable to the P2 Micro USB connector and [open a terminal](https://docs.embeddedarm.com/TS-4100#Get_a_Console). Connect a second Micro USB cable to the P1 connector to power on the device. In the terminal, use `ctrl+c` to enter the U-Boot shell. Once in the U-Boot shell, run the following commands
```
env set force_jpsdboot 1
env set usbboot 0
env save
```
This will force the unit to boot from MicroSD and skip detection of USB devices in U-Boot

* The TS-4100 is now set up to boot directly to the MicroSD card that has been formatted with the full application.

### PCB
#### Ordering
The `kicad_pcb/` directory contains the PCB source files. We used a PCB fabricator that could directly accept KiCad board files without having to create Gerber files for them. This was to simplify the ordering process as much as possible. We recommend using a similar PCB fabricator.

Inside of this directory, a bill-of-matierals is available with recommended distributors. The only component that is not readily available from a distributor is the LCD screen itself. All other components are available from a number of US distributors.

#### Assembly
While nearly all of the components are SMD, they are large enough to be able to be hand soldered by individuals with a moderate skill in SMD PCB soldering. The PCB itself has not been designed for automated assembly and is simply a demo.

### Mounting Frame
#### Ordering

### Final Assembly
TODO
#### TS-4100 and TS-4100-ENVIRON-DC
#### PIR Sensor

#### PIR Sensor
The PIR sensor used in this project was set up with:
* A JST PH connector soldered to the back side of the PCB. Installing this on the top side will result in reverse current to the OpenPIR sensor.
* The OSC trimpot was rotated fully CCW (this controls the output pulse duration and startup time).
* The SEN trimpot is adjusted after fully installed to acheieve the best sensitivity for the complete device. This can be adjusted through the hole in the rear of the frame.
* The switch on the rear is set to "SNGL" to allow triggering on someone approaching OR retreating from the unit.

#### Front Frame
#### Rear Frame

## Using Docker
Optionally, Buildroot can be built in a Docker container. The container is maintained in lock-step with this project and the upstream Buildroot submodule. Meaning it is possible to go back to a specific commit in history and get a valid environment for building in via Docker.

The container is implemented as a simple front-end script, any arguments passed to the script will be passed directly to the `buildroot/` directory inside of the container. The first time the script is run, it will build the container so this may take additional time.

Building the Buildroot otuput image via Docker would use the following command
```
./scripts/run_docker_buildroot.sh make ts4100_environ_defconfig clean all
```

### Notes on using Docker

* Choose building either from the host workstation or Docker container, it is not recommended to mix and match. Do a `make clean` from one build system in order to be able to cleanly switch to another. Switching between the two without `make clean` in between will likely cause build issues
* The `pwd` is mapped to `/work/` inside the container, with `$HOME` being set to `/work/`. Any changes made inside of `/work/` will be retained, any changes to the rest of the container filesystem will be lost once the container is exited
* The ts4100_environ config has ccache enabled though Buildroot. Normally, this lies at `~/.buildroot-ccache`. Inside the container however, the `buildroot/` directory is set to `$HOME`. If relying on ccache in Buildroot, be sure to continually use the same build system to prevent excessive work
* It's possible to enter the shell of the container by passing `bash` to the script, i.e. `./scripts/run_docker_buildroot.sh bash`

## License
