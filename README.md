qemu中实现一个虚拟设备(qemu/hw/misc/newdev.c) 作为TCP server
	该虚拟设备会创建本地socket用于给host和guest进行通信

其对应的驱动是eBPF-injection/shared/driver/driver.c(安装在guest系统中)
guest中运行应用程序 eBPF-injection/shared/daemon_bpf/daemon_bpf.c 来使用这个虚拟设备
	应用程序通过ioctl来操作虚拟设备驱动
		daemon_bpf---ioctl--->driver

	虚拟设备驱动通过io操作来访问虚拟设备,虚拟设备中执行最终操作
		driver---iowrite--->device

host中运行 eBPF-injection/host_interface/injectProgram.c (TCP client)向服务端发送数据(eBPF-injection/bpfProg/myprog.c)

bpf docker: /home/zeroway/github/kernel_drivers_examples/x86/bpf
如何编译myproc.c(使用bpf docker环境和对应的内核代码,比如ubuntu 22.04)

	cd /home/zeroway/github/hyperupcall/eBPF-injection/bpfProg
	drun -v $PWD:/code -v /home/zeroway/Misc/dres/linux-5.15.0:/usr/src/linux bpf /bin/bash
	cp myprog.c /usr/src/linux/samples/bpf/
	make -C /usr/src/linux M=samples/bpf

Config qemu

./configure \
	--target-list="x86_64-softmmu" \
	--enable-debug \
	--disable-docs \
	--disable-capstone \
	--disable-nettle \
	--disable-gnutls \
	--disable-gcrypt \
	--extra-cflags="-O0" \
	--enable-trace-backends=ftrace

Comile qemu

make all -j 9 CONFIG_NEWDEV=y CONFIG_VIRTFS=y CONFIG_VIRTIO_9P=y

Run qemu

/root/qemu/x86_64-softmmu/qemu-system-x86_64 \
        -serial mon:stdio \
        -drive file=/root/eBPF-injection/ubt2204.qcow2,format=qcow2 \
        -enable-kvm -m 2G -smp 2 \
        -device virtio-net-pci,netdev=ssh \
        -netdev user,id=ssh,hostfwd=tcp::2222-:22 \
		-vnc 0.0.0.0:0 \
        -device newdev

test working flow:

wrapper-test.py
	test.py
		injectProgram.sh(run injectProgram)
			将bpfProg编译产生的文件通过网络发送到虚拟设备的缓存中

在guest中运行了守护程序daemon_bpf来读取虚拟设备的缓存
1. 将主机发送过来的bpfProg程序保存到本地并加载运行
2. 设置好cpu affinity后调用 ioctl(fd, IOCTL_SCHED_SETAFFINITY) 来设置cpu亲和性
		iowrite32(requested_cpu, bufmmio + NEWDEV_REG_SETAFFINITY); //eBPF-injection/shared/driver/driver.c
			newdev_bufmmio_write
				sched_setaffinity//qemu/hw/misc/newdev.c

------------
使用ubuntu20.04 docker编译 daemon_bpf:

	drun -v $PWD:/code -v /home/zeroway/Misc/dres/linux-5.4.0:/usr/src/linux bpf2004 /bin/bash

修改内核linux-5.4.0/samples/bpf/Makefile

	hostprogs-y += hbm
	+hostprogs-y += daemon_bpf

	sockex3-objs := bpf_load.o sockex3_user.o
	+daemon_bpf-objs := bpf_load.o daemon_bpf.o

------------

bpfProg中在guest内核的sched_setaffinity处

Guest ubunt 22.04

	apt install -y openssh-server make gcc
