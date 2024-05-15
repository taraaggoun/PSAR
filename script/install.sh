# Install GCC
apt install gcc

# Install lib numa
apt install libnuma-dev

# Installer lstopo
apt install hwloc

# Disable Numa Balencing
echo "0" > proc/sys/kernel/numa_balancing

# Create file of size 50Mo
dd if=/dev/zero of=file bs=1M count=50