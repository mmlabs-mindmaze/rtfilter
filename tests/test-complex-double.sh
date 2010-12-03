#!/bin/sh

cmpwmatlab* -d 1 -p 3 -c 15 > test.log
cmpwmatlab* -d 1 -p 3 -c 16 > test.log

cmpwmatlab* -d 3 -p 1 -c 15 > test.log
cmpwmatlab* -d 3 -p 1 -c 16 > test.log

cmpwmatlab* -d 3 -p 3 -c 15 > test.log
cmpwmatlab* -d 3 -p 3 -c 16 > test.log

rm test.log
