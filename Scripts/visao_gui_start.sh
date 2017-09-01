#!/bin/sh

#./CCD47CtrlGUI -f conf/CCD47GUI.conf &
#sleep .2
./imviewer &
sleep .2
#./FocusMotorGUI -f conf/FocusMotorGUI.conf &
#sleep .2
#./WollastonStatusGUI -f conf/WollastonStatusGUI.conf &
#sleep .2
./sysmonDGUI -i sysmonD &
sleep .2
#sims/ShutterTesterGUI -f conf/sims/ShutterTesterGUI.conf &
#sleep .2

./VisAOEngGUI -i visaoengGUI

