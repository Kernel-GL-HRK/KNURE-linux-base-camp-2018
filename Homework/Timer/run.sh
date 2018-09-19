#!/bin/bash

export BUILD_KERNEL=/home/mgo/Documents/Course/ORAANGE_PI_ONE/OrangePiH3/kernel
#export BUILD_KERNEL="/home/mgo/Documents/Course/linux-4.17.8"

FILE_NAME="timer";
IP_NAME="localhost"
PATH_FOR_SAVE="/"


if (( "$#">0 )); then
	for param in "$@"; do
	   case $param in
	    -cl|--clean)
           echo "Clean command";
	       make clean;
	       exit 0;
	      ;;
	    -cop)
           echo "Copy command ";

            if [ -f "$FILE_NAME.ko" ]; then
                scp -P 8022 "$FILE_NAME.ko" root@"$IP_NAME":"$PATH_FOR_SAVE" 
            else
           		echo "File $FILE_NAME.ko is absent"
           	fi
        ;;
        -h|--help|*)
            echo "======HELP PAGE====="
	        echo "param -cl/-clean to clean directory "
	        echo "param -cop to make scp command. 
	              option 1 - name file for copy,  
	              option 2 - ip for copy          
	              option 3 - path to save (Default:/)  "
	        echo "without param make"
	        exit 0;
	      ;;       
	   esac;
	 done
else
	make
fi
