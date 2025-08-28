#!/bin/bash

# This script checks for the use of 'SafeAttributePersistenceProvider' and 'AttributePersistenceProvider' 
# symbols in the codebase. It takes exclusion folders as arguments and can also read exclusions from a file.

# Function to check for SafeAttributePersistenceProvider and AttributePersistenceProvider symbols
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

    # Search for SafeAttributePersistenceProvider usage (class usage, not just includes)
    safe_provider_matches=$(git grep -I -n '\<SafeAttributePersistenceProvider\>' -- './*' "${exclusions[@]}" | grep -v '#include')

    # Search for AttributePersistenceProvider usage (class usage, not just includes)
    provider_matches=$(git grep -I -n '\<AttributePersistenceProvider\>' -- './*' "${exclusions[@]}" | grep -v '#include')

    if [[ -n "$safe_provider_matches" || -n "$provider_matches" ]]; then
        echo "Error: Found 'SafeAttributePersistenceProvider' or 'AttributePersistenceProvider' usage in the following files and lines:"
        if [[ -n "$safe_provider_matches" ]]; then
            echo ""
            echo "SafeAttributePersistenceProvider usage:"
            echo "$safe_provider_matches"
        fi
        if [[ -n "$provider_matches" ]]; then
            echo ""
            echo "AttributePersistenceProvider usage:"
            echo "$provider_matches"
        fi
        exit 1
    else
        echo "SUCCESS: No unauthorized usage of AttributePersistenceProvider or SafeAttributePersistenceProvider found."
    fi
}

# Main script execution here
check_symbols "$@"
