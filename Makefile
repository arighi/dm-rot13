obj-m:= dm_rot13.o

dm_rot13.ko: dm_rot13.c
	make ARCH=um -C "/home/abhijit/play/kernel/uml-linux-2.6/" M=`pwd` modules

clean:
	make ARCH=um -C "/home/abhijit/play/kernel/uml-linux-2.6/" M=`pwd` clean
