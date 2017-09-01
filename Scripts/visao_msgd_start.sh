#!/bin/sh

sleep .2
echo "Starting VisAOJoeCtrl"
./VisAOJoeCtrl -i ccd47 &
sleep .2

echo "Starting VisAO Filter Wheel 2"
./VisAOSimpleMotorCtrl -i filterwheel2 &
sleep .2

echo "Starting VisAO Filter Wheel 3"
./VisAOSimpleMotorCtrl -i filterwheel3 &
sleep .2

echo "Starting VisAO Shutter Remote Control"
./ShutterRemoteControl -i shutter &
sleep .2
