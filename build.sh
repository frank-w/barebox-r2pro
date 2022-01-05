#!/bin/bash

export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

case $1 in
"importconfig")
	make rockchip_v8_defconfig
	;;
"config")
	make menuconfig
	;;
"build")
	make
	;;
"install")
	dev=/dev/sdb
	choice=y
	read -e -i "$dev" -p "Please enter target device: " dev
	if [[ $? -eq 0 ]];then
		sudo dd if=images/barebox-rk3568-evb.img of=$dev bs=1024 seek=32
	fi
	;;
*)
	echo "unknown command $1"
	;;
esac
