#!/bin/bash

num=0
file=~/bash/task1.out
fileNum=0

function echoerr() {
	echo "$*" 1>&2
}

function PrintHelp() {
	help_message="$scriptName [-h | --help] [-n num] [file]"
	echo $help_message
}

function ParseArgs() {
	while [ -n "$1" ]; do
		if [[ "$1" =~ ^- && ! "$1" == "--" ]]; then 
			case "$1" in
				-h | --help)
					PrintHelp
					exit
				;;
				-n)
					shift
					if [[ $1 =~ ^[0-9]+$ && ! ($1 == '0' || $1 == '1') ]]; then 
						num=$1
					else
						echoerr "n must be greater 1"
						exit 1
					fi
				;;
				*)
					echoerr "unexpected command $1"
					exit 1
				;;
			esac
		else
			file=$1
		fi
		shift
	done
}

function TrimFront() {
	echo $1 | sed 's/[ \t]//'
}

function RenameIfExists() {
	if [ -e $1 ]; then
		fileNum=$2
		name=$(basename "$file")
		dir=$(dirname "$file")
		filename=$(echo $name | cut -d '.' -f1)
		ext=$(echo $name | cut -d '.' -f2)
		date=$(date +"%Y%m%d")

		currDateFilenamePrefix=$(printf '%s-%s' $filename $date)
		latestFileNum=$(ls $dir | tr ' ' '\n' | sed -rn "s/$currDateFilenamePrefix-0{0,3}([0-9]*).out/\1/p" | sort -r | head -n1 | tr -d ' ')
		newFileName=$(printf '%s/%s-%04d.%s' $dir $currDateFilenamePrefix $fileNum $ext)
		if [[ $fileNum -le $latestFileNum ]]; then 
			fileNum=$((fileNum + 1))
			RenameIfExists $newFileName $fileNum
		fi
		cp -f "$1" "$newFileName" 
	fi
}

function CreateFile() {
	absPath=$(echo $file)
	echo "$absPath"
	if [ ! -e $absPath ]; then
		dirname=$(dirname "$absPath")
		mkdir -p "$dirname"
	else
		RenameIfExists $absPath 0
	fi
	touch "$absPath"
}

function CollectInfo() {
	date=$(date)
	
	cpu=$(cat /proc/cpuinfo | grep 'model name' | sed -n '1p' | cut -d ':' -f2 | sed 's/^[ \t]*//')	
	mem=$(cat /proc/meminfo | grep 'MemTotal' | cut -d ':' -f2 | sed 's/^[ \t]*//' | cut -d ' ' -f1)
	ram=$((mem / 1024))

	unknown="Unknown"
	baseboardMan=$(dmidecode | grep 'baseboard-manufacturer' | cut -d ':' -f2 | sed 's/^[ \t]*//')
	if [ -z "$baseboardMan" ]; then
		baseboardMan=$unknown
	fi
	baseboardProdName=$(dmidecode | grep 'baseboard-product-name' | cut -d ':' -f2 | sed 's/^[ \t]*//')
	if [ -z "$baseboardProdName" ]; then
		baseboardProdName=$unknown
	fi
	motherboard="\"$baseboardMan\", \"$baseboardProdName\""
	ssn=$(dmidecode | grep 'system-serial-number' | cut -d ':' -f2 | sed 's/^[ \t]*//')
	if [ -z "$ssn" ]; then
		ssn=$unknown
	fi


	distr=$(lsb_release -d | cut -d ':' -f2 | sed 's/^[ \t]*//')
	kernRev=$(uname -r)
	machine=$(uname -m)
	kernVersion="${kernRev}.${machine}"
	host=$(uname -n)
	uptime=$(uptime -p)
	numOfUsers=$(who | wc -l)
	numOfProccesses=$(ps -A --no-headers | wc -l)
	
	rootDeviceName=$(mount | grep 'on \/ ' | cut -d ' ' -f1)
	instDate=$(tune2fs -l $rootDeviceName  | grep 'Filesystem created:' | cut -d ':' -f2 | sed 's/^[ \t]*//')
	
	echo "Date: $date" > $file

	echo "---- Hardware ----" >> $file
	echo "CPU: $cpu" >> $file
	echo "RAM: $ram MB" >> $file
	echo "Motherboard: $motherboard" >> $file
	echo "System Serial Number: $ssn" >> $file

	echo "---- System ----" >> $file
	echo "OS Distribution: \"$distr\"" >> $file
	echo "Kernel version: $kernVersion" >> $file
	echo "Installation date: $instDate" >> $file
	echo "Hostname: $host" >> $file
	echo "Uptime: $uptime" >> $file
	echo "Processes running: $numOfProccesses" >> $file
	echo "User logged in: $numOfUsers" >> $file

	echo "---- Network ----" >> $file
	ipOutput=$(ip -f inet -o addr)
	echo "${ipOutput}" | while read line; do
		line=$(echo "${line}" | tr -s [:blank:] | cut -d ' ' -f2,4 | sed 's/ /: /')
		echo "${line}" >> $file
	done
	echo "---- \"EOF\" ----" >> $file
}

function RemoveFiles() {
	if [[ "$num" -gt 1 ]]; then 
		name=$(basename "$file")
		dir=$(dirname "$file")
		filename=$(echo $name | cut -d '.' -f1)
		ext=$(echo $name | cut -d '.' -f2)
		pwd=$(pwd)
		cd "$dir"
		ls "$dir" | egrep "$filename-[0-9]{8}-[0-9]{4}\.$ext" | tail -n +$num | xargs -d '\n' rm -f > /dev/null
		cd "$pwd"
	fi
}

scriptName=$0

inp_args=$*
ParseArgs $inp_args
CreateFile
CollectInfo
RemoveFiles
