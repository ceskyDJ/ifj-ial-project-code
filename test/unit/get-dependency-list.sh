#!/bin/bash

# Paths are relative from caller's directory
# $1 - Path to directory with unit tests
# $2 - Path to directory with object files
# $3 - Name of target for filtering - only dependencies for this target will be written (optional)

# Unit test sources
TEST_P=$1
# Directories with build data
OBJ_P=$2

# Generates dependencies using gcc (in format: unit test source file + needed header files)
function get_dependencies_from_gcc() {
  gcc -MM "$TEST_P"/test_*.c
}

# Trims whitespaces at the start of some rows
function trim_whitespaces() {
  sed -E 's/^ //'
}

# Removes Unity framework files (these are static and are resolved elsewhere)
function remove_unity_stuff() {
  # Inverted grep is used here (when the rule is accepted, the row is thrown away)
  grep -Ev "unity(_internals)?.h"
}

# Applies deduplication process (some of header files are generated twice or more)
function deduplicate() {
  # 1. at first we need to have one dependency per row (sometimes there are more rules at one line divided by space)
  # 2. replace all suitable line breaks with special divider '|' (for case of unification to one comparable line format)
  # 3. add divider to the end of each line, too (for future parsing and for finishing format unification)
  # 4. use divider to separate dependencies to individual rows for each of them (all lines has the same format, too)
  # 5. do the deduplication itself
  sed 's| '"$TEST_P"'/| \\\n'"$TEST_P"'/|' | sed ':a;N;$!ba;s/ \\\n/|/g' | sed 's/$/|/' | sed 's/|/|\n/g' | uniq
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
  # 2. paths to final linked obj file with compiled unit test for some module and all of its dependencies (targets)
  # 3. paths of unit test sources to paths of corresponding obj files (after compilation)
  sed -E 's|([a-zA-Z0-9_]+\.h)|'"$OBJ_P"'/\1|g' \
    | sed -E 's|([a-zA-Z0-9_]+)(\.o)|'"$OBJ_P"'/\1_final\2|g' \
    | sed -E 's|'"$OBJ_P"'/([a-zA-Z0-9_]+\.c)|'"$OBJ_P"'/\1|g' \
    | sed 's|'"$TEST_P"'|'"$OBJ_P"'|'
}

# Set correct file extensions
function set_extensions() {
  # We're constructing rules for linker, so every file should be obj one
  # Replace *.c and *.h extensions to *.o extension
  sed -E 's/\.[ch]/.o/g'
}

# Get all rules for unit test linker
function get_all() {
  get_dependencies_from_gcc | trim_whitespaces | remove_unity_stuff | deduplicate | parse_into_rules | fix_paths \
    | set_extensions
}

# Control space
if [ $# -ge 3 ]; then
  # Optional argument supplied --> filter output to dependencies of specified target
  get_all | grep "$3" | sed -E "s/.+: //"
else
  # Without optional argument --> return all of the rules
  get_all
fi
