# Path to kernel
KERNELDIR_LKP ?= /home/tara/linux-6.5.7

# Repository
BENCHMARK=benchmark
MODULES=modules

# Modules add to kernel modules
MODULE_1=print_node_file
MODULE_2=print_node_pid
MODULE_3=openctl
obj-m += $(MODULES)/$(MODULE_1).o
obj-m += $(MODULES)/$(MODULE_2).o 
obj-m += $(MODULES)/$(MODULE_3).o 


# Shared path
SHARE_PATH:=/home/tara/master/S2/PNL/share

all: print_node_file print_node_pid openctl benchmark_file benchmark_pid map_file add_open_pid_tab

# Modules
print_node_file:
	make -C $(KERNELDIR_LKP) M=$(PWD) modules
	cp $(MODULES)/$(MODULE_1).ko $(SHARE_PATH)/$(MODULE_1).ko

print_node_pid:
	make -C $(KERNELDIR_LKP) M=$(PWD) modules
	cp $(MODULES)/$(MODULE_2).ko $(SHARE_PATH)/$(MODULE_2).ko

openctl:
	make -C $(KERNELDIR_LKP) M=$(PWD) modules
	cp $(MODULES)/$(MODULE_3).ko $(SHARE_PATH)/$(MODULE_3).ko

# Programmes C
benchmark_file:
	gcc $(BENCHMARK)/benchmark_file.c -D_GNU_SOURCE -static -o $(SHARE_PATH)/benchmark_file

benchmark_pid:
	gcc $(BENCHMARK)/benchmark_pid.c -D_GNU_SOURCE -static -o $(SHARE_PATH)/benchmark_pid

map_file:
	gcc $(BENCHMARK)/map_file.c -D_GNU_SOURCE -static -o $(SHARE_PATH)/map_file

add_open_pid_tab:
	gcc $(BENCHMARK)/add_open_pid_tab.c -D_GNU_SOURCE -static -o $(SHARE_PATH)/add_open_pid_tab

clean:
	make -C $(KERNELDIR_LKP) M=$(PWD) clean
	rm -f $(SHARE_PATH)/$(MODULE_1).ko
	rm -f $(SHARE_PATH)/$(MODULE_2).ko
	rm -f $(SHARE_PATH)/$(MODULE_3).ko
	rm -f $(SHARE_PATH)/benchmark_file
	rm -f $(SHARE_PATH)/benchmark_pid
	rm -f $(SHARE_PATH)/map_file
	rm -f $(SHARE_PATH)/add_open_pid_tab
	rm -f $(MODULES)/*.mod.c $(MODULES)/*.mod $(MODULES)/*.o $(MODULES)/*.ko $(MODULES)/.*.cmd .*.cmd modules.order
	rm -rf .tmp_versions

