#!/bin/sh

####################################################################################
# archive_visao.sh
#
# This archives the visao data directories, for use at the end of the night
# 
#
# 
# Author: Jared Males (jrmales@email.arizona.edu)
# Date:  2014.11.06
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

#create the archive directories

mkdir -p /home/aosup/visao/data/archive/ccd47/$datedir
mkdir -p /home/aosup/visao/data/archive/ccd39/$datedir

mv  /home/aosup/visao/data/ccd47/*  /home/aosup/visao/data/archive/ccd47/$datedir
mv  /home/aosup/visao/data/ccd39/*  /home/aosup/visao/data/archive/ccd39/$datedir


echo "archive_visao.sh completed $(date)" >> /home/aosup/visao/data/archive/archive.log

