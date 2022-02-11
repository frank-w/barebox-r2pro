#!/bin/bash

export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

case $1 in
"importconfig")
	make rk3568_defconfig
	;;
"defconfig")
	nano arch/arm/configs/rk3568_defconfig
	;;
"config")
	make menuconfig
	;;
"build")
	make
	;;
"board")
	nano arch/arm/boards/rockchip-rk3568-bpi-r2pro/board.c
	;;
"dts")
	nano arch/arm/dts/rk3568-bpi-r2-pro.dts
	;;
"install")
	dev=/dev/sdb
	choice=y
	read -e -i "$dev" -p "Please enter target device: " dev
	if [[ $? -eq 0 ]];then
		sudo dd if=images/barebox-rk3568-bpi-r2pro.img of=$dev bs=1024 seek=32
	fi
	;;
"upload")
	scp images/barebox-rk3568-bpi-r2pro.img 192.168.0.10:/var/lib/tftp/
	;;
*)
	echo "unknown command $1"
	;;
esac
