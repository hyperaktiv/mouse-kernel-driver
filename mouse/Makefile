KDIR = /lib/modules/`uname -r`/build

all:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean
test:test.c
	cc -o test test.c