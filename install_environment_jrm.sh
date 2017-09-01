#!/bin/bash
# VISAO installation temporary environment
#
# Created on:
#

############################################################
# VisAO specific variables {Edit to match your system}
export VISAO_ROOT=/home/jaredmales/Source/Magellan/visao_base
export VISAO_SOURCE=/home/jaredmales/Source/Magellan/visao
export VISAO_LOG=/home/jaredmales/Source/Magellan/visao_base/log
export QMAKE=/usr/share/qt4/bin/qmake
export QWT_INCLUDE=/usr/local/qwt-5.2.1/include/
export QWT_LIB="-L/usr/local/qwt-5.2.1/lib/ -lqwt"
export F2C_LIB=-lf2c
export EDTDIR=/home/jaredmales/Source/EDTpdv
export AOSUP_USER=jaredmales
export AOSUP_GROUP=jaredmales
# ADOPT specific variables (location of adopt source code) {Edit to match your system}
#export ADOPT_SOURCE=/home/jaredmales/Source/Magellan/adopt/Magellan
export ADOPT_SOURCE=/home/jaredmales/Source/Magellan/adopt/merged/Magellan

#This is not a simulated system
#export VISAO_SIM=VISAO_NOSIM

#This is a simulated system for development testing (normal loops)
#export VISAO_SIM=VISAO_SIM

#This is a simulated system for development testing (slow loops)
export VISAO_SIM=VISAO_SIMDEV

export VISAO_REC_GPU=REC_NO_GPU
export ATLAS_LIB_PATH=/usr/local/atlas/lib

#Do not change these:
export QTDESTDIR=$VISAO_ROOT/bin
export ADOPT_LOG=$VISAO_LOG
export VISAO_NO_MAIN=__VISAO_NO_MAIN
############################################################
echo 
echo "##############################################"
echo "# Temporary installation environment created #"
echo "#                                            #"
echo "# Use only for installation. The run-time    #"
echo "# environment is set up by the procedure:    #"
echo "#    visao_environment.sh                    #"
echo "##############################################"
echo 
echo "      Source directory: " $VISAO_SOURCE
echo "Installation directory: " $VISAO_ROOT
echo 
echo "Adopt source directory: " $ADOPT_SOURCE
