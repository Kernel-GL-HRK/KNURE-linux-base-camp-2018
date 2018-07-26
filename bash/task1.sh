#!/bin/bash
FILE=~/bash/task1.out
UNKNOWN="Unknown"

printError()
{
	(>&2 echo "$@")
	exit 1
}


printHelp()
{
	echo "$0 [-h|--help] [-n num] [file]"
	echo -e "\t-n sets number of files with results which should be kept"
	echo -e "\t-h|--help prints this help"
}


printCommand()
{
	local OUTPUT
	OUTPUT=`eval $1`
	if [ $? -ne 0 -o -z OUTPUT ]; then
		echo $UNKNOWN
	else
		echo "$OUTPUT"
	fi
}


printNamedCommand()
{
	printf "$1: "
	printCommand "$2"
}


printCPUInfo()
{
	local CMD="false"
	if [ -f /proc/cpuinfo ]; then
		CMD="grep 'model name' /proc/cpuinfo 2>/dev/null | head -1 | sed -r 's/model name\s+: //'"
	fi

	printNamedCommand "CPU" "$CMD"
}


printMemInfo()
{
	local OUTPUT
	OUTPUT=`dmidecode -t memory 2>/dev/null`
	if [ $? -eq 0 ]; then
		local M=0

		for var in `printf "$OUTPUT" | egrep '^\s+Size:' | awk '{print $2}'`
		do
			M=$(($M+$var))
		done

		if [ $M -eq 0 ]; then
			OUTPUT=$UNKNOWN
		else
			OUTPUT="$M MB"
		fi
	else
		OUTPUT=$UNKNOWN
		local MEM=`free -m | grep Mem: | awk '{print $2}'`
		if [ ! -z MEM ]; then
			let "m = $MEM % 1024"
			if [ $m -gt 0 ]; then
				let "m = $MEM / 1024 + 1"
				MEM=`printf "%.0f" $m`
				let "MEM = $MEM * 1024"
			fi
			OUTPUT="$MEM MB"
		fi
	fi

	echo "RAM: $OUTPUT"
}


printMotherboardInfo()
{
	local OUTPUT="$UNKNOWN"
	local TEMP_OUTPUT
	TEMP_OUTPUT=`dmidecode -t baseboard 2>/dev/null`

	if [ $? -eq 0 ]; then
		local MANUFACTURER=`printf %s "$TEMP_OUTPUT" | grep Manufacturer | sed -r 's/\s+Manufacturer: //'`
		local PRODUCT=`printf %s "$TEMP_OUTPUT" | grep "Product Name" | sed -r 's/\s+Product Name: //'`
		OUTPUT="\"$MANUFACTURER\", \"$PRODUCT\""
	fi

	echo "Motherboard: $OUTPUT"
}


printSerialNumber()
{
	local OUTPUT
	OUTPUT=`dmidecode -t system 2>/dev/null`

	if [ $? -eq 0 ]; then
		OUTPUT=`printf %s "$OUTPUT" | grep 'Serial Number' | sed -r 's/\s+Serial Number: //'`
	else
		OUTPUT=$UNKNOWN
	fi
	echo "Serial Number: $OUTPUT"
}


printHardware()
{
	echo "---- Hardware ----"
	printCPUInfo
	printMemInfo
	printMotherboardInfo
	printSerialNumber
}


printOsDistributionInfo()
{
	printf "OS Distibution: "
	while [ true ]
	do
		if [ ! -f /etc/os-release ]; then
			break;
		fi

		local NAME
		NAME=`egrep "^NAME=" /etc/os-release | sed -r 's/NAME=//' | sed 's/"//g'`

		if [ $? -ne 0 ]; then
			break
		fi

		local VERSION=`egrep "^VERSION=" /etc/os-release | sed -r 's/VERSION=//' | sed 's/"//g'`

		if [ $? -ne 0 ]; then
			break
		fi

		echo "$NAME $VERSION"
		return 0
	done

	echo $UNKNOWN
}


printKernelVersion()
{
	printNamedCommand "Kernel version" "uname -r"
}


printInstallationDate()
{
	local OUTPUT=`df / | tail -1 | cut -f1 -d ' '`
	OUTPUT=`tune2fs -l $OUTPUT 2>/dev/null | grep 'Filesystem created:' | sed -r 's/Filesystem created:\s+//'`
	if [ -z "$OUTPUT" ]; then
		OUTPUT=$UNKNOWN
	fi
	echo "Installation date: $OUTPUT"
}


