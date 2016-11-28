project(arma_htk VERSION 0.1 LANGUAGES CXX)

# Define library. Only source files here!
add_library(arma_htk
    src/gen_filt.cpp
    src/htk_file.cpp
    src/mfcc_htk.cpp)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(arma_htk PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ../libs/armadillo/include
    PRIVATE src)

# If we have compiler requirements for this library, list them
# here
target_compile_features(arma_htk
    PUBLIC cxx_auto_type
    PUBLIC cxx_return_type_deduction
    PRIVATE cxx_variadic_templates)

# 'make install' to the correct location
install(TARGETS arma_htk
    ARCHIVE  DESTINATION arma_htk
    LIBRARY  DESTINATION arma_htk
    RUNTIME  DESTINATION bin)  # This is for Windows
install(DIRECTORY include/ DESTINATION include)

# This makes the project importable from the build directory
export(TARGETS arma_htk FILE ArmaHtkLibraryConfig.cmake)
