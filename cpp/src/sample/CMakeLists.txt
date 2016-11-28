# Define sample executable. Only source files here!
add_executable(sample
    src/main.cpp)

# Depend on a library that we defined in the top-level file
target_link_libraries(sample 
    arma_htk
    armadillo)
    
# copy /example to source dir
add_custom_command(TARGET sample PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/src/example
    ${CMAKE_CURRENT_BINARY_DIR}/example)

# copy /example to output dir
add_custom_command(TARGET sample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_BINARY_DIR}/example
            $<TARGET_FILE_DIR:sample>/example)
