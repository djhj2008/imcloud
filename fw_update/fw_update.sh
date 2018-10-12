#!/bin/bash
flag_reboot=0

cd /fw_update
file=`find -name fw_*.tar`
if [[ $file == "" ]]
then
  echo "Do nothing."
  exit
fi

echo "fw file:"${file}
tar xvf ${file}

path=${file%.tar}
echo "path:"${path}
cd ${path}


echo "update kernel start"
image=`find -name update.sh`
if [[ $image == "" ]]
then
  echo "no kernel"
else
  chmod 777 $image
  $image
  flag_reboot=1
fi

echo "update app start"
appfile=`find -name imcloud*.tar`
if [[ $appfile == "" ]]
then
  echo "no app"
else
  echo "app file:"${appfile}
  tar xvf ${appfile}
  apppath=${appfile%.tar}
  echo "app path:"${apppath}
  cd ${apppath}
  ./install.sh
fi

cd /fw_update
ret=$(ps -A | grep imcloud)
echo ${ret}

if [[ $ret == "" ]]
then
  echo "FAIL"
  exit
else
  rm -rf ${path}
  rm -rf ${file}
  echo "OK"
fi

if [ $flag_reboot == 1 ]
then
  reboot
fi



