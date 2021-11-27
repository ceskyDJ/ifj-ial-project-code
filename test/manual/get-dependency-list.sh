#!/bin/bash

# Paths are relative from caller's directory
# $1 - Path to directory with manual tests
# $2 - Path to directory with object files
# $3 - Path to directory with binary files
# $4 - Name of target for filtering - only dependencies for this target will be written (optional)

# Manual test sources
TEST_P=$1
# Directories with obj files
OBJ_P=$2
# Directory for storing binary files
BIN_P=$3

# Generates dependencies using gcc (in format: manual test source file + needed header files)
function get_dependencies_from_gcc() {
  gcc -MM "$TEST_P"/test_*.c
}

# Trims whitespaces at the start of some rows
function trim_whitespaces() {
  sed -E 's/^ //'
}

# Format to one dependency per line (needed for filter_modules function)
function one_dep_per_line() {
  # 1. at first we need to have one dependency per row (sometimes there are more rules at one line divided by space)
  # 2. replace all suitable line breaks with special divider '|' (for case of unification to one line format)
  # 3. add divider to the end of each line, too (for future parsing and for finishing format unification)
  # 4. use divider to separate dependencies to individual rows for each of them (all lines has the same format, too)
  sed 's| '"$TEST_P"'/| \\\n'"$TEST_P"'/|' | sed ':a;N;$!ba;s/ \\\n/|/g' | sed 's/$/|/' | sed 's/|/|\n/g'
}

# Filter only valid modules (has *.c and *.h files) and does deduplication
function filter_modules() {
  # For storing read files for deduplication
  read_files=()

  while read -r line; do
    if echo "$line" | grep -q "$TEST_P/../../"; then
      path="${line//h|/c}"
      # We want it here, it's a hack for checking if the item is in array
      # shellcheck disable=SC2199
      # shellcheck disable=SC2076
      if [[ ! " ${read_files[@]} " =~ " ${path} " ]]; then
        if [ -f "$path" ]; then
          echo "${path//.c/.h}|"
        fi

        # Add file into list of read files
        read_files+=("$path")
      fi


    else
      echo "$line"

      # Reset file list for the new rule
      read_files=()
    fi
  done
}

# Converts from special deduplication format into the right format of Make rules (target: dependency1 dependency2 ...)
function parse_into_rules() {
  # First sed is inspired by: https://stackoverflow.com/a/1252191
  # Thanks to '|' marks it's easy to remove only the right EOLs
  # After that extra parts of paths are omitted
  sed ':a;N;$!ba;s/|\n/ /g' | sed -E 's|'"$TEST_P"'/../../[a-zA-Z]+/||g'
}

# Fixes some of the used paths
function fix_paths() {
  # 1. paths of header files (dependencies from src/) to paths of corresponding obj files (after compilation)
  # 2. paths to final linked obj file with compiled manual test for some module and all of its dependencies (targets)
  # 3. paths of manual test sources to paths of corresponding obj files (after compilation)
  sed -E 's|([a-zA-Z0-9_]+\.h)|'"$OBJ_P"'/\1|g' \
    | sed -E 's|test_([a-zA-Z0-9_]+)\.o|'"$BIN_P"'/\1.mantest|g' \
    | sed 's|'"$TEST_P"/'|'"$OBJ_P"/man'|'
}

# Set correct file extensions
function set_extensions() {
  # We're constructing rules for linker, so every file should be obj one
  # Replace *.c and *.h extensions to *.o extension
  sed -E 's/\.[ch]/.o/g'
}

# Get all rules for manual test linker
function get_all() {
  get_dependencies_from_gcc | trim_whitespaces | one_dep_per_line | filter_modules  | parse_into_rules | fix_paths \
    | set_extensions
}

# Control space
if [ $# -ge 4 ]; then
  # Optional argument supplied --> filter output to dependencies of specified target
  get_all | grep "$4" | sed -E "s/.+: //"
else
  # Without optional argument --> return all of the rules
  get_all
fi
