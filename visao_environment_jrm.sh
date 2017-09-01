#!/bin/bash
# VisAO startup file
#
# Created on: 
#

############################################################
# VisAO specific variables
export VISAO_ROOT=/home/jaredmales/Source/Magellan/visao_base
export VISAO_SOURCE=/home/jaredmales/Source/Magellan/visao
export VISAO_LOG=/home/jaredmales/Source/Magellan/visao_base/log
export EDTDIR=/opt/EDTpdv
# ADOPT specific variables (location of adopt source code)
export ADOPT_SOURCE=/home/jaredmales/Source/Magellan/adopt/Magellan
export ADOPT_ROOT=$VISAO_ROOT
export ADOPT_LOG=$VISAO_LOG
export ADOPT_HOME=/home/jaredmales/Source/Magellan/visao_base
export ADOPT_LOGBASE=$VISAO_LOG
export ADOPT_MEAS=/home/jaredmales/Source/Magellan/visao_base/measures
export ADOPT_SIDE=L
export ADOPT_SUBSYSTEM=WFS
#python setup
export PYTHONPATH=$VISAO_ROOT/lib/python
export AOSYS_LOGDIR=/home/jaredmales/Data/Astro/Magellan/magao/LCO/aosys/
export PYTHONPATH=$PYTHONPATH:$VISAO_SOURCE/VisAOScript

############################################################


