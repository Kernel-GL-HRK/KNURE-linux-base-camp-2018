#!/bin/bash

DIR=/usr/local/bin
FILE=task1.out

printError()
{
	(>&2 echo "error: $@")
	exit 1
}


printHelp()
{
	echo "Installs $FILE to $DIR"
	echo "Usage: $0 [-h|--help]"
	echo -e "\t-h|--help - print this help"
}


prepareDir()
{
	mkdir -p $DIR

	if [[ $? -ne 0 ]]; then
		printError "failed to create directory: $DIR"
	fi

	if [ ! -w $DIR ]; then
		printError "$DIR is not writeable"
	fi
}


validateUser()
{
	if [[ "$EUID" -ne 0 ]]; then
		printError "must be run as root"
	fi
}


validateParams()
{
	while [[ $# -gt 0 ]];
	do
		case "$1" in
			-h|--help)
				printHelp "$@"
				exit 0
				;;
			*)
				printError "unknown parameter: $1"
				;;
		esac
		shift
	done

	if [ ! -f $FILE ]; then
		printError "file $FILE does not exists"
	fi

	if [ ! -r $FILE ]; then
		printError "can't read $FILE"
	fi
}


backupFile()
{
	local DIR
	DIR=`dirname $1`

	if [ ! -w $DIR ]; then
		printError "$DIR is not writable"
	fi


	if [ -f $1 ]; then
		local FILENAME
		FILENAME=`basename $1`
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

		NEW_FILE_NAME=`printf %s_%s_%04d "$1" "$DATE_POSTFIX" $LAST_INDEX`

		cp $1 $NEW_FILE_NAME

		if [[ $? -ne 0 ]]; then
			printError "can't rename file: $1"
		fi
	fi
}


checkLoginDefs()
{
	local LD_SU_PATH=`egrep '^ENV_SUPATH\s+' /etc/login.defs | grep $DIR`
	local LD_PATH=`egrep '^ENV_PATH\s+' /etc/login.defs | grep $DIR`

	if [ -z "$LD_SU_PATH" -o -z "$LD_PATH" ]; then
		return 1
	fi

	return 0
}


checkConfig()
{
	egrep '^\s*export PATH' $1 | grep $DIR > /dev/null
}


addDirToConfig()
{
	if checkLoginDefs; then
		return
	fi

	if checkConfig /etc/profile; then
		return
	fi

	if checkConfig /etc/bash.bashrc; then
		return
	fi

	backupFile /etc/profile
	local DIR_VAR=`echo $DIR | sed -r 's/\//\\\\\//g'`

	sed -i -r "0,/^$/s/^$/export\ PATH=\$PATH:$DIR_VAR/" /etc/profile
}


validateParams "$@"
validateUser
prepareDir
addDirToConfig
cp $FILE $DIR
