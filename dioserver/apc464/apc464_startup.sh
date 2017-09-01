#!/bin/sh

mknod /dev/apmc464_0 c 46 0
chmod a+w /dev/apmc464_0
/sbin/insmod /home/pmc/pmc464/dev464/apmc464.ko

