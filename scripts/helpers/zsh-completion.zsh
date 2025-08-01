#!/usr/bin/env zsh

#
# build_examples.py completion functions
#

function _matter_build_targets() {
  compadd -S '' -- $(${words[1]} targets --format completion ${words[CURRENT]})
}


function _matter_build_examples() {

  _arguments : \
    "--help[Print help]" \
    "--log-level[Determines the verbosity of script output]:log_level:(debug info warn fatal)" \
    "--verbose[Pass verbose flag to ninja]" \
    "*--target[Define a build target]:target:_matter_build_targets" \
    "--enable-link-map-file[Enable generation of link map files]" \
    "--enable-flashbundle[Also generate the flashbundles for the app]" \
    "--repo[Path to the root of the CHIP SDK repository checkout]:directory:_files -/" \
    "--out-prefix[Prefix for the generated file output]:directory:_files -/" \
    "--ninja-jobs[Number of ninja jobs]" \
    "--pregen-dir[Directory where generated files have been pre-generated]:directory:_files -/" \
    "--clean[Clean output directory before running the command]" \
    "--dry-run[Only print out shell commands that would be executed]" \
    "--dry-run-output[Where to write the dry run output]:directory:_files -/" \
    "--no-log-timestamps[Skip timestaps in log output]" \
    "--pw-command-launcher[Set pigweed command launcher]:pw_command_launcher:(ccache)" \
    "1:command:->cmds" \
    "*::arg:->args" \
    && return

    case "$state" in
      cmds)
        _values "command" \
          "build[Generate and run ninja/make as needed to compile]" \
          "gen[Generate ninja/makefiles (but does not run the compilation)]" \
          "targets[List the targets that can be used with the build and gen commands]"
        ;;
      args)
        case "$line[1]" in
          build)
            _arguments : \
              "--help[Print help]" \
              "--copy-artifacts-to[Prefix for the generated file output]:directory:_files -/" \
              "--create-archives[Prefix of compressed archives of the generated files]:directory:_files -/" \
              && return
            ;;
          gen)
            _arguments : \
              "--help[Print help]" \
              && return
            ;;
          targets)
            _arguments : \
              "--help[Print help]" \
              "--format:targets_format:(summary expanded json completion)" \
              && return
            ;;
        esac
        ;;
    esac
}


compdef _matter_build_examples scripts/build/build_examples.py

#
# chip-tool completion functions
#


#
# Copied from scripts/helpers/bash-completion.sh
#

# Get the list of commands from the output of the chip-tool,
# where each command is prefixed with the ' | * ' string.
function _matter_chip_tool_commands() {
  local -a commands=(${(f)"$("${@}" --help 2>&1 | awk '/ [|] [*] /{ print $3 }')"})
  compadd -- "${commands[@]}"
}


# Get the list of options from the output of the chip-tool,
# where each option starts with the '[--' string.
function _matter_chip_tool_command_options() {
  local -a options=( ${(f)"$("$@" --help 2>&1 | awk -F'[[]|[]]' '/^[[]--/{ print $2 }')"} )
  compadd -- "${options[@]}"
}


function _matter_chip_tool() {
  local -a orig_words=("${words[@]}")

  _arguments : \
    "--help[Print help]" \
    "1:command:->cmds" \
    "2:subcommand:->subcmds" \
    "*::arg:->args" \
    && return

    case "$state" in
      cmds)
        _matter_chip_tool_commands "${words[@]}"
        ;;
      subcmds)
        _matter_chip_tool_commands "${words[@]}"
        ;;
      args)
        _matter_chip_tool_command_options "${orig_words[@]}"
        ;;
    esac
}


compdef _matter_chip_tool chip-tool