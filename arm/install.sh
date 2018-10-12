#!/bin/bash
#basepath=$(cd `dirname $0`; pwd)

cp -r ./imlibs /usr/local/lib/
echo '/usr/local/lib/imlibs' > imcloud.conf

systemctl stop imcloud
mkdir /etc/imcloud
cp config/* /etc/imcloud/
cp imcloud /usr/local/bin/
chmod 755 /usr/local/bin/imcloud
cp imcloud.conf /etc/ld.so.conf.d/
ldconfig
ldconfig -p
cp imcloud.service /lib/systemd/system/
systemctl daemon-reload
systemctl enable imcloud
systemctl start imcloud
