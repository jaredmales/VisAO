#!/bin/sh

sleep .2
echo "Stoping VisAOJoeCtrl"
pkill VisAOJoeCtrl
sleep .2
kill -s 9 $(ps -Ac | grep  VisAOJoeCtrl | awk '{print $1}')

echo "Stoping VisAO Filter Wheel 2"
pkill VisAOSimpleMotorCtrl
sleep .2
kill -s 9 $(ps -Ac | grep  VisAOSimpleMot | awk '{print $1}')

echo "Stoping VisAO Filter Wheel 3"
pkill VisAOSimpleMotorCtrl
sleep .2
kill -s 9 $(ps -Ac | grep  VisAOSimpleMot | awk '{print $1}')

echo "Stoping VisAO Shutter Remote Control"
pkill ShutterRemoteControl
sleep .2
kill -s 9 $(ps -Ac | grep  ShutterRemote | awk '{print $1}')
