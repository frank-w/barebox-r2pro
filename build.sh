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
"dtsi")
	nano arch/arm/dts/rk3568.dtsi
	;;
"install")
	dev=/dev/sdb
	choice=y
	read -e -i "$dev" -p "Please enter target device: " dev
	if [[ $? -eq 0 ]];then
		sudo dd if=images/barebox-rk3568-bpi-r2pro.img of=$dev bs=1024 seek=32
	fi
	;;
"mount")
	mount | grep "BPI-BOOT" > /dev/null
	if [[ $? -ne 0 ]];then
		udisksctl mount -b /dev/disk/by-label/BPI-BOOT
	fi
	mount | grep "BPI-ROOT" > /dev/null
	if [[ $? -ne 0 ]];then
		udisksctl mount -b /dev/disk/by-label/BPI-ROOT
	fi
	;;

"umount")
	echo "umount SD Media"
	dev=$(mount | grep BPI-ROOT | sed -e 's/[0-9] .*$/?/')
	if [[ ! -z "$dev" ]];then
		umount $dev
	fi
	;;
"upload")
	scp images/barebox-rk3568-bpi-r2pro.img 192.168.0.10:/var/lib/tftp/
	;;
"")
	$0 build
	;;
*)
	echo "unknown command $1"
	;;
esac
