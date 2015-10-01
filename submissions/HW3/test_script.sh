#!/bin/bash
TEST_FILES=./mytests/*
for f in $TEST_FILES
do
  echo "Processing $f file..."
  cat $f | ./hw3 -
  #output="$(cat $f | ./hw3 -)"
  #echo $output | tr " " "S"
  #echo $output | wc -m
done
