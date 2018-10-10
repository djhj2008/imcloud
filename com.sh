#!/bin/bash
IMCLOUD_VERSION="1.0.1.release"
basepath=$(cd `dirname $0`; pwd)
export PREFIX_CURL=${basepath}/libcurl
export PREFIX_HIREDIS=${basepath}/libhiredis
export PREFIX_JOSN=${basepath}/libjson
export PREFIX_PLE=${basepath}/plc_lib_intel
export PREFIX_OPENSSL=${basepath}/libssl-dev
export LIBS_INSTALL_PATH=$basepath/arm/libs
flag=0

if [ $1x == "clean"x ]
then
  echo "Clean Libs."
  rm -rf imcloud_${IMCLOUD_VERSION}.tar
  rm -rf ${PREFIX_CURL}
  rm -rf ${PREFIX_HIREDIS}
  rm -rf ${PREFIX_JOSN}
  rm -rf ${PREFIX_OPENSSL}
  rm -rf ${LIBS_INSTALL_PATH}/*.so
  cd ${basepath}/curl
  make clean
  cd ${basepath}/hiredis
  make clean
  cd ${basepath}/json-c
  make clean
  cd ${basepath}/plc_lib_intel
  make clean
  cd ${basepath}/openssl-1.0.2g
  make clean
  cd ${basepath}/arm
  make clean
  exit
fi

if [ $1x == "new"x ]
then
  rm -rf ${PREFIX_CURL}
  rm -rf ${PREFIX_HIREDIS}
  rm -rf ${PREFIX_JOSN}
  rm -rf ${PREFIX_OPENSSL}
  rm -rf ${LIBS_INSTALL_PATH}/*.so
  flag=1
fi

if [ $1x == "r"x ]
then
  flag=1
fi

if [ $flag != 1 ]
then
  echo $1 error.
  echo "Usage: r/new/clean/"
  exit
fi

export CROSS_COMPILE=arm-linux-gnueabihf-
export ARCH=arm
export PATH=~/windows/arm-linux/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf/bin:$PATH

#curl
cd ${basepath}/curl
#./configure --prefix=${PREFIX_CURL} --host=arm-linux CC=${CROSS_COMPILE}gcc CXX=${CROSS_COMPILE}g++
rm -rf ${PREFIX_CURL}
if [ $1 == "new" ]
then
  ./configure --prefix=${PREFIX_CURL} --host=arm-linux CC=${CROSS_COMPILE}gcc CXX=${CROSS_COMPILE}g++
  make clean
fi
make
make install
cp ${basepath}/libcurl/lib/*.so ${LIBS_INSTALL_PATH}

#hiredis
cd ${basepath}/hiredis
export PREFIX_HIREDIS=${basepath}/libhiredis
if [ $1 == "new" ]
then
  make clean
fi
make
make install
cp ${PREFIX_HIREDIS}/lib/*.so ${LIBS_INSTALL_PATH}


#json-c
cd ${basepath}/json-c
export PREFIX_JOSN=${basepath}/libjson
#./configure --prefix=${PREFIX_JOSN} --host=arm-linux-gnueabihf --build=i686-pc-linux
if [ $1 == "new" ]
then
  ./configure --prefix=${PREFIX_JOSN} --host=arm-linux-gnueabihf --build=i686-pc-linux
  make clean
fi
make
make install
cp ${PREFIX_JOSN}/lib/*.so ${LIBS_INSTALL_PATH}

#plc_lib_intel
cd ${basepath}/plc_lib_intel
export PREFIX_PLE=${basepath}/plc_lib_intel
if [ $1 == "new" ]
then
  make clean
fi
make
cp ${PREFIX_PLE}/*.so ${LIBS_INSTALL_PATH}

#openssl-1.0.2g
cd ${basepath}/openssl-1.0.2g
export PREFIX_OPENSSL=${basepath}/libssl-dev
if [ $1 == "new" ]
then
  make clean
fi
make
make install
cp ${PREFIX_OPENSSL}/lib/*.so ${LIBS_INSTALL_PATH}

#main
cd ${basepath}/arm
make clean
make
mkdir imcloud_${IMCLOUD_VERSION}
cp -r ./imcloud ./libs/ imcloud.service install.sh ReadMe ./config/ imcloud_${IMCLOUD_VERSION}
tar -cvf imcloud_${IMCLOUD_VERSION}.tar imcloud_${IMCLOUD_VERSION}
mv imcloud_${IMCLOUD_VERSION}.tar ../
rm -rf ./imcloud_${IMCLOUD_VERSION}
echo "End"


