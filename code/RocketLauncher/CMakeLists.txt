cmake_minimum_required(VERSION 4.1.0)
project(RocketLauncher VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_TESTS "Build unit tests" ON)
option(ENABLE_FORMATTING "Enable code formatting" ON)
option(PLATFORMIO_INTEGRATION "Enable PlatformIO integration" ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find required packages (PkgConfig not needed for this project)

# Source files (for testing and documentation)
set(SOURCES
    src/RocketController.cpp
)

set(HEADERS
    src/ArduinoInterface.h
    src/RocketController.h
)

# Tests (native only - Arduino builds handled by PlatformIO)
if(BUILD_TESTS)
    enable_testing()
    
    # Test executable (excludes main.cpp and Arduino-specific code)
    add_executable(rocket_tests
        test/test_rocket_controller.cpp
        src/RocketController.cpp
    )
    
    # Test configuration (same as PlatformIO native env)
    target_compile_definitions(rocket_tests PRIVATE
        ARDUINO=0
        UNITY_INCLUDE_DOUBLE
        UNITY_DOUBLE_PRECISION=1e-12
        UNITY_INCLUDE_CONFIG_H
    )
    
    # Compiler flags (same as PlatformIO native env)
    target_compile_options(rocket_tests PRIVATE
        -std=c++17
        -Wall -Wextra -Wpedantic
    )
    
    # Add tests
    add_test(NAME RocketControllerTests COMMAND rocket_tests)
endif()

# PlatformIO integration targets (these become proper CMake targets for CLion)
if(PLATFORMIO_INTEGRATION)
    # Find PlatformIO
    find_program(PLATFORMIO "pio")
    
    if(PLATFORMIO)
        # Main firmware targets that CLion will see
        add_custom_target(firmware_sim ALL
            COMMAND ${PLATFORMIO} run -e simulide
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Building simulation firmware with PlatformIO"
            SOURCES ${SOURCES} ${HEADERS}
        )
        
        add_custom_target(firmware_hw ALL
            COMMAND ${PLATFORMIO} run -e uno_hw
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Building hardware firmware with PlatformIO"
            SOURCES ${SOURCES} ${HEADERS}
        )
        
        # Test target that CLion will see
        add_custom_target(pio_tests
            COMMAND ${PLATFORMIO} test -e native
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running PlatformIO tests"
            SOURCES ${SOURCES} ${HEADERS}
        )
        
        # Utility targets
        add_custom_target(upload
            COMMAND ${PLATFORMIO} run -e uno_hw -t upload
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Uploading firmware to Arduino with PlatformIO"
            DEPENDS firmware_hw
        )
        
        add_custom_target(monitor
            COMMAND ${PLATFORMIO} device monitor -b 115200
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Starting serial monitor with PlatformIO"
        )
        
        # Clean target
        add_custom_target(pio_clean
            COMMAND ${PLATFORMIO} run -t clean
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Cleaning PlatformIO build files"
        )
        
        # Combined build target (what CLion will see as the main target)
        add_custom_target(all_firmware ALL
            COMMAND ${CMAKE_COMMAND} -E echo "Building all firmware targets..."
            COMMAND ${PLATFORMIO} run -e simulide
            COMMAND ${PLATFORMIO} run -e uno_hw
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Building both simulation and hardware firmware with PlatformIO"
            SOURCES ${SOURCES} ${HEADERS}
            DEPENDS firmware_sim firmware_hw
        )
        
        # Set dependencies so CLion understands the build order
        if(BUILD_TESTS)
            add_dependencies(rocket_tests all_firmware)
        endif()
        
        message(STATUS "PlatformIO integration enabled - CMake targets created for CLion")
    else()
        message(WARNING "PlatformIO not found. Install with: pip install platformio")
        message(WARNING "Firmware targets will not be available")
    endif()
endif()

# Code formatting
if(ENABLE_FORMATTING)
    find_program(CLANG_FORMAT "clang-format")
    if(CLANG_FORMAT)
        add_custom_target(format
            COMMAND ${CLANG_FORMAT} -i ${SOURCES} ${HEADERS}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Formatting source code with clang-format"
        )
    endif()
endif()

# Custom targets
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMENT "Cleaning all CMake build files"
)

# Help target
add_custom_target(show_help
    COMMAND ${CMAKE_COMMAND} --help
    COMMENT "Showing CMake help"
)

# Status target
add_custom_target(status
    COMMAND ${CMAKE_COMMAND} -E echo "Project: ${PROJECT_NAME}"
    COMMAND ${CMAKE_COMMAND} -E echo "Version: ${PROJECT_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "Build directory: ${CMAKE_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "Source directory: ${CMAKE_SOURCE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "PlatformIO integration: ${PLATFORMIO_INTEGRATION}"
    COMMAND ${CMAKE_COMMAND} -E echo "Tests enabled: ${BUILD_TESTS}"
    COMMENT "Showing project status"
)

# Install rules (for tests only)
if(BUILD_TESTS)
    install(TARGETS rocket_tests
        RUNTIME DESTINATION bin
    )
endif()

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
