cmake_minimum_required(VERSION 3.16)
project(DeerBourne C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Options
option(DEERBOURNE_ENABLE_TESTS "Enable DeerBourne tests" ON)
option(DEERBOURNE_BUILD_DOCS "Enable documentation target" ON)
option(DEERBOURNE_ENABLE_GUI "Build GTK-based front-end components" ON)

find_package(PkgConfig REQUIRED)
if(DEERBOURNE_ENABLE_GUI)
    pkg_check_modules(GTK4 QUIET IMPORTED_TARGET gtk4)
    if(NOT GTK4_FOUND)
        message(WARNING "GTK4 not found; GUI targets will be skipped.")
    endif()
endif()
find_package(Threads REQUIRED)

# Bourne library (static)
file(GLOB_RECURSE BOURNE_LIBRARY_SOURCES CONFIGURE_DEPENDS
    "bourne/src/*.c"
)
add_library(bourne STATIC ${BOURNE_LIBRARY_SOURCES})
target_include_directories(bourne PUBLIC bourne/include)
target_link_libraries(bourne PUBLIC Threads::Threads)
add_library(deerbourne::bourne ALIAS bourne)

# Bourne service executable
file(GLOB_RECURSE BOURNE_SERVICE_SOURCES CONFIGURE_DEPENDS
    "bourne/service/*.c"
)
add_executable(bourne_service ${BOURNE_SERVICE_SOURCES})
target_link_libraries(bourne_service PRIVATE bourne)

# Debug-friendly Bourne service executable
add_executable(bourne_service_debug ${BOURNE_SERVICE_SOURCES})
target_link_libraries(bourne_service_debug PRIVATE bourne)
target_compile_definitions(bourne_service_debug PRIVATE DEERBOURNE_DEBUG_BUILD)
target_compile_options(bourne_service_debug PRIVATE -O0 -g)

set(DEER_CORE_SOURCES
    deer/src/client.c
    deer/src/ui_config.c
)
add_library(deer_core ${DEER_CORE_SOURCES})
target_include_directories(deer_core PUBLIC deer/include)
target_link_libraries(deer_core PRIVATE bourne)
add_library(deerbourne::deer_core ALIAS deer_core)

if(DEERBOURNE_ENABLE_GUI AND GTK4_FOUND)
    pkg_check_modules(JSONGLIB QUIET json-glib-1.0)

    set(DEER_UI_SOURCES
        deer/src/ui_layout.c
        deer/src/sample_data.c
    )

    add_library(deer_ui ${DEER_UI_SOURCES})
    target_include_directories(deer_ui PUBLIC deer/include)
    target_link_libraries(deer_ui PUBLIC deer_core PkgConfig::GTK4)
    if(JSONGLIB_FOUND)
        target_link_libraries(deer_ui PUBLIC PkgConfig::JSONGLIB)
        target_compile_definitions(deer_ui PRIVATE DEER_HAVE_JSON_GLIB)
    else()
        message(WARNING "json-glib not found; Deer sample data will use built-in fixtures.")
    endif()
    add_library(deerbourne::deer_ui ALIAS deer_ui)

    file(GLOB_RECURSE DEER_APP_SOURCES CONFIGURE_DEPENDS
        "deer/app/*.c"
    )
    add_executable(deer_app ${DEER_APP_SOURCES})
    target_link_libraries(deer_app PRIVATE deer_ui deer_core bourne PkgConfig::GTK4)

    add_executable(deer_app_debug ${DEER_APP_SOURCES})
    target_link_libraries(deer_app_debug PRIVATE deer_ui deer_core bourne PkgConfig::GTK4)
    target_compile_definitions(deer_app_debug PRIVATE DEERBOURNE_DEBUG_BUILD)
    target_compile_options(deer_app_debug PRIVATE -O0 -g)
endif()

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
