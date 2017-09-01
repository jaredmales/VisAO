#!/bin/sh
sleep .2
pkill dioserver
sleep .2
echo $(ps -A | grep dioserver)
pkill ShutterControl
sleep .2
echo $(ps -A | grep ShutterControl)
pkill ShutterTester
sleep .2
echo $(ps -A | grep ShutterTester)
pkill FocusMotorCtrl
sleep .2
echo $(ps -A | grep FocusMotorCtrl)
pkill WollastonStatus
sleep .2
echo $(ps -A | grep WollastonStatus)
pkill framegrabber47
sleep .2
echo $(ps -A | grep framegrabber47)
pkill framewriter
sleep .2
echo $(ps -A | grep framewriter)
pkill frameserver
sleep .2
echo $(ps -A | grep frameserver)
pkill CCD47Ctrl
sleep .2
echo $(ps -A | grep CCD47Ctrl)
pkill sysmonD
sleep .2
echo $(ps -A | grep sysmonD)

