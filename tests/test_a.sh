#!/bin/bash

#make
cd partA_cases
../simulation -f specInput.txt -m 100 -a multi > out.txt
diff out.txt specOutput-multi-100.txt
../simulation -f specInput.txt -m 100 -a fcfs > out.txt
diff out.txt specOutput-fcfs-100.txt
# Kurt is not rounding up properly in his output. Omitted for now.
#../simulation -f in_hard.txt -m 1000 -a multi > out.txt
#diff out.txt kurt_out.txt
../simulation -f tang.txt -m 100 -a multi > out.txt
diff out.txt tang_out_multi.txt
../simulation -f tang.txt -m 100 -a fcfs > out.txt
diff out.txt tang_out_fcfs.txt
rm out.txt
cd ..
#make clean
