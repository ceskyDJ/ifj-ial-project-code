# Script for generating exit code after running unit tests for Github Actions

# Move to this script folder
script="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
cd "$script" || exit 2

if grep -qs "FAIL[^E]" ../../build/results/*.txt; then
  # There is at least one failed test in results, so tests have failed
  exit 1
else
  # There are no failed tests in results, tests have been successful
  exit 0
fi
