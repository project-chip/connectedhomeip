#!/bin/bash

# This script checks for the use of 'ember' and 'emAf' symbols in the codebase.
# It takes exclusion folders as arguments.

# Function to check for 'ember' and 'emAf' symbols
check_symbols() {
    local exclusions=()
    while [[ $# -gt 0 ]]; do
        case $1 in
        --skip-subtree)
            exclusions+=(":(exclude)$2")
            shift 2
            ;;
        --help)
            echo "Usage: $0 [--skip-subtree <dir>]"
            echo "  --skip-subtree <dir>  Skip the specified subtree or file from the search (can be used multiple times for different trees/files)"
            echo "  --help            Display this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
        esac
    done

    ember_matches=$(git grep -I -n '\<ember[A-Za-z0-9_]*(' -- './*' "${exclusions[@]}")

    emAf_matches=$(git grep -I -n '\<emAf[A-Za-z0-9_]*(' -- './*' "${exclusions[@]}")

    if [[ -n "$ember_matches" || -n "$emAf_matches" ]]; then
        echo "Error: Found 'ember' or 'emAf' symbols in the following files and lines:"
        [[ -n "$ember_matches" ]] && echo "$ember_matches"
        [[ -n "$emAf_matches" ]] && echo "$emAf_matches"
        exit 1
    fi
}

# Main script execution
check_symbols "$@"
