#!/bin/bash

/etc/init.d/cups stop
#ifconfig eth0 12.12.12.12 
arp -s 12.12.12.13 00:f8:c1:d8:c7:a6
sudo route add 12.12.12.13 dev eth0
