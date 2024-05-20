# Path to kernel
KERNELDIR_LKP ?= /home/tara/linux-6.5.7

# Repository
BENCHMARK=benchmark
MODULES=modules
IOCTL=ioctl

# Modules add to kernel modules
MODULE_1=print_node_file
MODULE_2=print_node_pid
MODULE_3=openctl
obj-m += $(MODULES)/$(MODULE_1).o
obj-m += $(MODULES)/$(MODULE_2).o 
obj-m += $(IOCTL)/$(MODULE_3).o 


all: print_node_file print_node_pid openctl benchmark_file benchmark_pid map_file add_open_pid_tab

# Modules
print_node_file:
	make -C $(KERNELDIR_LKP) M=$(PWD) modules
	cp $(MODULES)/$(MODULE_1).ko $(MODULE_1).ko

print_node_pid:
	make -C $(KERNELDIR_LKP) M=$(PWD) modules
	cp $(MODULES)/$(MODULE_2).ko $(MODULE_2).ko

openctl:
	make -C $(KERNELDIR_LKP) M=$(PWD) modules
	cp $(IOCTL)/$(MODULE_3).ko $(MODULE_3).ko

# Programmes C
benchmark_file:
	gcc $(BENCHMARK)/benchmark_file.c -D_GNU_SOURCE -static -o benchmark_file

benchmark_pid:
	gcc $(BENCHMARK)/benchmark_pid.c -D_GNU_SOURCE -static -o benchmark_pid

benchmark_pid:
	gcc $(BENCHMARK)/benchmark.c -D_GNU_SOURCE -static -o benchmark

clean:
	make -C $(KERNELDIR_LKP) M=$(PWD) clean
	rm -f $(MODULE_1).ko
	rm -f $(MODULE_2).ko
	# rm -f $(MODULE_3).ko
	rm -f benchmark_file
	rm -f benchmark_pid
	rm -f benchmark
	rm -f $(MODULES)/*.mod.c $(MODULES)/*.mod $(MODULES)/*.o $(MODULES)/*.ko $(MODULES)/.*.cmd .*.cmd modules.order
	rm -f $(IOCTL)/*.mod.c $(IOCTL)/*.mod $(IOCTL)/*.o $(IOCTL)/*.ko $(IOCTL)/.*.cmd
	rm -rf .tmp_versions

