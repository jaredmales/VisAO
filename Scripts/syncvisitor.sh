#!/bin/sh

####################################################################################
# syncvisitor.sh
#
# This synchronizes the visaosup data directory with the visitor data directory.
# 
#
# 
# Author: Jared Males (jrmales@email.arizona.edu)
# Date:  2014.04.01
# 
# History:
#       - Updated for running as a cron job by JRM 2014.11.03
#
####################################################################################



#These are absolute paths so this runs as a cron job
#though once .bashrc is included it should be fine.
source /home/aosup/.bashrc

source /home/aosup/bin/magao_dirdate.sh

datedir=visao_$(magao_dirdate) #20141103_04

echo $datedir

rsync -ave  ssh aosup@visaosup.visao:/home/aosup/visao/data/ccd47/  --ignore-existing /home/visitor/VisAO/$datedir

sleep 30
rsync -ave ssh aosup@visaosup.visao:/home/aosup/visao/data/syslogs /home/aosup/visao/data/


#chmod a+rw -R /home/visitor/VisAO/data/ccd47

/home/aosup/Source/visao/PyScripts/visao_headup.py -r /home/visitor/VisAO/$datedir

echo "syncvisitor.sh completed $(date)" >> /home/aosup/visao/synclog.txt




