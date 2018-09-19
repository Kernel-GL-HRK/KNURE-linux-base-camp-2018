#!/bin/bash -e

export CROSS_COMPILE=arm-linux-gnueabi-

MODNAME="ssd1306.ko"

# parse commandline options
while [ ! -z "$1"  ] ; do
        case $1 in
           -h|--help)
                echo "TODO: help"
                ;;
            --clean)
                echo "Clean module sources"
                make ARCH=arm clean
                ;;
            --module)
                echo "Build module"
                make ARCH=arm
                ;;
            --deploy)
                echo "Deploy kernel module"
                cp $BUILD_KERNEL/arch/arm/boot/dts/sun8i-h3-orangepi-one.dts ${TRAINING_ROOT}
                scp $MODNAME root@192.168.10.2:~
                scp $BUILD_KERNEL/arch/arm/boot/dts/sun8i-h3-orangepi-one.dtb root@192.168.10.2:~
                ;;
            --kconfig)
                echo "configure kernel"
                make ARCH=arm config
                ;;
            
            --dtb)
                echo "configure kernel"
                make ARCH=arm dtb
                ;;
        esac
        shift
done

echo "Done!"
