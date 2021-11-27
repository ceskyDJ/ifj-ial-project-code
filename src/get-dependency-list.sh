#!/bin/bash

# Paths are relative from caller's directory
# $1 - Path to directory with source files
# $2 - Path to directory with object files

# Source files
SRC_P=$1
# Directories with obj files
OBJ_P=$2

# Generates dependencies using gcc (in format: obj file name (module after compilation) + needed header files)
function get_dependencies_from_gcc() {
  gcc -MM "$SRC_P"/*.c
}

# Trims whitespaces at the start of some rows
function trim_whitespaces() {
  sed -E 's/^ //'
}

# Removes line breaks added by gcc after some number of characters
function remove_additional_line_breaks() {
  sed ':a;N;$!ba;s/ \\\n/ /g'
}

# Paths are bad formed now (just file names), we need to add the right paths to the files
function fix_paths() {
  sed -E 's| ([^.]+\.[ch])| '"$SRC_P"'/\1|g' | sed -E 's|^([^.]+\.o)|'"$OBJ_P"'/\1|'
}

# Control space
get_dependencies_from_gcc | trim_whitespaces | remove_additional_line_breaks | fix_paths
