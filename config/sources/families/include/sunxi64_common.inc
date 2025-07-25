#
# SPDX-License-Identifier: GPL-2.0
#
# Copyright (c) 2013-2023 Igor Pecovnik, igor@armbian.com
#
# This file is a part of the Armbian Build Framework
# https://github.com/armbian/build/
#
enable_extension "sunxi-tools"
declare -g ARCH=arm64
declare -g ATF_TARGET_MAP="PLAT=$ATF_PLAT DEBUG=1 bl31;;build/$ATF_PLAT/debug/bl31.bin"
declare -g ATFBRANCH="tag:lts-v2.12.1"
declare -g BOOTDELAY=1
declare -g BOOTPATCHDIR="${BOOTPATCHDIR:-"u-boot-sunxi"}"
declare -g BOOTBRANCH="${BOOTBRANCH:-"tag:v2024.01"}"
declare -g BOOTENV_FILE='sunxi.txt'
declare -g UBOOT_TARGET_MAP="${UBOOT_TARGET_MAP:-BINMAN_ALLOW_MISSING=1;;u-boot-sunxi-with-spl.bin}"
declare -g BOOTSCRIPT='boot-sun50i-next.cmd:boot.cmd'
declare -g OVERLAY_DIR="/boot/dtb/allwinner/overlay"
declare -g LINUXFAMILY=sunxi64

# shellcheck source=config/sources/families/include/crust_firmware.inc
source "${BASH_SOURCE%/*}/crust_firmware.inc"

case $BRANCH in

	legacy)
		declare -g KERNEL_MAJOR_MINOR="6.6" # Major and minor versions of this kernel.
		declare -g KERNELBRANCH="tag:v6.6.75" # frozen on this version
		;;

	current)
		declare -g KERNEL_MAJOR_MINOR="6.12" # Major and minor versions of this kernel.
		declare -g KERNELBRANCH="tag:v6.12.30"
		;;

	edge)
		declare -g KERNEL_MAJOR_MINOR="6.15" # Major and minor versions of this kernel.
		declare -g KERNELBRANCH="tag:v6.15"
		;;
esac

KERNELPATCHDIR="archive/sunxi-${KERNEL_MAJOR_MINOR}"

family_tweaks() {
	# execute specific tweaks function if present
	[[ $(type -t family_tweaks_s) == function ]] && family_tweaks_s
	cp $SRC/packages/blobs/splash/armbian-u-boot-24.bmp $SDCARD/boot/boot.bmp
}

write_uboot_platform() {
	dd if=/dev/zero of=$2 bs=1k count=1023 seek=1 status=noxfer > /dev/null 2>&1
	dd if=$1/u-boot-sunxi-with-spl.bin of=$2 bs=1024 seek=8 status=noxfer > /dev/null 2>&1
}

setup_write_uboot_platform() {
	local tmp part dev
	if grep -q "ubootpart" /proc/cmdline; then
		# mainline with new boot script
		tmp=$(cat /proc/cmdline)
		tmp="${tmp##*ubootpart=}"
		tmp="${tmp%% *}"
		[[ -n $tmp ]] && part=$(findfs PARTUUID=$tmp 2> /dev/null)
		[[ -n $part ]] && dev=$(lsblk -n -o PKNAME $part 2> /dev/null)
		[[ -n $dev ]] && DEVICE="/dev/$dev"
	else
		# legacy or old boot script
		tmp=$(cat /proc/cmdline)
		tmp="${tmp##*root=}"
		tmp="${tmp%% *}"
		[[ -n $tmp ]] && part=$(findfs $tmp 2> /dev/null)
		[[ -n $part ]] && dev=$(lsblk -n -o PKNAME $part 2> /dev/null)
		# do not try to write u-boot to USB devices
		[[ -n $dev && $dev == mmcblk* ]] && DEVICE="/dev/$dev"
	fi
}
