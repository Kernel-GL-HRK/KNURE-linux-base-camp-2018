#!/bin/bash
FILE=task1.out
FILE_DIR=~/bash/
FILE_COUNT=0

ERROR_FILE="failed to create file"
HELP="DESCRIPTIO\n\n task1.sh gathers basic information about hardware, perating system and configuration of network interfac ./task1.sh [-h|--help] [-n num] [file]\n\nWhere:\nnum-number of files with results, file - path and name of the file in which the result is to be written;"
#if [ -z "$1" ] # Проверка наличия аргумента командной строки.
#then
#    echo "./ task1.sh [-h|--help]"
#fi

#Функция резервирования
#принемает 2 параметра 
#$1-имя файла
#$2-директория    

function backup {
NAME=$1
DIR=$2

FILE_SUF=""
FILE_NAME=$NAME
  if [ $(expr index $NAME . ) -ne 0 ]
  then
    FILE_SUF=.${NAME:$(expr index $NAME . )} 
    FILE_NAME=${NAME: 0 : $(expr index $NAME . )-1}
  

  
  fi

DATE=$(date +%Y%m%d)
COUNT=$(ls "$DIR" | grep -c "$FILE_NAME-$DATE")
NUM=0


#Резериврование
  if [ $COUNT -ne 0 ]
  then
  
    stringZ=$(ls "$DIR" | grep  "$FILE_NAME-$DATE" | sort -r | head -1) 
    NUM=$(expr "$stringZ" : '.*\([0-9][0-9][0-9][0-9]\)')
    NUM="0000"$(($NUM+1))
    mv $DIR$NAME  "$DIR$FILE_NAME-$DATE-${NUM: -4}$FILE_SUF"
    $(touch "$DIR$NAME")
    	if [ $? -ne 0 ]
	then
	    echo -e $ERROR_FILE "$DIR$NAME" >&2
	    exit 1
	fi
  else
  mv $DIR$NAME  "$DIR$FILE_NAME-$DATE-0000$FILE_SUF"
    $(touch "$DIR$FILE")
    	if [ $? -ne 0 ]
	then
	    echo -e $ERROR_FILE "$DIR$FILE" >&2
	    exit 1
	fi
  fi

}

