#!/bin/bash
:<<!
******NOTICE******
MUST set SDK_PATH & BIN_PATH firstly!!!
example:
export SDK_PATH=/mnt/hgfs/share/ESP8266/ESP8266_RTOS_SDK-2.0.0/
export BIN_PATH=/mnt/hgfs/share/ESP8266/ESP8266_RTOS_SDK-2.0.0/bin
!

echo "based on gen_misc.sh version 20150911"
echo ""

if [ ! $SDK_PATH ]; then
    export SDK_PATH=$(dirname $(dirname $(pwd)))
fi
echo "SDK_PATH:"
echo "$SDK_PATH"
echo ""

if [ ! $BIN_PATH ]; then
    export BIN_PATH=$SDK_PATH/bin
fi


boot=new

echo "boot mode: $boot"
echo ""

echo "STEP 2: choose bin generate(0=eagle.flash.bin+eagle.irom0text.bin, 1=user1.bin, 2=user2.bin)"
echo "enter (0/1/2, default 0):"
read input

if [ -z "$input" ]; then
    if [ $boot != none ]; then
    	boot=none
	echo "ignore boot"
    fi
    app=0
    echo "generate bin: eagle.flash.bin+eagle.irom0text.bin"
elif [ $input == 1 ]; then
    if [ $boot == none ]; then
    	app=0
	echo "choose no boot before"
	echo "generate bin: eagle.flash.bin+eagle.irom0text.bin"
    else
	app=1
        echo "generate bin: user1.bin"
    fi
elif [ $input == 2 ]; then
    if [ $boot == none ]; then
    	app=0
	echo "choose no boot before"
	echo "generate bin: eagle.flash.bin+eagle.irom0text.bin"
    else
    	app=2
    	echo "generate bin: user2.bin"
    fi
else
    if [ $boot != none ]; then
    	boot=none
	echo "ignore boot"
    fi
    app=0
    echo "generate bin: eagle.flash.bin+eagle.irom0text.bin"
fi


spi_speed=40

spi_mode=QIO

spi_size_map=6


echo "spi mode: $spi_mode"
echo ""



echo "start..."
date
echo ""

make clean

make BOOT=$boot APP=$app SPI_SPEED=$spi_speed SPI_MODE=$spi_mode SPI_SIZE_MAP=$spi_size_map

date
ls -l $BIN_PATH/eagle.[if]*
echo ../../../esptool/esptool.py --baud 230400 -p /dev/cu.usbserial-* write_flash 0x00000 $BIN_PATH/eagle.flash.bin 0x14000 $BIN_PATH/eagle.irom0text.bin
