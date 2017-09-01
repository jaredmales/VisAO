#!/bin/sh

####################################################################################
# syncvisao.sh
#
# This synchronizes the visaosup data directory with vizzy's data directory.
# Should be run every morning at the completion of observing.
#
# 
# Author: Jared Males (jrmales@email.arizona.edu)
# Date:  2014.04.01
# 
# History:
#       - Updated for running as a cron job by JRM 2015.05.24
#
####################################################################################



#These are absolute paths so this runs as a cron job
#though once .bashrc is included it should be fine.
source /home/aosup/.bashrc

source /home/aosup/bin/magao_dirdate.sh


rsync -ave  ssh aosup@visaosup.visao:/home/aosup/visao/data/archive  --ignore-existing /home/aosup/visao/data/

#sleep to make sure the latest syslogs are available
sleep 120
rsync -ave ssh aosup@visaosup.visao:/home/aosup/visao/data/syslogs /home/aosup/visao/data/

#now update the heads for today
visao_headup.py -r /home/aosup/visao/data/archive/ccd47/visao_$(magao_dirdate)

