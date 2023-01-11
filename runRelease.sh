chmod +x run.sh
chmod +x runTestharness.sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cd build
make
cd .. 
./runTestharness.sh