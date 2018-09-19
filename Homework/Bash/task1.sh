#!/bin/bash

#This is homework  by  Maksim Holikov

#--Global definitions
LOCALIZ=$(locale | grep LANGUAGE | cut -d= -f2 | cut -d_ -f1)
PWD=`pwd`
OFILE_NAME="task1.out"
DFILE=$PWD"/$OFILE_NAME"  #"~/bash/task.out"
OFILE=$DFILE
QUONTITY_MIN="1"
QUONTITY_CURR=""


#Function for check is strinf empty
#First  param is string
#Second param is ok message text 
function WriteDataToFile(){
  MESSAGE="Unknown"
  if (( $#==2 )); then
     MESSAGE=$2;
  fi
  echo $MESSAGE >> "$OFILE"
}

#Fuction for write error message
#First  param is key in localize file
function ShowErrorMessage(){
 MESS_TEXT="Unknown error"
  if (( $#>=1)); then  
     MESS_TEXT=`cat "$PWD/Localiz/$LOCALIZ" | grep $1 | sed "s|.*:||"`;
  fi
echo $MESS_TEXT;
exit 0; 
}



#=============================

if (( $#==0 ));then
    ShowErrorMessage "abs_param";
fi

for param in "$@"; do
   case $param in
    -n) 
         while [ -n "$1" ]; do
           if [ "-n" != "$1" ]; then
            QUONTITY_CURR=$1;break;
           fi
         shift
         done
         
         re='^[0-9]+$'
         if [[ $QUONTITY_CURR =~ $re ]]; then
            if (( $QUONTITY_CURR<$QUONTITY_MIN )); then
              ShowErrorMessage "n_smalTh1"
              exit 1;
            fi
         else
           ShowErrorMessage "n_notInt"
           exit 1;
         fi
         
     ;;
    -h|--help)
         echo "Structure of command"
         echo "./task.sh [-h|--help] [-n num] [-f file]"
         exit 0;
      ;;
   esac;
 done

if (( $#==2 )); then
   OFILE=$2;
  if [ ! -f $2 ]; then
    mkdir -p $OFILE; 
    touch "$OFILE/task1.out";
  else
     count=000$(find ./bash/task1*| wc -l);
     date=`date '+%Y%m%y'`
     outputName=$date"-"$count;
     cp ~/bash/task1.out  ~/bash/task1-$outputName.out;
  fi

   OFILE="$2/$OFILE_NAME" 
fi




#===GET INF AND PRINT TO FILE
DATE=`date`;
WriteDataToFile "$DATE" "Date:  $DATE"
WriteDataToFile " " "----Hardware----"

#====HARDWARE====
CPU=`cat /proc/cpuinfo | grep "model name" | sed "s|.*:||"`
WriteDataToFile "$CPU" "CPU:  $CPU"

RAM=`sudo dmidecode -t memory | grep "Maximum Total Memory Size:" | sed "s|.*:||"`
WriteDataToFile  "$RAM" "RAM:  $RAM"

MBOARD=`sudo dmidecode baseboard-product-name |grep "SKU Number" | sed "s|.*:||"`
WriteDataToFile "$MBOARD" "Matherboard:    $MBOARD"

SSNUMBER=`sudo dmidecode -t system |grep "Serial Number:" | sed "s|.*:||"`
WriteDataToFile "$SSNUMBER" "System Serial Number:  $SSNUMBER"

#====SYSTEM=====
WriteDataToFile " " "----System----"

OSDISTR=`cat /etc/*-release |grep "DISTRIB_DESCRIPTION" | sed "s|.*=||"`
WriteDataToFile "$OSDISTR" "OS Distribution:    $OSDISTR"

KVERS=`uname -r`
WriteDataToFile "$KVERS" "Kernel version:   $KVERS"

INST_DATE=`uname -v`
WriteDataToFile "$INST_DATE" "Intalation date:    $INST_DATE"

HOSTNAME=`hostname`
WriteDataToFile "$HOSTNAME" "Hostname:    $HOSTNAME"

UPTIME=`uptime -p`
WriteDataToFile "$UPTIME" "Uptime:    $UPTIME"

PS_RUN=`ps -A --no-headers | wc -l`
WriteDataToFile "$PS_RUN" "Process running:    $PS_RUN"

USER_LOGINED=$(who | wc -l)
WriteDataToFile  "$USER_LOGINED" "User logged in:    $USER_LOGINED"

#====NETWORK====
WriteDataToFile " " "----Network----"

NETWORK=`sudo /sbin/ifconfig -a | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}'`
WriteDataToFile "$NETWORK" "Network:    $NETWORK"

WriteDataToFile " " "----EOF----"
