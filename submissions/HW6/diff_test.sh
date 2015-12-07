#!/bin/bash
bash test_script.sh > interm_test_out
output="$(diff -s interm_test_out TEST_OUTPUT)"
echo "$output"
