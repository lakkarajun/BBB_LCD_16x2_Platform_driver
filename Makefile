obj-m := bone_lcd.o
bone_lcd-objs = lcd_platform_driver.o
bone_lcd-objs += lcd_api.o
bone_lcd-objs += lcd_gpio.o
bone_lcd-objs += lcd_api.o
ARCH=arm
CROSS_COMPILE=/home/raju/projects/beaglebone/buildroot/output/host/bin/arm-buildroot-linux-gnueabihf-
KERN_DIR = /home/raju/projects/beaglebone/buildroot/output/build/linux-6.15.5/
HOST_KERN_DIR = /lib/modules/6.14.0-1014-oem/build/

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) modules
clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) clean 
help: 
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) help 
host:
	make -C $(HOST_KERN_DIR) M=$(PWD) modules

copy-dt:
	scp /home/raju/projects/beaglebone/linux-6.15.5/arch/arm/boot/dts/ti/omap/am335x-boneblack.dtb root@192.168.29.166:/root/drivers/.
copy-drv:
	scp *.ko root@192.168.29.166:/root/drivers/.


