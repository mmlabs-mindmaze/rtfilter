#!/bin/sh
  
LOGFILE=testdown.log

if {
    downsample* -t 0 -c 16 \
 && downsample* -t 1 -c 16
} > $LOGFILE
then
	rm $LOGFILE
	stty echo
	exit 0
else
	stty echo
	exit 1
fi
