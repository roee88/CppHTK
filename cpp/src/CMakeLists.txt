# At LEAST 3.0 but newer is better
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(armahtk VERSION 0.1 LANGUAGES CXX)

# External 3rd party libs that we include
add_definitions(-DARMA_USE_LAPACK -DARMA_USE_BLAS)
add_definitions(-DARMA_USE_CXX11)
add_subdirectory(libs/armadillo)
 
# Targets that we develop
add_subdirectory(arma_htk)
add_subdirectory(sample)
