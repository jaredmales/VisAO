#!/usr/bin/env python
#
#@File: sys_processes.py
#
# AO system process control. Starts, stops and restarts AO-related processes.
#@


import sys
from qt import *
from AdOpt import cfg
from VisAO import VisAOprocess

# Start application

app = QApplication( sys.argv)

font = QFont("System", 10)
app.setFont(font)

main = VisAOprocess.VisAOprocess( app, multi=True, rowSize=20, caption = "VisAO System processes", showStatus=True)

main.addTitle("<b>VisAO System</b>")
main.addSeparator()

for k in cfg.visao_processes.split():
    main.addProcess( k)
main.addSeparator()

app.setMainWidget(main)
main.show()

app.exec_loop()
