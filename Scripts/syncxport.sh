#!/bin/sh

#This synchronizes vizzy's data directory with an attached USB drive.

drive="MagAO_xport_001.1"

rsync -av /home/aosup/visao/data/archive  --ignore-existing /media/$drive/visao/data/
rsync -av /home/aosup/visao/data/syslogs  --ignore-existing /media/$drive/visao/

