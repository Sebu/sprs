#!/bin/bash

export LD_LIBRARY_PATH=~/git/epitome/epicore:~/playground/lib
echo $1
~/git/epitome/epicli/epicli -i $1 -e 0.15  > /dev/null
