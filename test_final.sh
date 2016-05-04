#!/bin/bash
#make
cd final_tests_datafiles/
mkdir -p my_out

../simulation -f test1.txt -a fcfs -m 100 > my_out/test1.out
../simulation -f test2.txt -a fcfs -m 100 > my_out/test2.out
../simulation -f test3.txt -a fcfs -m 25 > my_out/test3.out
../simulation -f test6.txt -a multi -m 100 > my_out/test6.out
../simulation -f test7.txt -a multi -m 150 > my_out/test7.out
../simulation -f test8.txt -a multi -m 160 > my_out/test8.out
../simulation -f test9.txt -a multi -m 400 > my_out/test9.out
../simulation -f test10.txt -a multi -m 100 > my_out/test10.out
../simulation -f test11.txt -a multi -m 16384 > my_out/test11.out
../simulation -f test12.txt -a multi -m 1000 > my_out/test12.out

for file in *.out
do
    diff $file ./my_out/$file
done

rm -R my_out
cd ../
#make clean
