#!/bin/sh

####################################################################################
# magao_dirdate.sh
#
# A bash function to generate the MagAO data directory date stamp.  Is correct for
# all dates (end of month, end of year, etc.)
#
# Assumes that the result of $(date) is local time
# 
# Author: Jared Males (jrmales@email.arizona.edu)
# Date:  2014.11.03
# 
# History:
#
####################################################################################
function magao_dirdate()
{

  hour="$(date +%H)" 

  if [  $hour -gt 12  ]; then
    first=today
    second=tomorrow  
  else
    first=yesterday
    second=today
  fi

  year="$(date --date=$first +%Y)"
  month="$(date --date=$first +%m)"
  today="$(date --date=$first +%d)"
  tomorrow="$(date --date=$second +%d)"  
  

  echo $year$month$today"_"$tomorrow

}










