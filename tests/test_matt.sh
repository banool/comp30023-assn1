#!/bin/bash

#make
mkdir -p test_matt/out
cd test_matt/in/
for file in *.txt
do
    #echo $file
    ../../simulation -f $file -a fcfs -m 100 > ../out/fcfs-$file
    #echo $file
    ../../simulation -f $file -a multi -m 100 > ../out/multi-$file
    #echo $file
done

cd ../out/
for file in *.txt
do
    diff $file ../matt_out/$file
done
cd ../../
#make clean
