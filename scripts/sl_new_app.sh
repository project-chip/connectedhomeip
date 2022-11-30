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
	mkdir -p "$dest_path/$source_path"
	# Get absolute path of the destination
	current_path=$(pwd) && cd $dest_path && dest_path=$(pwd) &&	cd $current_path

	[[ $? -ne 0 ]] && echo "Please check the validity of the destination path \"$dest_path\"" && return 1
	
	echo "Start copy app from the \"$source_path\" to \"$dest_path\"."
	
	# Copy efr32 and appname-common of the selected app
	cp -PRvf "$source_path/efr32" "$source_path/$common_name" "$dest_path/$source_path"

	# Copy "./third_party/silabs/matter_support/matter/efr32/${efr32_family}/${efr32_board}"/config and autogen
	copy_path1="third_party/silabs/matter_support/matter/efr32/$efr32_family/$efr32_board"
	# Create the new destination path if it does not exist
	mkdir -p "$dest_path/$copy_path1"
	cp -PRvf "./$copy_path1/config" "./$copy_path1/autogen" "$dest_path/$copy_path1"

    # Copy "./third_party/silabs/efr32_sdk.gni"
	copy_path2="third_party/silabs"
	cp -PRvf "./$copy_path2/efr32_sdk.gni" "$dest_path/$copy_path2"

	# Copy "./third_party/zap/repo/zcl-builtin/shared/manufacturers.xml"
	copy_path5="third_party/zap/repo/zcl-builtin/shared"

	# Create the new destination path if it does not exist
	mkdir -p "$dest_path/$copy_path5"
	cp -PRvf "./$copy_path5/manufacturers.xml" "$dest_path/$copy_path5"
	
	# Copy zzz_generated directory of the copied app
	copy_path3="zzz_generated/$app_name"
	mkdir -p "$dest_path/$copy_path3"
	cp -PRvf "./$copy_path3/zap-generated" "$dest_path/$copy_path3"

	# Copy zap-templates for the copied app
	copy_path4="src/app"
	mkdir -p "$dest_path/$copy_path4"
	cp -PRvf "./$copy_path4/zap-templates" "$dest_path/$copy_path4"

	#1. Root directory of matter
	if [ -n "${MATTER_ROOT}" ]; then	
		root="${MATTER_ROOT}"
	else
		root=$(pwd)
	fi

	#2. symlink the buildoverride folder
	if [ -L "$dest_path/$source_path/efr32/build_overrides" ]; then
		unlink "$dest_path/$source_path/efr32/build_overrides"
	fi
	ln -s "${root}/examples/build_overrides" "$dest_path/$source_path/efr32/build_overrides"

	#3. symlink the third_party to point to the actual location of the matter repo
	if [ -L "$dest_path/$source_path/efr32/third_party/connectedhomeip" ]; then
		unlink "$dest_path/$source_path/efr32/third_party/connectedhomeip"
	fi	
	ln -s "${root}" "$dest_path/$source_path/efr32/third_party/connectedhomeip"

	#4. Modify app/efr32/BUILD.gn and app/app-common/BUILD.gn to correct paths corresponding to coppied app
	# Modification in app/efr32/BUILD.gn
	sed -i.bak -e "s+import(\"\${efr32_sdk_build_root}/efr32_sdk.gni\")+import(\"../../../$copy_path2/efr32_sdk.gni\")+" -- "$dest_path/$source_path/efr32/BUILD.gn"
	sed -i.bak -e "s+\"\${chip_root}/examples/lighting-app/efr32\"+\"$dest_path/$source_path/efr32\"+" -- "$dest_path/$source_path/efr32/BUILD.gn"
	sed -i.bak -e "s+\"\${chip_root}/examples/lighting-app/lighting-common\"+\"$dest_path/$source_path/$common_name\"+" -- "$dest_path/$source_path/efr32/BUILD.gn" &&
	rm -- "$dest_path/$source_path/efr32/BUILD.gn.bak"
	# Modification in app/app-common/BUILD.gn
	sed -i.bak -e "s+\"\${chip_root}/zzz_generated/lighting-app/zap-generated\"+\"../../../zzz_generated/$app_name/zap-generated\"+" -- "$dest_path/$source_path/$common_name/BUILD.gn" &&
	rm -- "$dest_path/$source_path/$common_name/BUILD.gn.bak"

	#5. Modify efr32_sdk.gni
	# Correct the import path
	sed -i.bak -e "s+import(\"silabs_board.gni\")+import(\"../../$source_path/efr32/third_party/connectedhomeip/$copy_path2/silabs_board.gni\")+" -- "$dest_path/$copy_path2/efr32_sdk.gni"
	# Correct path to the coppied app
    sed -i.bak -e "s+\"\${sdk_support_root}/matter/efr32/\${silabs_family}/\${silabs_board}/config\"+\"$dest_path/$copy_path2/matter_support/matter/efr32/\${silabs_family}/\${silabs_board}/config\"+" -- "$dest_path/$copy_path2/efr32_sdk.gni"
	sed -i.bak -e "s+\"\${sdk_support_root}/matter/efr32/\${silabs_family}/\${silabs_board}/autogen\"+\"$dest_path/$copy_path2/matter_support/matter/efr32/\${silabs_family}/\${silabs_board}/autogen\"+" -- "$dest_path/$copy_path2/efr32_sdk.gni"
	# Import the newly created file customized.gni
	sed -i.bak -e "s+declare_args() {+import(\"customized.gni\")+" -- "$dest_path/$copy_path2/efr32_sdk.gni"
	# Delete range of text
	sed -i.bak -e "/# Location of the efr32 SDK./{N;N;N;N;N;N;N;N;N;N;N;N;N;N;d;}" -- "$dest_path/$copy_path2/efr32_sdk.gni" &&
	rm -- "$dest_path/$copy_path2/efr32_sdk.gni.bak"

	#6. Create a customized.gni and move arg declaration from efr32_sdk.gni
	# Copy text from line 1~38 of file efr32_sdk.gni and write to file customized.gni (new created file)
	sed -n "1,38 p" "./$copy_path2/efr32_sdk.gni" > "$dest_path/$copy_path2/customized.gni"
	# Delete line 16~22 and 38 of newly created customized.gni
	sed -i.bak -e "16,22d;38d" -- "$dest_path/$copy_path2/customized.gni"
	# Replace the patern with a blank line
	sed -i.bak -e "s/declare_args() {/\n/" -- "$dest_path/$copy_path2/customized.gni"
	# Remove leading spaces of all text lines
	sed -i.bak -e "s/^[ \t]*//" -- "$dest_path/$copy_path2/customized.gni" &&
	rm -- "$dest_path/$copy_path2/customized.gni.bak"
    
	[[ $? -eq 0 ]] && echo "Creation of selected app from matter to destination folder \"$dest_path\" is done."
}

main