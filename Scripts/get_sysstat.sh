#!/bin/sh


TEMPREGEX="[ ]+[+-][0-9]*[.]*[0-9]*"
MEMREGEX="Mem:[ 0-9]*"
MEMREGEX2="cache:[ 0-9]*"
SWAPREGEX="Swap:[ 0-9]*"
DFREGEX="[ ]+[ 0-9%]*/$"

sensors > sensors.txt
free -b > free.txt
df -P > df.txt

if [ ! -e "./procload.txt" ] ; then
   mpstat -P ALL 1 1 > procload.txt
fi

TEMPLINES=$(cat sensors.txt | grep -E -o "$TEMPREGEX")
MEMLINE=$(cat free.txt | grep -E -o "$MEMREGEX" | grep -E -o "[ ]+[ 0-9]*")
MEMLINE2=$(cat free.txt | grep -E -o "$MEMREGEX2" | grep -E -o "[ ]+[ 0-9]*")
SWAPLINE=$(cat free.txt | grep -E -o "$SWAPREGEX" | grep -E -o "[ ]+[ 0-9]*")
DFLINE=$(cat df.txt | grep -E -o "$DFREGEX" | grep -E -o "[0-9]+[ ]+[0-9]+[ ]+[0-9]+")
LOADLINES=$(cat procload.txt | grep -E -v "Average" | grep -E -v "all" | grep -E -v "CPU" | grep -E -o "[0-9]{1,2}([ ]+[0-9]+\.[0-9]+)+")

echo $TEMPLINES > sysstat.txt
echo $MEMLINE >> sysstat.txt
echo $MEMLINE2 >> sysstat.txt
echo $SWAPLINE >> sysstat.txt

echo $DFLINE >> sysstat.txt
echo $LOADLINES >> sysstat.txt


mpstat -P ALL 1 1 > procload.txt &

