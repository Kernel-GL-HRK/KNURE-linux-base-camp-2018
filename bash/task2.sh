#!/bin/bash
FILE_COPY=task1.sh
FILE_DIR="/usr/local/bin"
FLAG_DIR=$(echo $PATH | grep -c $FILE_DIR)
DATE=$(date +%Y%m%d)
ERROR_FILE="failed to copy file"
HELP=" Description \n\ntask2.sh installs the task1.sh program on the system. ./task2.sh [-h | --help]"
ERROR_PARAM="incorrect parameter -n\ninteger n>1 "
ERROR_DIR="failed to create dir"

if [[ $LANG==ru_UA.UTF-8 ]]; then
 
 ERROR_FILE="не вдалося скопіювати файл"
 HELP="Опис\n\n task2.sh встановлює  програму task1.sh в систему. ./task2.sh [-h|--help]"
  ERROR_PARAM="неправильний параметр"
 ERROR_DIR="не вдалося створити директорію"
fi

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
	
	*)
       
	echo "./task2.sh [-h|--help]"
        exit 1
	;;
    esac
shift
done

if [ -d "$FILE_DIR" ]
then #директория существует

  if [ -e "./$FILE_COPY" ]
  then
    $(cp "./$FILE_COPY" $FILE_DIR)
    if [ $? -ne 0 ]
    then
      echo -e ---$ERROR_FILE "./$FILE_COPY" >&2
     exit 1
    fi
    chmod 755 $FILE_DIR/$FILE_COPY
  else 
   echo -e $ERROR_FILE "./$FILE_COPY" >&2
   exit 1
  fi
else #директория не существует
 $(mkdir $FILE_DIR 2> /dev/null)
 if [ $? -ne 0 ]
   then
     echo -e $ERROR_DIR >&2
     exit 1
 fi
 if [ -e "./$FILE_COPY" ]
  then
    $(cp "./$FILE_COPY" $FILE_DIR)
    if [ $? -ne 0 ]
    then
      echo -e $ERROR_FILE "./$FILE_COPY" >&2
    exit 1
    fi
    chmod 755 $FILE_DIR/$FILE_COPY
  else 
   echo -e $ERROR_FILE "./$FILE_COPY" >&2
   exit 1
  fi
fi

if [ $FLAG_DIR ]
then

  if [ -e "/etc/profile" ]
  then
    $(cp "/etc/profile" "/etc/profile-$DATE")
    echo export PATH=$PATH:$FILE_DIR >> /etc/profile
  fi

  if [ -e "/etc/bash.bashrc" ]
  then
    $(cp "/etc/bash.bashrc" "/etc/bash-$DATE.bashrc")
    echo export PATH=$PATH:$FILE_DIR >> /etc/bash.bashrc
  fi

  

fi
