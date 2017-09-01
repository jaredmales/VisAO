#!/bin/sh
sleep .2
./dioserver -f conf/dioserver.conf &
sleep .2
./ShutterControl -f conf/ShutterControl.conf &
sleep .2
./sims/ShutterTester  &
sleep .2
./FocusMotorCtrl -f conf/FocusMotorCtrl.conf &
sleep .2
./WollastonStatus -f conf/WollastonStatus.conf &
sleep .2
./framegrabber47 -f conf/framegrabber47.conf &
sleep .2
./framewriter -f conf/framewriter47.conf &
sleep .2
./frameserver -f conf/frameserver47.conf > /dev/null &
sleep .2
./CCD47Ctrl -f conf/CCD47Ctrl.conf &
sleep .2
./sysmonD -f conf/sysmonD.conf &

