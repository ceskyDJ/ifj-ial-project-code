#!/bin/bash
# Script for running error tests (checks correct exit code for source code containing some mistake)
# If your script needs some input, you need to create file test-XX.in (test-XX.ifj21) and put the input in there

# Move to this script folder
script="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
cd "$script" || exit 2

# Build compiler's binary
cd ../../src/ && make || exit 2
cd "$script" || exit 2

output=../../build/error-tests

fails=0
for test_file in test-*.ifj21; do
  input_file=${test_file//.ifj21/.in}
  test_name=${test_file//.ifj21/}

  echo -ne "\e[1;33mTest: \e[1;34m$test_name\e[1;33m"

  # Compile test
  ../../build/bin/ifj21_compiler <"$test_file" >"$output/$test_name.ifjcode21" 2>"$output/$test_name.log.ifj21"
  exit_code=$?

  # Run compiled test with ic21int
  if [ $exit_code == 0 ]; then
    ./ic21int "$output/$test_name.ifjcode21" <"$input_file" >/dev/null 2>"$output/$test_name.log.ic21int"
    exit_code=$?
    detected_by="interpreter"
  else
    detected_by="compiler"
  fi

  # Get correct (wanted) exit code from comment on the first line of the test script
  correct_exit_code=$(head "$test_file" -n 1 | grep -oE -- '-- [0-9][0-9]?' | sed 's/-- //')

  # Everything ok --> test passed
  if [[ $exit_code == "$correct_exit_code" ]]; then
    echo -e " [\e[32mPASSED\e[1;33m]\e[0m"
    continue
  fi

  # Result code is different --> test failed
  echo -e " [\e[31mFAILED\e[1;33m]\e[0m"

  # Compiler nor interpreter didn't detect the error --> test failed
  if [[ $exit_code == 0 ]]; then
    echo -e "\t\e[1;33mDetails:\e[0m ifj21_compiler didn't find the error and nothing was detected in runtime"

    echo -e "\n\t\e[1;33mCorrect exit code:\e[0m $correct_exit_code"
  fi

  # Compiler or interpreter returned bad exit code
  if [[ $exit_code != 0 ]]; then
        echo -e "\n\t\e[1;33mResult exit code:\e[0m $exit_code (by $detected_by)"
        echo -e "\t\e[1;33mCorrect exit code:\e[0m $correct_exit_code"
  fi

  if [ ! "$CI" == "true" ]; then
    echo -e "\n\t\e[1;33mUseful files:\e[0m"
    echo -e "\t\t- Compiled sources (by ifj21_compiler): $output/$test_name.ifjcode21"
    echo -e "\t\t- Error output of ifj21_compiler: $output/$test_name.log.ifj21"
    echo -e "\t\t- Error output of ic21int: $output/$test_name.log.ic21int"
  fi

  ((fails+=1))
done

exit "$fails"
