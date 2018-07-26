#!/bin/bash

file="task1.sh"

function echoerr() {
	echo "$*" 1>&2
}

function PrintHelp() {
	help_message="$scriptName [-h | --help] [file]\nDefault file - task1.sh"
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

function main() {
	installPath="/usr/local/bin"
	inp_args=$*
	ParseArgs $inp_args
	mkdir -p "${installPath}"
	if [[ ! "$?" -eq 0 ]]; then 
		echoerr "Failed to create dir - ${installPath}"
		exit 1
	fi

	cp "${file}" "${installPath}/"

	filename=$(basename "${file}")
	chmod a+rx "${installPath}/${filename}"

	isAdded=$(echo $PATH | grep -E ":?${installPath}:?")
	if [ -z "$isAdded" ]; then
		configFile=~/.bash_profile
		if [ -e "${configFile}" ]; then
			RenameIfExists "${configFile}" 0
		else
			touch ${configFile}
		fi
		
		echo "export PATH=${PATH}:${installPath}" >> "${configFile}"
	fi
}

main $*
