#!/bin/bash
# VISAO installation temporary environment
#
# Created on:
#

############################################################
# VisAO specific variables
export VISAO_ROOT=/home/aosup/visao
export VISAO_SOURCE=/home/aosup/Source/visao
export VISAO_LOG=/home/aosup/visao/log
export QMAKE=/usr/lib64/qt4/bin/qmake
export QWT_INCLUDE=/usr/local/qwt-5.2.1/include/
export QWT_LIB="-L/usr/local/qwt-5.2.1/lib/ -lqwt"
export F2C_LIB="-L/usr/lib/gcc/x86_64-redhat-linux/3.4.6/ -lg2c"
export EDTDIR=/opt/EDTpdv
export AOSUP_USER=aosup
export AOSUP_GROUP=aosup
# ADOPT specific variables (location of adopt source code)
export ADOPT_SOURCE=/home/aosup/Source/adopt/Magellan

#This is NOT a simulated system
export VISAO_SIM=VISAO_NOSIM
#changed for vizcacha
export VISAO_REC_GPU=REC_USE_GPU
#export VISAO_REC_GPU=REC_NO_GPU
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
