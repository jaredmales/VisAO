#!/bin/bash
# VisAO startup file
#
# Created on: 
#

############################################################
# VisAO specific variables
export VISAO_ROOT=/home/aosup/visao
export VISAO_SOURCE=/home/aosup/Source/visao
export VISAO_LOG=/home/aosup/visao/log
export EDTDIR=/opt/EDTpdv
# ADOPT specific variables (location of adopt source code)
export ADOPT_SOURCE=/home/aosup/Source/adopt/Magellan
export ADOPT_ROOT=$VISAO_ROOT
export ADOPT_LOG=$VISAO_LOG
export ADOPT_HOME=/home/aosup/visao
export ADOPT_LOGBASE=$VISAO_LOG
export ADOPT_MEAS=/home/aosup/visao/measures
export ADOPT_SIDE=L
export ADOPT_SUBSYSTEM=WFS
#python setup
export PYTHONPATH=$VISAO_ROOT/lib/python
export AOSYS_LOGDIR=$VISAO_ROOT/data/aosys/
export PYTHONPATH=$PYTHONPATH:$VISAO_SOURCE/VisAOScript

############################################################

PATH=$VISAO_ROOT/bin:$PATH


############################################################
# Development specific variables
export QMAKE=/usr/lib64/qt4/bin/qmake
export QWT_INCLUDE=/usr/local/qwt-5.2.1/include/
export QWT_LIB="-L/usr/local/qwt-5.2.1/lib/ -lqwt"
export F2C_LIB="-L/usr/lib/gcc/x86_64-redhat-linux/3.4.6/ -lg2c"

export AOSUP_USER=aosup
export AOSUP_GROUP=aosup
# ADOPT specific variables (location of adopt source code)
export ADOPT_SOURCE=/home/aosup/Source/adopt/Magellan

#This is NOT a simulated system
export VISAO_SIM=VISAO_NOSIM

export VISAO_REC_GPU=REC_USE_GPU
export ATLAS_LIB_PATH=/usr/local/atlas/lib

#Do not change these:
export QTDESTDIR=$VISAO_ROOT/bin
export ADOPT_LOG=$VISAO_LOG
export VISAO_NO_MAIN=__VISAO_NO_MAIN

