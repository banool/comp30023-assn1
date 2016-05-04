echo "If this passes all the tests, it should only print the lines for make and make clean."

make
cp simulation tests/
cd tests
./test_a.sh
./test_matt.sh
./test_final.sh
rm simulation
cd ..
make clean
