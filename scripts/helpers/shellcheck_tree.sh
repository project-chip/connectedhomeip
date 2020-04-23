#!/usr/bin/env bash

me=$(basename "$0")

# Run shellcheck on all the shell-ish files in the tree
patterns=('*.bash*' 'bash*'
  '*.ksh*' 'ksh*'
  '*.zsh*' 'zsh*'
  '.zlogin*' 'zlogin*'
  '.zlogout*' 'zlogout*'
  '.zprofile*' 'zprofile*'
  '*.sh' '.shlib*' 'shlib*'
  '.profile*' 'profile*'
  'suid_profile')

# excluding these directories (e.g. third_party/* )
excludes=()

# shellcheck disable=SC1091
[[ -f .shellcheck_tree ]] && . .shellcheck_tree

[[ ${*/--help//} != "${*}" ]] && {
  echo "Usage: $me <OPTIONS>

  Shellcheck all scripts in the tree.

  Options:
   --git      shellcheck via git ls-files, default is 'find .'
   --help     get this message

"
  exit 0
}

if [[ ${*/--git//} != "${*}" ]]; then
  #
  #  To run on git-files only
  #
  git ls-files -- "${patterns[@]}" | while read -r file; do
    for exclude in "${excludes[@]}"; do
      [[ $file =~ $exclude ]] && continue 2
    done
    shellcheck -f gcc "$file"
  done
else
  #
  # use find
  #
  exclude_args=()
  for exclude in "${excludes[@]}"; do
    exclude_args+=('!' -path "./$exclude" -a)
  done

  pattern_args=()
  for pattern in "${patterns[@]}"; do
    pattern_args+=(-o -name "$pattern")
  done

  find . "${exclude_args[@]}" '(' "${pattern_args[@]:1}" ')' -exec shellcheck -f gcc {} +
fi
