export CROSS_COMPILE=arm-linux-gnueabihf-
export ARCH=arm
export PATH=~/windows/arm-linux/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf/bin:$PATH
make clean
make
