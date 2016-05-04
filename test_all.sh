echo "If this passes all the tests, it should only print the lines for make and make clean."

make
./test_a.sh
./test_matt.sh
./test_final.sh
make clean
