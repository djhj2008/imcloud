#!/bin/bash
basepath=$(cd `dirname $0`; pwd)
echo ${basepath}/libs > imcloud.conf

systemctl stop imcloud
cp config/imcloud.json /etc/
cp imcloud /usr/local/bin/
cp imcloud.conf /etc/ld.so.conf.d/
ldconfig
ldconfig -p
cp imcloud.service /lib/systemd/system/
systemctl daemon-reload
systemctl enable imcloud
systemctl start imcloud
