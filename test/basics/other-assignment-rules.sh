#!/bin/bash

cd tmp || { echo "Cannot move to directory: ./tmp" >&2; exit 1; }

total_errors=0

function echo_result() {
  # Usage:
  # - $1: Test number
  # - $2: Test name

  if [[ $? -eq 0 ]]; then
    result="Successful"
    color="\e[32m"
  else
    result="Failed"
    color="\e[31m"

    # Error counter
    total_errors=$(( total_errors + 1 ))
  fi

  echo -e "${color}[${1}] $2 | ${result}\e[0m"
}

# symtable.c, symtable.h
[[ -f symtable.c ]] && [[ -f symtable.h ]]; echo_result 1 "Required files: symtable.c and symtable.h"

# Forbidden files
allowed_extensions=("c" "h")
error=0
for file in *; do
  # Source: https://stackoverflow.com/a/965069
  extension="${file#*.}"
  if [[ $file == "Makefile" ]]; then
    continue
  elif [[ $file == "dokumentace.pdf" ]]; then
    continue
  elif [[ $file == "rozdeleni" ]]; then
    continue
  elif [[ $file == "rozsireni" ]]; then
    continue
  elif [[ $file == "xsmahe01.tar.gz" ]]; then
    continue
  elif [[ "${allowed_extensions[*]}" =~ $extension ]]; then
    # Source: https://stackoverflow.com/a/15394738
    continue
  fi

  error=1
  forbidden_files+=("$file")
done
[[ $error -eq 0 ]]; echo_result 2 "Forbidden files. Allowed are only: *.c, *.h, rozdeleni, rozsireni, Makefile, xsmahe01.tar.gz dokumentace.pdf"
[[ ! $error -eq 0 ]] && echo -e "\tFound forbidden files: ${forbidden_files[*]}"

# File name convention
error=0
for file in *; do
  if echo "$file" | grep -qE "^[a-zA-Z0-9_.]+$"; then
    continue
  fi

  error=1
  bad_named_files+=("$file")
done
[[ $error -eq 0 ]]; echo_result 3 "Bad named files. This chars can be used only: english alphabet, numbers, underscores (_), dots (.)"
[[ ! $error -eq 0 ]] && echo -e "\tFound bad named files: ${forbidden_files[*]}"

# Required file header
error=0
for file in *; do
  # Source: https://stackoverflow.com/a/965069
  extension="${file#*.}"
  # Skip every file except Makefile, *.c and *.h (different structure)
  if [[ $file != "Makefile" ]] && [[ ! "${allowed_extensions[*]}" =~ $extension ]]; then
    # Condition's second part source: https://stackoverflow.com/a/15394738
    continue
  fi

  if [[ "$file" != "Makefile" ]]; then
    # C files (*.c and *.h)
    grep -qFi " * @file" "$file" \
      && grep -qF " * IFJ and IAL project (IFJ21 compiler)" "$file" \
      && grep -qF " * Team: 128 (variant II)" "$file" \
      && grep -qFi " * @author " "$file" \
      && continue
  else
    grep -qF "# IFJ and IAL project (IFJ21 compiler)" "$file" \
      && grep -qF "# Team: 128 (variant II)" "$file" \
      && grep -qFi "# Author: " "$file" \
      && continue
  fi

  error=1
  files_without_header+=("$file")
done
[[ $error -eq 0 ]]; echo_result 4 "Files without required header. See CODE_STYLE.md for more info"
[[ ! $error -eq 0 ]] && echo -e "\tFound files without header (or with bad header, check CODE_STYLE.md): ${files_without_header[*]}"

exit "$total_errors"
