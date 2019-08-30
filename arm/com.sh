export CROSS_COMPILE=arm-linux-gnueabihf-
export ARCH=arm
export PATH=~/windows/arm-linux/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf/bin:$PATH
FW_VERSION=true
export VERSION_MAJOR=1
export VERSION_MINOR=0
export VERSION_REVISION=0
export VERSION_HOST=1
export BUILD_DATE=$(date "+%y%m%d%k%M")
make clean
make
