CONFIG_MODULE_SIG=n

obj-m += rootkit.o

CFLAGS_rootkit.o := -DMALWARE_NAME=\"malware\"

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/ M=$(PWD) clean
