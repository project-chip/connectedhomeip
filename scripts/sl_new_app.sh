#!/bin/bash

source_path=""
source_path1=""
source_path2=""
dest_path=""
board_name=""
board_family=""
app_name=""
common_name=""
root=""
temp=""
temp1=""

EFR32MG12_boards=("BRD4161A" "BRD4162A" "BRD4163A" "BRD4164A" "BRD4166A" "BRD4170A" "BRD4304A")
EFR32MG24_boards=("BRD2601B" "BRD2703A" "BRD4186A" "BRD4186C" "BRD4187C" "BRD4187A" "BRD4187C")
MGM24_boards=("BRD2704A" "BRD4317A" "BRD4318A" "BRD4319A" "BRD4316A" "BRD4325B")

function Print_Help {
    echo "This bash script is to create a new app according to source sample app, board and path to the new project directory."
    echo "Usage:"
    echo "        sl_new_app.sh -h"
    echo "        sl_new_app.sh -s <sourcepath> -d <destpath> -b <board>"
    echo
    echo "Available options:"
    echo "        -h   Print this help."
    echo "        -s   Source directory containing application that is coppied to the destination."
    echo "        -d   Destination directory containing the copied application."
    echo "        -b   The board that the new app builds for."
}

# Get arguments and store in corresponding variables
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
			board_name=${OPTARG}
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
	if [[ -n "${board_name}"  ]];
	then
		for i in "${EFR32MG24_boards[@]}"
		do
		    if [[ "${i}" == "${board_name}"  ]]; then
				board_family="efr32mg24"
				echo "${board_family}"
	       		return  0
	       fi
		done

        for j in "${MGM24_boards[@]}"
		do
			if [[ "${j}" == "${board_name}"  ]]; then
				board_family="mgm24"
				echo "${board_family}"
	            return 0
		    fi
		done

		for k in "${EFR32MG12_boards[@]}"
		do
		    if [[ "${k}" == "${board_name}"  ]]; then
				board_family="efr32mg12"
				echo "${board_family}"
	            return 0
		    fi
		done
	fi
	return 1
}

