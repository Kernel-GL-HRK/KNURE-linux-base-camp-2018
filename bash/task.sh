#This is home work by Mordyk Alex
# !/bin/bash

#Description: Function to check -h and -help key for information;
function checkHelpParam(){
case  $1 in
        -h) echo "Information of script: [-h|--help] [-n num] [file];"; exit 0 ;;
        --help) echo "Information of script: [-h|--help] [-n num] [file];"; exit 0; shift;;
esac
}

#Description: Function to check n key;
function checkNParam(){
	if [ "$1" != "-n" ]; then echo "Erro key -n" >&2; exit 1; fi;
}

#Description:Function to check exist file;

checkHelpParam $1;
checkNParam $1;
shift;

if [[ $1 =~ ^-?[0-9]+$ ]]; then
                if (($1<2)); then echo "Error of number" >&2; exit 1; 
                else
                Number=$1;
                shift;
                fi;
else 
        echo "Dont integer number" >&2; exit 1;
fi;

shift;

if [ ! -f $1 ] || [ -z "$1" ] ; then
       DIR=~/bash/
       FILE=~/bash/task1.out
       date=`date '+%d%m%y'`

       if [ ! -d $DIR ] ; then  mkdir ~/bash/; fi;
       if [ ! -f $FILE ] ; then  touch  ~/bash/task1.out;
       else
                count=000$(find ./bash/task1*| wc -l);

                date=`date '+%Y%m%y'`
                outputName=$date"-"$count;

                cp ~/bash/task1.out  ~/bash/task1-$outputName.out;
#rm -f abc.log.*
        fi;

                countFiles=$(ls -f ~/bash/ | wc -l)
               # if (( $countFiles > $Number )); then echo 33 fi;

        else
        FILE=$1;
fi;

fileData=$(date +"%a,%d-%m-%y %r %z ");
if [[ $? && ! -z $fileData ]] ; then 
echo  "Date:" $fileData >> $FILE;
else
echo "Date: unknow">>$FILE;
fi;

echo  "---- HardWare ----" >> $FILE

modelCPU=$(lscpu | grep "Имя модели:");
if [[ $? && ! -z $modelCPU ]]; then
echo $modelCPU>> $FILE;
else
echo "CPU: unknow" >> $FILE;
fi;

ram=$(sudo dmidecode -t 16 | grep  -oP 'Maximum Capacity:\s*\K.+');
if [[ $? && ! -z $ram ]]; then
echo  "RAM:\"" $ram  "\"" >>$FILE;
else
echo "RAM: unknow">>$FILE;
fi;

motherB=$(sudo dmidecode -t 2 | grep  -oP 'Manufacturer:\s*\K.+');
if [[ $? && ! -z $motherB ]]; then 
echo "Motherboard:" $motherB>>$FILE;
else
echo "Motherboard: unknow">>$FILE;
fi;

SSB=$(sudo dmidecode -t 2 | grep  -oP 'Serial Number:\s*\K.+');
if [[ $? && ! -z $SSD ]]; then
echo "System Serial Number:" $SSB>>$FILE;
else
echo "System Serial Number: unknow">>$FILE;
fi;

echo "---- System ----">>$FILE

osD=$(cat /etc/*release* | grep -oP 'PRETTY_NAME=\s*\K.+');
if [[ $? && ! -z $osD ]]; then 
echo "OS Distribution:" $osD>>$FILE;
else
echo "OS Distribution: unknow">>$FILE;
fi;

KV=$(uname -r) ;
if [[ $? && ! -z $KV ]]; then 
echo "Kernel version:" $KV >>$FILE;
else
echo "Kernel version: unknow">>$FILE;
fi;


inDate=$(uname -a |tail -1|awk '{print $6, $7, $8, $9, $11}')
if [[ $? && ! -z $inDate ]]; then 
echo "Installation date:" $inDate >>$FILE;
else
echo "Installation date: unknow">>$FILE;
fi;

HN=$(hostname);
if [[ $? && ! -z $HN ]]; then
echo "Hostname:" $HN >>$FILE;
else
echo "Hostname: unknow">>$FILE;
fi;

UT=$(uptime);
if [[ $? && ! -z $UT ]]; then
echo "Uptime:" $UT>>$FILE;
else
echo "Uptime: unknow">>$FILE;
fi;

PT=$(ps aux | wc -l);

if [[ $? && ! -z $PT ]]; then 
echo "Processes tunning:" $PT >>$FILE;
else
echo "Processes tunning: unknow">>$FILE;
fi;

ULI=$(who | wc -l);
if [[ $? && ! -z $ULI ]]; then 
echo "User logged in:" $(who | wc -l)>>$FILE;
else
echo "User logged in: unknow">>$FILE;
fi;

echo "---- Network ----">>$FILE

#loIP= $(ip address show | grep -A1  inet | cut -d "/" -f1);
if [[ $? && ! -z $loIP ]]; then 
echo "lo:" $olIP>>$FILE;
else
echo "lo: unknow">>$FILE;
fi;



echo "---- "EOF"----" >>$FILE

exit 0;














