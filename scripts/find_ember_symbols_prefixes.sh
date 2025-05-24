#!/bin/bash

# This script checks for the use of 'ember' and 'emAf' symbols in the codebase.
# It takes exclusion folders as arguments and can also read exclusions from a file.

# Function to check for 'ember' and 'emAf' symbols
check_symbols() {
    local exclusions=()
    while [[ $# -gt 0 ]]; do
        case $1 in
            --skip-subtree)
                exclusions+=(":(exclude)$2")
                shift 2
                ;;
            --skip-from-file)
                if [[ -f "$2" ]]; then
                    while IFS= read -r line || [[ -n "$line" ]]; do
                        [[ -z "$line" || "$line" =~ ^[[:space:]]*# ]] && continue
                        exclusions+=(":(exclude)$line")
                    done <"$2"
                else
                    echo "Warning: File '$2' not found, continuing without these exclusions"
                fi
                shift 2
                ;;
            --help)
                echo "Usage: $0 [--skip-subtree <dir>] [--skip-from-file <file>]"
                echo "  --skip-subtree <dir>     Skip the specified subtree or file from the search (can be used multiple times)"
                echo "  --skip-from-file <file>  Read paths to exclude from the specified file (one path per line)"
                echo "  --help                   Display this help message"
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

# Main script execution here
check_symbols "$@"
