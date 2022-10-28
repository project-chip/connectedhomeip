#!/bin/bash

source_path=""
dest_path=""
efr32_board=""
efr32_family=""
app_name=""
common_name=""
root=""

MG12_boards=("BRD4161A" "BRD4162A" "BRD4163A" "BRD4164A" "BRD4166A" "BRD4170A")
MG24_boards=("BRD4186C" "BRD4187C" "BRD2703A" "BRD2601B" "BRD4316A" "BRD4317A" "BRD4319A")

function Print_Help {
    echo "This bash script is to create a new app according to source sample app, board and path to the new project directory."
    echo "Usage:"
    echo "        newlightapp -h"
    echo "        newlightapp -s <sourcepath> -d <destpath> -b <board>"
    echo
    echo "Available options:"
    echo "        -h   Print this help."
    echo "        -s   Source directory containing application that is coppied to the destination."
    echo "        -d   Destination directory containing the copied application."
    echo "        -b   The board that the new app builds for."
}

while getopts "s:d:b:h" FLAG
do
	case "${FLAG}" in
		s)
			source_path=${OPTARG}
			;;
		d)
			dest_path=${OPTARG}
			;;
		b)
			efr32_board=${OPTARG}
			;;
		h)
			Print_Help
			;;
		*)
			Print_Help
			exit 1
			;;
	esac
done

# If the board is supported, function returns 0, otherwise 1 is returned
function check_board {
	if [[ -n "$efr32_board"  ]];
	then
		for i in "${MG24_boards[@]}"
		do
		    if [[ "$i" == "$efr32_board"  ]]; then
				efr32_family="efr32mg24"
	       		return  0
	       fi
		done

		for k in "${MG12_boards[@]}"
		do
		    if [[ "$k" == "$efr32_board"  ]]; then
				efr32_family="efr32mg12"
	            return 0
		    fi
		done
	fi
	return 1
}

function main {
	# check if the using format of the script is wrong
	[[ -z "$source_path" || -z "$dest_path" || -z "$efr32_board" ]] && echo "Wrong format" && Print_Help && return 1
		
	# Check if the board is supported
	if ! check_board;
	then
		echo  "The board is not supported"
		Print_Help
		return 1
	fi
	# Check if the source path is a directory and exist
	[[ ! -d "$source_path"  ]] && echo "The source path is incorrect!" && Print_Help && return 1

	# Get the name of the selected app
	app_name=$(basename $source_path)
	
	# create the common directory name from provided app name
    if [[ "${app_name: -4}" != "-app" ]]; then 
		common_name="${app_name}-common" 
	else 
		common_name="${app_name:0:${#app_name}-4}-common"
	fi

	# Create the new destination path if it does not exist
	mkdir -p "$dest_path/$app_name"

	[[ $? -ne 0 ]] && echo "Please check the validity of the destination path \"$dest_path\"" && return 1
	
	echo "Start copy app from the \"$source_path\" to \"$dest_path\"."

	# Copy efr32 and appname-common of the selected app
	cp -PRvf "$source_path/efr32" "$source_path/$common_name" "$dest_path/$app_name"
	
	#1. Root directory of matter
	if [ -n "${MATTER_ROOT}" ]; then	
		root="${MATTER_ROOT}"
	else
		root=$(pwd)
	fi

	#2. symlink the buildoverride folder
	if [ -L "$dest_path/$app_name/efr32/build_overrides" ]; then
		unlink "$dest_path/$app_name/efr32/build_overrides"
	fi
	ln -s "${root}/examples/build_overrides" "$dest_path/$app_name/efr32/build_overrides"

	#3. symlink the third_party to point to the actual location of the matter repo
	if [ -L "$dest_path/$app_name/efr32/third_party/connectedhomeip" ]; then
		unlink "$dest_path/$app_name/efr32/third_party/connectedhomeip"
	fi	
	ln -s "${root}" "$dest_path/$app_name/efr32/third_party/connectedhomeip"
    
	[[ $? -eq 0 ]] && echo "Creation of selected app from matter to destination folder \"$dest_path\" is done."
}

main
