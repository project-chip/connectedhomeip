# Define minimal required version of CMake.
cmake_minimum_required(VERSION "3.25")

# Project definition
project(
	generator
	VERSION 1.0
	LANGUAGES C CXX ASM
)

# Include the definition of the slc_generator target,
# which contains the content of the SLC project
include(generator.cmake)

add_executable(generator
    # Add additional sources here
)

target_include_directories(generator PUBLIC
    # Add additional include paths here
)

target_compile_definitions(generator PUBLIC
    # Add additional macros here
)

target_compile_options(generator PUBLIC
    # Set additional compiler flags here
)

target_link_options(generator PUBLIC
    # Set additional linker flags here
)

# Link with the content defined in the SLC project
target_link_libraries(generator PRIVATE
    slc_generator
)

# Include managed project content if available
include(generator_project.cmake OPTIONAL RESULT_VARIABLE managed_project)
if(managed_project)
    message(STATUS "Using managed project content from ${managed_project}")
endif()

# Create .bin, .hex and .s37 artifacts after building the project
add_custom_command(TARGET generator
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O srec "$<TARGET_FILE:generator>" "$<TARGET_FILE_DIR:generator>/$<TARGET_FILE_BASE_NAME:generator>.s37"
    COMMAND ${CMAKE_OBJCOPY} -O ihex "$<TARGET_FILE:generator>" "$<TARGET_FILE_DIR:generator>/$<TARGET_FILE_BASE_NAME:generator>.hex"
    COMMAND ${CMAKE_OBJCOPY} -O binary "$<TARGET_FILE:generator>" "$<TARGET_FILE_DIR:generator>/$<TARGET_FILE_BASE_NAME:generator>.bin" 
)


