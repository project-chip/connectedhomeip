#!/bin/bash -e

CHIP_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../../"

finish() {
    rc=$?
    if "${CI:-false}" && [[ $rc -eq 1 ]]; then
        echo "**********************************************************************************"
        echo "Copyright year is incorrect in one or more files and can be auto-fixed by running:"
        echo ""
        echo "$0 -c"
        echo "**********************************************************************************"
    fi
}
trap finish EXIT

usage() {
    echo "A tool to find and fix copyright years."
    echo ""
    echo "Usage: $0 [options]"
    echo ""
    echo "OPTIONS:"
    echo "-f  - Check the provided file. If no option is provided all files will be checked."
    echo "-d  - Check the files in the specified directory."
    echo "-c  - Correct copyright year issue."
    echo ""
    echo "EXAMPLES: "
    echo "$0 -f <file_name>            # Check the specified file"
    echo "$0 -f <file_name> -f <file2> # Check the two specified files"
    echo "$0 -f <file_name> -c         # Check and fix the specified file"
    echo "$0 -d <dir_name>             # Check files in the specified directory"
    echo "$0 -d <dir_name> -c          # Check and correct all files in the specified directory"
    echo "$0                           # Check all files in this repository"
    echo "$0 -c                        # Correct all files in this repository"
    exit 1
}

CORRECT=false
SEARCH_PATH=$CHIP_ROOT
SOURCE_FILES=()

while getopts "hcf:d:" opt; do
    case $opt in
        f) SOURCE_FILES+=("$OPTARG") ;;
        d) SEARCH_PATH="$OPTARG" ;;
        c) CORRECT=true ;;
        h) usage ;;
        \?) usage ;;
    esac
done

if [ ${#SOURCE_FILES[@]} -gt 0 ]; then
    for file in "${SOURCE_FILES[@]}"; do
        if [[ ! -f "$file" ]]; then
            echo "Skipping invalid file '$file' (does not exist)"
            continue
        fi
    done
else
    while IFS= read -r -d $'\0'; do
        SOURCE_FILES+=("$REPLY")
    done < <(
        find "$SEARCH_PATH" \
            -not -path "*/\.git/*" \
            -not -path "*/\.environment/*" \
            -not -path "*third_party*" \
            -not -path "*scripts/helpers/check_copyright_years.sh" \
            ! -type l \
            -type f \
            -print0
    )
fi

echo "Looking for files with incorrect copyright years..."
RESULT=0
COPYRIGHT_REGEX="Copyright \(c\) (([0-9]{4})(-[0-9]{4})?) Project CHIP Authors"
for SOURCE_FILE in "${SOURCE_FILES[@]}"; do
    # Find the year in which the file was most recently modified
    LAST_MODIFIED=$(git log --follow --format=%aD -- "$SOURCE_FILE" | head -1 | awk '{print $4}')

    # Find copyright year listed in the file
    COPYRIGHT_LINE=$(grep -i "Project CHIP Authors" "$SOURCE_FILE" || true)
    if [[ $COPYRIGHT_LINE =~ $COPYRIGHT_REGEX ]]; then
        COPYRIGHT_YEAR=${BASH_REMATCH[1]}
    else
        # Skip the file if it has no copyright
        continue
    fi

    # Check if the copyright years don't match
    if [[ ! "$COPYRIGHT_YEAR" =~ .*"$LAST_MODIFIED".* ]]; then
        # Correct the copyright year if requested by the user
        if ("$CORRECT"); then
            echo "Correcting file $SOURCE_FILE"
            START_YEAR=$(echo "$COPYRIGHT_YEAR" | awk -F- '{print $1}')
            if [[ "$(uname)" == "Darwin" ]]; then
                sed -i '' -E 's/([0-9]{4})(-[0-9]{4})? Project CHIP Authors/'"$START_YEAR-$LAST_MODIFIED"' Project CHIP Authors/g' "$SOURCE_FILE"
            else
                sed -i -E 's/([0-9]{4})(-[0-9]{4})? Project CHIP Authors/'"$START_YEAR-$LAST_MODIFIED"' Project CHIP Authors/g' "$SOURCE_FILE"
            fi

        else
            echo "Incorrect copyright year in $SOURCE_FILE. Modified Year: $LAST_MODIFIED Copyright Year: $COPYRIGHT_YEAR"
            RESULT=1
        fi
    fi
done

if [[ "$RESULT" -eq 0 ]] && ! ("$CORRECT"); then
    echo "***************"
    echo "No issues found"
    echo "***************"
    exit 0
fi

exit "$RESULT"
