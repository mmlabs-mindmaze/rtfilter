#!/bin/sh

LOGFILE=testf.log

if {
     cmpwmatlab* -d 0 -p 0 -c 16 \
  && cmpwmatlab* -d 0 -p 0 -c 15
} > $LOGFILE
then
	rm $LOGFILE
	exit 0
else
	exit 1
fi
