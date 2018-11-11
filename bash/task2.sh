# !/bin/bash

case  $1 in
        -h) echo "This is script for home work"; exit 0 ;;
        --h) echo "This is script for home work"; exit 0; shift;;
esac

 	DIR=/usr/local/bin/
        if [ ! -d $DIR ] ; then  mkdir /usr/local/bin/; fi;
	sudo cp task1.sh /usr/local/bin/task1.sh
	cd /usr/local/bin/;
	sudo chmod 755 task1.sh
	exit 0;