function main {
	# check if the using format of the script is wrong
	[[ -z "${source_path}" || -z "${dest_path}" || -z "${board_name}" ]] && echo "Wrong format" && Print_Help && return 1
		
	# Check if the board is supported
	if ! check_board;
	then
		echo  "The board is not supported"
		Print_Help
		return 1
	fi
	# Check if the source path is a directory and exist
	[[ ! -d "${source_path}"  ]] && echo "The source path is incorrect!" && Print_Help && return 1

	# Get the name of the selected app
	source_path1=$(dirname "${source_path}")
	echo "${source_path1}"
	app_name=$(basename "${source_path1}")
	echo "${app_name}"

	# create the common directory name from provided app name
    if [[ "${app_name: -4}" -ne "-app" ]]; then
		common_name="${app_name}-common" 
	else 
		common_name="${app_name:0:${#app_name}-4}-common"
	fi

	# Create the new destination path if it does not exist
	mkdir -p "${dest_path}/${source_path}"
	# Get absolute path of the destination
	current_path=$(pwd) && cd "${dest_path}" && dest_path=$(pwd) &&	cd "${current_path}"

	[[ $? -ne 0 ]] && echo "Please check the validity of the destination path \"${dest_path}\"" && return 1
	
	echo "Copying application from  \"${source_path}\" to \"${dest_path}\"."
	
	# Copy app and appname-common of the selected app
	source_path2=$(dirname "${source_path}")
	cp -PRvf "${source_path}" "${dest_path}/${source_path2}"
	cp -PRvf "${source_path1}/${common_name}" "${dest_path}/${source_path1}"

	# Copy "./third_party/silabs/matter_support/matter/efr32/${board_family}/${board_name}"/config and autogen
	copy_path1="third_party/silabs/matter_support/matter/efr32/${board_family}/${board_name}"
	# Copy "./third_party/silabs/slc_gen/\${silabs_board}"/config and autogen
	copy_path5="third_party/silabs/slc_gen/${board_name}"

	# Create the new destination path if it does not exist
	mkdir -p "${dest_path}/${copy_path1}"
	mkdir -p "${dest_path}/${copy_path5}"
	cp -PRvf "${copy_path1}/autogen" "./${copy_path1}/config" "${dest_path}/${copy_path1}"
	cp -PRvf "${copy_path5}/autogen" "./${copy_path5}/config" "${dest_path}/${copy_path5}"

    # Copy "./third_party/silabs/efr32_sdk.gni"
	temp="efr32_sdk.gni"
	if [[ "${board_family}" == "mgm24" ]]; then
		temp="SiWx917_sdk.gni"
	fi

	copy_path2="third_party/silabs"
	cp -PRvf "./${copy_path2}/${temp}" "${dest_path}/${copy_path2}"

	#1. Root directory of matter
	if [[ -n "${MATTER_ROOT}" ]]; then
		root="${MATTER_ROOT}"
	else
		root=$(pwd)
	fi

	#2. symlink the buildoverride folder
	if [[ -L "${dest_path}/${source_path}/build_overrides" ]]; then
		unlink "${dest_path}/${source_path}/build_overrides"
	fi
	ln -s "${root}/examples/build_overrides" "${dest_path}/${source_path}/build_overrides"

	#3. symlink the third_party to point to the actual location of the matter repo
	if [[ -L "${dest_path}/${source_path}/third_party/connectedhomeip" ]]; then
		unlink "${dest_path}/${source_path}/third_party/connectedhomeip"
	fi	
	ln -s "${root}" "${dest_path}/${source_path}/third_party/connectedhomeip"

	#4. Modify app BUILD.gn and app-common BUILD.gn to correct paths corresponding to copied app
	temp="efr32_sdk.gni"
	temp1="silabs"
	if [[ "${board_family}" == "mgm24" ]]; then
		temp="SiWx917_sdk.gni"
		temp1="silabs/SiWx917"
	fi
	# Modification in app BUILD.gn
	sed -i.bak -e "s+import(\"\${efr32_sdk_build_root}/${temp}\")+import(\"../../../../${copy_path2}/${temp}\")+" -- "${dest_path}/${source_path}/BUILD.gn"
	sed -i.bak -e "s+\"\${chip_root}/examples/${app_name}/${temp1}\"+\"${dest_path}/${source_path1}/${temp1}\"+" -- "${dest_path}/${source_path}/BUILD.gn"
	sed -i.bak -e "s+import(\"../../../../third_party/silabs/efr32_sdk.gni\")+import(\"${dest_path}/third_party/silabs/efr32_sdk.gni\")+" -- "${dest_path}/${source_path}/BUILD.gn"
	sed -i.bak -e "s+\"\${chip_root}/examples/${app_name}/${common_name}\"+\"${dest_path}/${source_path1}/${common_name}\"+" -- "${dest_path}/${source_path}/BUILD.gn" &&
	rm -- "${dest_path}/${source_path}/BUILD.gn.bak"

	# Modification in app-common BUILD.gn
	sed -i.bak -e "s+\"\${chip_root}/zzz_generated/${app_name}/zap-generated\"+\"../../../zzz_generated/${app_name}/zap-generated\"+" -- "${dest_path}/${source_path1}/${common_name}/BUILD.gn" &&
	rm -- "${dest_path}/${source_path1}/${common_name}/BUILD.gn.bak"

	#5. Modify efr32_sdk.gni or SiWx917_sdk.gni correspondingly
	# Correct the import path
	sed -i.bak -e "s+import(\"silabs_board.gni\")+import(\"../../${source_path}/third_party/connectedhomeip/${copy_path2}/silabs_board.gni\")+" -- "${dest_path}/${copy_path2}/${temp}"
	# Correct path to the coppied app
	sed -i.bak -e "s+\"\${chip_root}/third_party/silabs/slc_gen/\${silabs_board}/\"+\"${dest_path}/${copy_path2}/slc_gen/\${silabs_board}/\"+" -- "${dest_path}/${copy_path2}/${temp}"
	sed -i.bak -e "s+\"\${sdk_support_root}/matter/efr32/\${silabs_family}/\${silabs_board}/\"+\"${dest_path}/${copy_path2}/matter_support/matter/efr32/\${silabs_family}/\${silabs_board}/\"+" -- "$dest_path/$copy_path2/$temp"

	# Fixing for building
	declare -i num_of_curly_braces=0
	declare -i line_number=0
	declare -i x=0
	gni_filename="${dest_path}/${copy_path2}/${temp}"
	while IFS= read -r line
	do 
		((line_number++))
		if [ "$line" = "}" ] && [[ "$num_of_curly_braces" -gt 0 ]]; then
			sed -i.bak -e "$(expr ${line_number} - $x) d" -- "$gni_filename"
			((num_of_curly_braces--))
			((x++))
		elif [ "$line" = "declare_args() {" ]; then
			((num_of_curly_braces++))
			sed -i.bak -e "$(expr ${line_number} - $x) d" -- "$gni_filename"
			((x++))
		fi 
	done < "${gni_filename}"

	rm -- "${dest_path}/${copy_path2}/${temp}.bak"

	[[ $? -eq 0 ]] && echo "Creation of selected app from matter to destination folder \"${dest_path}\" is done."
}

main