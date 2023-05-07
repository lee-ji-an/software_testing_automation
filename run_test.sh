#!/bin/bash
echo input the name of c file that contains the function to be tested. ex. triangle.c
read test_function
echo input the name of txt file that contains test cases. ex. triangle_test_cases.txt
read test_cases

gcc automation_program.c -o automation_program
./automation_program ${test_cases} ${test_function} 
sh run.sh start