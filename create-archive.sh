#!/bin/bash
# Creates final archive
# $1 ... Name of the TeX file with docs

doc_name=$1

# Move to this script folder
script="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
cd "$script" || exit 1

# Create tmp folder where the final project structure will be prepared
mkdir tmp

# Go to tmp folder
cd tmp || exit 1

# Clone documentation from its repository
git clone git@github.com:ceskyDJ/ifj-ial-project-docs.git
cd ifj-ial-project-docs || exit 1

# Compile documentation (from its Git repository)
# BibTeX may not be needed
grep -q "\cite" "$doc_name.tex" && pdflatex "$doc_name.tex"
grep -q "\cite" "$doc_name.tex" && bibtex "$doc_name.aux"
pdflatex "$doc_name.tex"
pdflatex "$doc_name.tex"

# Go back to tmp/
cd ../ || exit 1

# Move result PDF file into tmp folder
mv ifj-ial-project-docs/"$doc_name.pdf" dokumentace.pdf

# Remove directory with documentation sources
rm -rf ifj-ial-project-docs

# Go to project root
cd ../ || exit 1

# Copy source files into tmp folder (for creating archive)
# *.h files could missing
cp src/*.c tmp/
ls src/*.h >/dev/null 2>&1 && cp src/*.h tmp/

# Copy special files (required by assignment) into tmp folder
# `rozsireni` could missing
cp rozdeleni tmp/
[ -f rozsireni ] && cp rozsireni tmp/

# Copy Makefile and its patch (see  into tmp folder
cp src/Makefile* tmp/

# Go back to tmp/
cd tmp || exit 1

# Apply patch to Makefile (removed not needed rules, variables etc.)
patch -p0 < Makefile.patch
rm Makefile.patch

# Create archive
tar -czvf ../xsmahe01.tar.gz *

# Go to project root
cd ../ || exit 1

# Remove tmp/
rm -rf tmp/
