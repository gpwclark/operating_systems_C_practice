#!/bin/bash
TEST_FILES=./mytests/*
bash compile.sh
for f in $TEST_FILES
do
  echo "Processing $f file..."
  cat $f | ./hw6 -
done
