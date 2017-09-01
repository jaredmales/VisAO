#!/usr/bin/env python

import sys
from qt import *
from AdOpt import AOExcept


class VisAOAppGUI( QMainWindow):

    def __init__(self, qApplication, name='AOAppGUI', debug=True, multi=False):
        QMainWindow.__init__(self)

        self.qApp = qApplication

    #@Method{API}: callAfter
    #
    # Method to call a generic function from the main (GUI) thread instead
    # of the current thread.
    # QT IS NOT THREAD-SAFE: any worker thread wishing to manipulate GUI objects
    # must do it through this function.
    #@

    def callAfter( self, func, args):
        self.qApp.postEvent( self, QCustomEvent( QEvent.User, (func, args)))

    #@Method: customEvent
    #
    # Handler for the custom event posted by callAfter()
    #@

    def customEvent(self, ev):
        f, args = ev.data()
        if not hasattr(args, '__iter__'):
            args = [args]
        apply(f, args)

    def die(self):
        self.qApp.quit()
        


if __name__ == "__main__":
    import sys

    qApp = QApplication( sys.argv)
    main = VisAOAppGUI(qApp)
    qApp.setMainWidget(main)
    main.show()
    qApp.exec_loop()

