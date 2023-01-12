chmod +x run.sh
chmod +x runTestharness.sh
cmake -B build -DCMAKE_BUILD_TYPE=Release > /dev/null
cd build
make > /dev/null
cd .. 
./runTestharness.sh