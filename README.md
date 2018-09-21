# Doodle-Jump-Wii
A Doodle Jump clone for the Nintendo Wii

## Installation (assuming you have [Homebrew](http://wiibrew.org/wiki/Homebrew_Channel))
* Download the latest release from [here](https://github.com/JorelAli/Doodle-Jump-Wii/releases/latest)
* Unzip the file and copy the __doodlejumpwii__ folder into your /apps/ directory on your SD card

## Screenshots
Alpha test:

![img](https://raw.githubusercontent.com/JorelAli/Doodle-Jump-Wii/master/screenshots/alpha%20test.png)

Main menu:

![img](https://raw.githubusercontent.com/JorelAli/Doodle-Jump-Wii/master/screenshots/main%20menu.png)

## Building (complete beginner's guide [Windows])
* Install devkitPPC:
  * Download the automatic installer from [sourceforge](https://sourceforge.net/projects/devkitpro/files/Automated%20Installer/) (I used version 2.2.1)
  * Follow the installer through. When given the choice for components, untick devkitARM and devkitA64 (you don't need these)
  * Make sure you install it in its default location (`C:/devkitPro`)
* Install GRRLIB:
  * Download the latest release of GRRLIB from [here](http://grrlib.santo.fr/wiki/wikka.php?wakka=HomePage) (I used version 4.3.2)
  * Copy the GRRLIB folder into `C:/devkitPro`
  * Open up a terminal in `C:/devkitPro/GRRLIB` and run `make`
  * When that is complete, go to `C:/devkitPro/GRRLIB/lib` and run `make`
* Setup your projects folder:
  * Create a folder `C:/projects`
  * Copy the contents of this repo into a folder (`C:/projects/doodlejump`) (the Makefile should be in `C:/projects/doodlejump/Makefile`)
* Building:
  * Run `make` inside `C:/projects/doodlejump`

## Creating images for /source/gfx
* Ensure that your dimensions of your image file is a multiple of 4 (for example, 640x480 or 32x32)
* Download [WiiBuilder](http://wiibrew.org/wiki/WiiBuilder)
* In the WiiBuilder settings, ensure __Binary File Conversion__ is set to __C and Header File__ (and save this to registry if desired)
* Drag and drop the image file into the __File__ area in WiiBuilder
