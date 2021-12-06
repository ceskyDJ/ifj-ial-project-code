#!/bin/bash
# Script for semi-automatic testing of the expressions parser by running its output in IFJcode21 interpreter
# Caution: you need to compile manual tests of expressions parser before running this!

# Move to this script folder
script="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
cd "$script" || exit 2

tmp_file=../../build/ifjcode21/program.ifjcode21
echo "" >$tmp_file

{
cat <<EOF;
.IFJcode21

JUMP &calls
LABEL &test
PUSHFRAME
DEFVAR LF@\$op_tmp_1
DEFVAR LF@\$op_tmp_2
# ------------------------------------------------------------------------------- #
EOF
../../build/bin/expr_parser.mantest
exit_code=$?
if [[ $exit_code != 0 ]]; then
  echo -e "\nCompilation failed with exit code: $exit_code" >&2
  exit 1
fi
cat <<EOF
# ------------------------------------------------------------------------------- #
POPFRAME
RETURN
LABEL &calls

CREATEFRAME
DEFVAR TF@\$test
CALL &test
POPS TF@\$test
DPRINT TF@\$test

EOF
} >>$tmp_file


echo -e "\n\nOUTPUT:\n"
./ic21int $tmp_file