# Функция формирования логов
# $1-полный путь к файлу
function writeInfo {
NAME=$1


date +'DATE: %a, %d %h %X %z'>>$NAME
echo ---- Hardware ---->>$NAME
inf=$(cat /proc/cpuinfo | grep 'model name' | head -1)
inf=${inf:$(expr index "$inf" : ) }
if [ -z inf ]
then
  echo CPU: "Unknown" >>$NAME
else
  echo CPU: $inf >>$NAME
fi

inf=$( vmstat -s | head -1 )
inf=${inf:0:$(expr index "$inf" K )-1 }
if [ -z inf ]
then
  echo RAM: "Unknown" >>$NAME
else
  echo RAM: $(echo "scale=2; $inf/1024/1024" | bc) GB>>$NAME
fi

inf='"'$(dmidecode --string baseboard-manufacturer)'", ''"'$(dmidecode --string baseboard-product-name)'"'
#system-serial-number
if [ $? -ne 0 ]	
then
  echo Motherboard: "Unknown" >>$NAME
else
  echo Motherboard: $inf >>$NAME
fi

inf='"'$(dmidecode --string system-serial-number)'"'

if [ $? -ne 0 ]	
then
  echo System Serial Number: "Unknown" >>$NAME
else
  echo System Serial Number: $inf >>$NAME
fi

echo ---- System ---- >>$NAME


inf=$(lsb_release -a | grep 'Description')
inf=${inf:$(expr index "$inf" : ) }
if [ -z inf ]
then
  echo OS Distribution: "Unknown" >>$NAME
else
  echo OS Distribution: '"'$inf'"' >>$NAME
fi

inf=$(uname -r)
if [ -z inf ]
then
  echo Kernel version: "Unknown" >>$NAME
else
  echo Kernel version:: '"'$inf'"' >>$NAME
fi

inf=$(dumpe2fs $(mount | grep 'on \/ ' | awk '{print $1}')  | grep 'Filesystem created:')

if [  $? -ne 0 ]
then
  echo 'Installation date': "Unknown" >>$NAME
else
  
  inf=${inf:$(expr index "$inf" : ) }
  echo 'Installation date': $inf >>$NAME
fi

inf=$(hostname)

if [ -z inf ]
then
  echo Hostname: "Unknown" >>$NAME
else
  
  echo Hostname: $inf >>$NAME
fi

inf=$(uptime)
inf=${inf:0:$(expr index "$inf" , ) }
if [ -z inf ]
then
  echo Uptime: "Unknown" >>$NAME
else
  echo Uptime: $inf >>$NAME
fi


inf=$(ps -e | wc -l)

if [  $? -ne 0 ]
then
  echo 'Processes running': "Unknown" >>$NAME
else
  
  echo 'Processes running': $inf >>$NAME
fi

inf=$(who | grep -c [a-Z])

if [  $? -ne 0 ]
then
  echo 'User logged in': "Unknown" >>$NAME
else
  
  echo 'User logged in': $inf >>$NAME
fi

echo ---- Network ---- >>$NAME
NET_COUNT=$(ip link show | grep -c ^[1-9]:)

for (( i = 1; i <= NET_COUNT; i++ ))
do
 inf=$(ip link show | grep  ^$i:)
 first=${#i}+2
 last=$(expr index "$inf" "<")-6
 iterf=${inf:$first:$last}
 str_ip=$(ip address show $iterf | grep "inet ")

 if [[ $str_ip = '' ]]
 then
 echo $iterf: "Unknown">>$NAME
 else
 str_ip=`ip address show $iterf | grep -E -o '([0-9]{1,3}[.]){1,3}[0-9]{1,3}[/][0-9]{1,2}'`
 echo $iterf: $str_ip >>$NAME

 fi


done
echo ----"EOF"---- >>$NAME
}


# Разбор аргументов
while [ -n "$1" ]
do 
    case "$1" in
	--help)
	echo -e  $HELP
        exit 0 
	;;
	-h)
	echo -e  $HELP
	exit 0 
	;;
	-n) 
	   num=$(echo "$2" | grep -E -w [1-9]+)
	   if [ -n "$num" ]
	   then
		FILE_COUNT="$2";shift
	   else
		echo -e $ERROR_PARAM >&2
		exit 1
	   fi 
	   ;;
	*)
        num=$(echo "$1" | grep -E  "^-")
	echo =$num=
	   if [ -n "$num" ]
	   then
		echo "./ task1.sh [-h|--help]"
                exit 1

	   fi 
        
	FILE=$(basename "$1")
	FILE_DIR=$(dirname "$1")/


	;;
    esac
shift
done

echo "--------------$FILE $FILE_DIR-----------"

#Проверка директории
if [ -d "$FILE_DIR" ]
then #директория существует
    FILE_COUNT=$(ls "$FILE_DIR" | grep -c task1)
else #директория не существует
$(mkdir $FILE_DIR 2> /dev/null)
    	if [ $? -ne 0 ]
	then
	    echo -e "failed to create dir">&2
	    exit 1
	fi
fi
#Проверка файла
if [ -e "$FILE_DIR$FILE" ]
    then
	backup $FILE $FILE_DIR
    else 
    	$(touch "$FILE_DIR$FILE")
    	if [ $? -ne 0 ]
	then
	    echo -e "failed to create file">&2
	    exit 1
	fi
fi
   
writeInfo $FILE_DIR$FILE 

#Удаление лишних файлов
if [ $FILE_COUNT -gt 0 ]
then
  NAME=$FILE
  if [[ $(expr index $FILE . ) -ne 0 ]]
  then
    NAME=${FILE: 0 : $(expr index $FILE . )-1}
  fi

 list=$(ls "$FILE_DIR" | grep  "$NAME") 
 count=$(($(ls "$FILE_DIR" | grep -c "$NAME")-$FILE_COUNT-1))

 for var in $list
  do
  if [ $count -le 0 ]
  then
   break
  fi
  count=$(( $count -1 ))
  rm $FILE_DIR$var
 
  done
fi
exit 0



  

 


















         


