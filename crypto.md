Prepare qemu code

	git checkout 59e1b8a22e

Compile qemu and run

	./configure --target-list=x86_64-softmmu \
		--extra-ldflags="`pkg-config --libs openssl`"


cd fabvm
sudo ~/src/qemu/build/x86_64-softmmu/qemu-system-x86_64 \
	-drive file=~/golden/cld.base -smp 2 -m 1024 -enable-kvm \
	-device pci-crypto,aes_cbc_256="abc" \
	-netdev tap,id=nd0,ifname=tap0,script=./nat_up.py,downscript=./nat_down.py \
	-device e1000,netdev=nd0,mac=52:54:00:12:34:27

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

或者使用lspci查看(下面的0xfebf1000就是bar0地址, 即mmio的地址)
lspci -vvvv -xxxx -s 00:03.0
Region 0: Memory at febf1000 (32-bit, non-prefetchable) [size=4K]

或者通过qemu monitor查看也能查看到该地址
(qemu) info qtree
(qemu) info mtree

下面命令会调用到mmio的read函数
./devmem 0xfebf1000 b
./devmem 0xfebf1000 w
./devmem 0xfebf1000 l

写mmio空间的第二个地址0xfebf1000 + 2,命令是1:对应的是reset
./devmem 0xfebf1002 w 1

Encrypt:命令是2
./devmem 0xfebf1002 w 2

Decrypt:命令是3
./devmem 0xfebf1002 w 3
