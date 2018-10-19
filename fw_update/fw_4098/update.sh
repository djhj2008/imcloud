#!/bin/bash

mkdir temp
if [ -f "./zImage" ]
then
  mount /dev/mmcblk0p1 temp
  cp ./temp/zImage /zImage.bak
  cp zImage temp/
  umount temp
fi

