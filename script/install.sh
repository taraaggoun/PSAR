# Install GCC
apt install gcc

# Install lib numa
apt install libnuma-dev

# Installer lstopo
apt install hwloc

# Disable Numa Balencing
echo "0" > /proc/sys/kernel/numa_balancing

# Create repertory for file
mkdir -p res

# Create 1000 files of size 50Mo
for ((i=1; i<=1000; i++))
do
    dd if=/dev/zero of=res/file$i bs=1M count=50

done