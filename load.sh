#!/bin/sh

sudo insmod ourdevice.ko
sudo mknod /dev/ourdevice c 242 0
sudo chmod 777 /dev/ourdevice
