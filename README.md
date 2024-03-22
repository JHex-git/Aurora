# Feature
Currently only Phong shading is supported and the system is not interactive.

# How to build
Only MinGW is tested fully. So use MinGW if you want to have this project compiled. No support for MSVC yet as it takes some pain to use dll in it. MSYS2 MINGW64 is recommended as the version of its g++ has a good support for C++17. g++ with versions lower than 9.0.0 may not work as std::filesystem is not supported in these versions.

``` shell
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
make -j4
```
