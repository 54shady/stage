Prepare qemu code

	git checkout 59e1b8a22e

Compile qemu and run

	./configure --target-list=x86_64-softmmu \
		--extra-ldflags="`pkg-config --libs openssl`"

	~/src/qemu/build/x86_64-softmmu/qemu-system-x86_64 \
		-drive file=crypto.qcow2 -smp 2 -m 1024 -enable-kvm \
		-device pci-crypto,aes_cbc_256="abc"

查看pci设备寄存器(通用)
setpci --dumpregs

00 W VENDOR_ID
02 W DEVICE_ID
04 W COMMAND
06 W STATUS
08 B REVISION
09 B CLASS_PROG
...

用lspci -n 找到对应设备的BDF值
比如00:03.0这个设备

查看设备对应寄存器的值

比如查看vendor id
setpci -s 00:03.0 00.w

比如查看device id
setpci -s 00:03.0 02.w

还可以使用devmem(https://github.com/VCTLabs/devmem2)工具来查看

查看设备的bar0的地址
setpci -s 00:03.0 10.l

或者使用lspci查看(下面的0xfb71000就是bar0地址)
lspci -vvvv -xxxx -s 00:03.0
Region 0: Memory at feb71000 (32-bit, non-prefetchable) [size=4K]

下面命令会调用到mmio的read函数
./devmem 0xfeb71000 b
./devmem 0xfeb71000 w
./devmem 0xfeb71000 l
