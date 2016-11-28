# Introduction
This is a direct port of the python code to c++11 with Armadillo.

# Building

## Unix
1. ```cd build```
2. ```cmake ../src && make```

## Windows
1. install cmake
2. ```cd build``` 
3. ```cmake ../src```
4. open build/arma_htk.sln in visual studio.
5. in solution explorer, right click on `sample` and select `Manage NuGet Packages`.
6. install openblas from NuGet.
6. build entire solution.
7. select `sample` as startup project before running the sample.
