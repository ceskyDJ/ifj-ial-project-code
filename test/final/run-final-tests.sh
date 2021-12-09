#!/bin/bash
# Script for running final tests

# Move to this script folder
script="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
cd "$script" || exit 2

output=../../build/final-tests

fails=0
for test_file in test-*.ifj21; do
  input_file=${test_file//.ifj21/.in}
  test_name=${test_file//.ifj21/}

  echo -ne "\e[1;33mTest: \e[1;34m$test_name\e[1;33m"

  # Run with Teal original interpreter
  ./tl run "$test_file" <"$input_file" >"$output"/"$test_name".out.tl 2>"$output"/"$test_name".log.tl

  # Run with out compiler
  ../../build/bin/ifj21_compiler <"$test_file" >"$output"/"$test_name".ifjcode21 2>"$output"/"$test_name".log.ifj21
  ./ic21int "$output"/"$test_name".ifjcode21 <"$input_file" >"$output"/"$test_name".out.ifj21 2>"$output"/"$test_name".errs

  if diff "$output"/"$test_name".out.tl "$output"/"$test_name".out.ifj21 >"$output"/"$test_name".diff; then
    echo -e " [\e[32mPASSED\e[1;33m]\e[0m"
  else
    echo -e " [\e[31mFAILED\e[1;33m]\e[0m"

    echo -e "\t\e[1;33mDiffs:\e[0m"
    cat "$output"/"$test_name".diff | sed 's/^/\t/'

    echo -e "\n\t\e[1;33mInterpreter errors:\e[0m"
    cat "$output"/"$test_name".errs | sed 's/^/\t/'

    if [ ! "$CI" == "true" ]; then
      echo -e "\n\t\e[1;33mUseful files:\e[0m"
      echo -e "\t\t- Compiled sources (by ifj21_compiler): $output/$test_name.ifjcode21"
      echo -e "\t\t- Error output of ifj21_compiler: $output/$test_name.log.ifj21"
      echo -e "\t\t- Error output of Teal compiler: $output/$test_name.log.tl"
    fi

    ((fails+=1))
  fi
done

exit "$fails"
