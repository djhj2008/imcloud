#!/bin/bash
basepath=$(cd `dirname $0`; pwd)
echo ${basepath}/libs > imcloud.conf
cp imcloud.conf /etc/ld.so.conf.d/
ldconfig
ldconfig -p
