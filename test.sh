#!/bin/bash

make
./simulation -f specInput.txt -m 100 -a multi > specInput-multi-100-out.txt
diff specInput-multi-100-out.txt specOutput-multi-100.txt
./simulation -f specInput.txt -m 100 -a fcfs > specInput-fcfs-100-out.txt
diff specInput-fcfs-100-out.txt specOutput-fcfs-100.txt
./simulation -f in_hard.txt -m 1000 -a multi > out_hard.txt
diff out_hard.txt kurt_out.txt 
#./simulation -f exp_in.txt -m 30 -a multi