printHostname()
{
	printNamedCommand "Hostname" "hostname"
}


printUptime()
{
	printNamedCommand "Uptime" "printf %s `uptime | awk '{print $1}'`"
}


printProcessCount()
{
	printNamedCommand "Processes running" "ps aux --no-heading 2>/dev/null | wc -l"
}


printLoggedUsersCount()
{
	printNamedCommand "Users logged in" "who | wc -l"
}


printNetwork()
{
	echo "--- Network ---"
	ip addr show | egrep '^[0-9]+' | awk '{print $2}' | sed 's/://' | while read iface
	do
#		echo $iface
#		printf "\tv4: "
#		local ADDR=`ip addr show $iface | grep 'inet ' | awk '{print $2}'`
#		if [ -n $ADDR ]; then
#			echo $ADDR
#		else
#			echo "--/--"
#		fi

#		printf "\tv6: "
#		ADDR=`ip addr show $iface | grep 'inet6 ' | awk '{print $2}'`
#		if [ -n $ADDR ]; then
#			echo $ADDR
#		else
#			echo "--/--"
#		fi
		printf "%s: " $iface
		local ADDR=`ip addr show $iface | grep 'inet ' | awk '{print $2}'`
		if [ -n "$ADDR" ]; then
			echo $ADDR
		else
			echo "--/--"
		fi
	done
}



printSystemInfo()
{
	echo "---- System ----"
	printOsDistributionInfo
	printKernelVersion
	printInstallationDate
	printHostname
	printUptime
	printProcessCount
	printLoggedUsersCount
}


validateParams()
{
	local FILE_SET=0

	while [[ $# -gt 0 ]];
	do
		case "$1" in
			-h|--help)
				printHelp "$@"
				exit 0
				;;
			-n)
				shift
				if (( $# )); then
					if [ $1 -gt 0 ]; then
						NUM_FILES=$1
					else
						printError "-n argumet's value is not positive number"
					fi
				else
					printError "-n argument has no value"
					exit 1
				fi
				;;
			*)
				if [ $FILE_SET -eq 1 ]; then
					printError "unknown argument passed '" $1 "'"
					exit 1
				fi

				FILE_SET=1
				FILE=$1
				;;
		esac
		shift
	done
}


printAll()
{
	printf "Date: %s\n" "`date`"
	printHardware
	printSystemInfo
	printNetwork
	echo "---\"EOF\"---"
}


prepareDir()
{
	local DIR
	DIR=`dirname $FILE 2>/dev/null`
	mkdir -p $DIR 2>/dev/null
	if [ $? -ne 0 ]; then
		printError "can't create dir: $DIR"
		exit 1
	fi

	if [ ! -w $DIR ]; then
		printError "can't write into: $DIR"
	fi


	if [ -f $FILE ]; then
		local FILENAME
		FILENAME=`basename $FILE`
		local DATE_POSTFIX
		DATE_POSTFIX=`date '+%Y%m%d'`
		local MASK=`printf %s_%s_* "$FILENAME" "$DATE_POSTFIX"`

		pushd $DIR &>/dev/null
		local LAST_INDEX=`ls $MASK 2>/dev/null | sort -r | egrep '_[0-9]{8}_[0-9]{4}$' | head -1 | sed -rn 's/^.+([0-9]{4})$/\1/p' | sed -r 's/^0{0,3}//'`
		popd &>/dev/null
		
		if [ -z $LAST_INDEX ]; then
			LAST_INDEX=0
		else
			((LAST_INDEX++))
		fi

		MOVE_FILE_NAME=`printf %s_%s_%04d "$FILE" "$DATE_POSTFIX" $LAST_INDEX`

		mv $FILE $MOVE_FILE_NAME

		if [ $? -ne 0 ]; then
			printError "can't rename file: $FILE"
		fi
	fi
}


removeBackupFiles()
{
	if [ -z $NUM_FILES ]; then
		return 0
	fi

	local N=$NUM_FILES
	((N++))

	MASK=`printf "%s_*" "$FILE"`
	ls $MASK 2>/dev/null | egrep '_[0-9]{8}_[0-9]{4}$' | sort -r | tail -n +$N | xargs rm -f

	if [ $? -ne 0 ]; then
		printError "failed to remove files"
	fi
}


validateParams "$@"

prepareDir
removeBackupFiles

printAll > $FILE
exit 0
