#!/bin/sh

cmpwmatlab* -d 0 -p 2 -c 15 > test.log

cmpwmatlab* -d 2 -p 0 -c 15 > test.log

cmpwmatlab* -d 2 -p 2 -c 15 > test.log

rm test.log
