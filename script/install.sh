# Install GCC
apt install gcc

# Install lib numa
apt install libnuma-dev

# Installer lstopo
apt install hwloc

# Create repertory for file
mkdir -p res

# Create a file of size 50Mo
dd if=/dev/zero of=res/file bs=1M count=50

# Create 1000 files of size 1Mo
for ((i=1; i<=1000; i++))
do
    dd if=/dev/zero of=res/file$i bs=1M count=1

done