#!/bin/bash
TEST_FILES=./mytests/*
bash compile.sh
for f in $TEST_FILES
do
  echo "Processing $f file..."
  cat $f | ./hw4 -
  #output="$(cat $f | ./hw4 -)"
  #echo $output | tr " " "S"
  #echo $output | wc -m
done
