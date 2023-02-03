qemu中实现一个虚拟设备(qemu/hw/misc/newdev.c) 作为TCP server
	该虚拟设备会创建本地socket用于给host和guest进行通信

其对应的驱动是eBPF-injection/shared/driver/driver.c(安装在guest系统中)
guest中运行应用程序eBPF-injection/shared/daemon_bpf/daemon_bpf.c来使用这个虚拟设备
	应用程序通过ioctl来操作虚拟设备驱动
		daemon_bpf---ioctl--->driver

	虚拟设备驱动通过io操作来访问虚拟设备,虚拟设备中执行最终操作
		driver---iowrite--->device

host中运行eBPF-injection/host_interface/injectProgram.c(TCP client)向服务端发送数据(eBPF-injection/bpfProg/myprog.c)

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

make all -j 9 CONFIG_NEWDEV=y CONFIG_VIRTFS=y CONFIG_VIRTIO_9P=y
