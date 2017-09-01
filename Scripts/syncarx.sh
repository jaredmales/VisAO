#!/bin/sh


localdir=/home/aosup/visao/data/archive/ccd47/
remotedir=/Data/VisAO/archive/ccd47/

rsync --verbose --archive --progress --update --recursive --compress \
--bwlimit=500 -e ssh \
$localdir jrmales@magaoarx.as.arizona.edu:$remotedir


