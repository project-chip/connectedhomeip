#!/bin/bash

# This script checks for the use of 'ember' and 'emAf' symbols in the codebase.
# It takes exclusion folders as arguments.

# Function to check for 'ember' and 'emAf' symbols
check_symbols() {
    local exclusions=()
    while [[ $# -gt 0 ]]; do
        case $1 in
            --skip-dir)
                exclusions+=(":(exclude)$2")
                shift 2
                ;;
            *)
                echo "Unknown option: $1"
                exit 1
                ;;
        esac
    done

    ember_matches=$(git grep -I -n '\<ember[A-Za-z0-9_]*' -- './*' "${exclusions[@]}" \
        ':(exclude).github/*' \
        ':(exclude)src/app/util/*' \
        ':(exclude)zzz_generated/app-common/app-common/zap-generated/access/Accessors.h' \
        ':(exclude)src/app/zap-templates/templates/app/attributes/Accessors-src.zapt')

    emAf_matches=$(git grep -I -n '\<emAf[A-Za-z0-9_]*' -- './*' "${exclusions[@]}" \
        ':(exclude).github/*' \
        ':(exclude)src/app/util/*' \
        ':(exclude)zzz_generated/app-common/app-common/zap-generated/access/Accessors.h' \
        ':(exclude)src/app/zap-templates/templates/app/attributes/Accessors-src.zapt')

    if [[ -n "$ember_matches" || -n "$emAf_matches" ]]; then
        echo "Error: Found 'ember' or 'emAf' symbols in the following files and lines:"
        [[ -n "$ember_matches" ]] && echo "$ember_matches"
        [[ -n "$emAf_matches" ]] && echo "$emAf_matches"
        exit 1
    fi
}

# Main script execution
check_symbols "$@"
