rm -rf build
mkdir build
cd build
cmake ..
make
cp -r ../shaders .
./toph