cmake_minimum_required(VERSION 3.16)
project(DeerBourne C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Options
option(DEERBOURNE_ENABLE_TESTS "Enable DeerBourne tests" ON)
option(DEERBOURNE_BUILD_DOCS "Enable documentation target" ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

# Bourne library (static)
file(GLOB_RECURSE BOURNE_LIBRARY_SOURCES CONFIGURE_DEPENDS
    "bourne/src/*.c"
)
add_library(bourne STATIC ${BOURNE_LIBRARY_SOURCES})
target_include_directories(bourne PUBLIC bourne/include)
add_library(deerbourne::bourne ALIAS bourne)

# Bourne service executable
file(GLOB_RECURSE BOURNE_SERVICE_SOURCES CONFIGURE_DEPENDS
    "bourne/service/*.c"
)
add_executable(bourne_service ${BOURNE_SERVICE_SOURCES})
target_link_libraries(bourne_service PRIVATE bourne)

# Deer front-end library
file(GLOB_RECURSE DEER_SOURCES CONFIGURE_DEPENDS
    "deer/src/*.c"
)
add_library(deer ${DEER_SOURCES})
target_include_directories(deer PUBLIC deer/include)
add_library(deerbourne::deer ALIAS deer)

target_link_libraries(deer PRIVATE bourne)

# Deer GUI executable
file(GLOB_RECURSE DEER_APP_SOURCES CONFIGURE_DEPENDS
    "deer/app/*.c"
)
add_executable(deer_app ${DEER_APP_SOURCES})
target_link_libraries(deer_app PRIVATE deer bourne PkgConfig::GTK4)

# Documentation target
if(DEERBOURNE_BUILD_DOCS)
    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND)
        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    else()
        message(WARNING "Doxygen not found: docs target will be unavailable.")
    endif()
endif()

# Tests placeholder
if(DEERBOURNE_ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
