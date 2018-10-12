#!/bin/bash
systemctl stop fw_update
cp fw_update.sh /usr/local/bin/
cp fw_update.service /lib/systemd/system/
systemctl daemon-reload
systemctl enable fw_update
systemctl start fw_update
