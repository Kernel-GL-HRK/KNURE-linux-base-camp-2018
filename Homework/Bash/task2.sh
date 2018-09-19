#!/bin/bash

set -e
#==Defenitions
LOCALIZ=$(locale | grep LANGUAGE | cut -d= -f2 | cut -d_ -f1)
TARGET_DIR="/usr/local/bin"



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





for param in "$@"; do
   case $param in
    -h|--help)
         echo "======HELP PAGE====="
         exit 0;
      ;;
     *)
        ShowErrorMessage "unknown"
      ;;
   esac;
 done



#Check is target directory exist
if [ ! -f "$TARGET_DIR" ]; then
 sudo mkdir -p "$TARGET_DIR";
fi


if [ ! -f "$TARGET_DIR/Localiz" ]; then
	sudo mkdir -p "$TARGET_DIR/Localiz";
	sudo cp "$PWD/Localiz/en" "$TARGET_DIR/Localiz/en"
	sudo cp "$PWD/Localiz/uk" "$TARGET_DIR/Localiz/uk"
	sudo cp "$PWD/Localiz/ru" "$TARGET_DIR/Localiz/ru"
fi


#install process
sudo install "$(dirname "$0")/task1.sh" "$TARGET_DIR/"
ShowErrorMessage "OK"