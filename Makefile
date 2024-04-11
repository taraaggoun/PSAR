# Path to kernek
KERNELDIR_LKP ?= /home/tara/linux-6.5.7

MODULE_1=print_node_file
MODULE_2=print_node_pid
obj-m += $(MODULE_1).o
obj-m += $(MODULE_2).o 

# Path to share
SHARE_PATH:=/home/tara/master/S2/PNL/share

all : print_node_file print_node_pid benchmark_file benchmark_pid map_file

print_node_file:
	make -C $(KERNELDIR_LKP) M=$$PWD modules
	cp $(MODULE_1).ko $(SHARE_PATH)/$(MODULE_1).ko

print_node_pid:
	make -C $(KERNELDIR_LKP) M=$$PWD modules
	cp $(MODULE_2).ko $(SHARE_PATH)/$(MODULE_2).ko

benchmark_file:
	gcc benchmark_file.c -D_GNU_SOURCE -static -o $(SHARE_PATH)/benchmark_file

benchmark_pid:
	gcc benchmark_pid.c -D_GNU_SOURCE -static -o $(SHARE_PATH)/benchmark_pid

map_file:
	gcc map_file.c -D_GNU_SOURCE -static -o $(SHARE_PATH)/map_file

clean:
	make -C $(KERNELDIR_LKP) M=$$PWD clean
