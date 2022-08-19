cmake_minimum_required(VERSION 3.2)
project("Vectorpack" CXX)
set(exec_name "vectorpack")
set(lib_name "Vectorpack")

######### TODO ##########
# set the library extension and make the executable/library usable in Windows
#########################


# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(include_algorithms "Include packing algorithms in the vectorpack library" ON)
option(build_executable "Build the vectorpack executable" OFF)

# Algorithms are required in the executable
if (build_executable AND NOT include_algorithms)
    message("Setting 'include_algorithms' to ON to build the executable")
    set(include_algorithms ON)
endif()

if (include_algorithms)
    message("Packing algorithms will be included in the Vectorpack library")
endif()

if (build_executable)
    message("Only the executable will be built. The Vectorpack library will NOT be installed")
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


# Get the vectorpack lib
set(HEADER_LIB
    src/lib/bin.hpp
    src/lib/item.hpp
    src/lib/instance.hpp
)

set(SOURCE_LIB
    src/lib/bin.cpp
    src/lib/item.cpp
    src/lib/instance.cpp
)

if (include_algorithms)
    set(SOURCE_ALGOS
        src/algos/weights_measures_scores.cpp
        src/algos/base_algo.cpp
        src/algos/algos_ItemCentric.cpp
        src/algos/algos_BinCentric.cpp
        src/algos/algos_MultiBin.cpp
        src/algos/lower_bounds.cpp
        src/algos/utils.cpp
    )

    set(HEADER_ALGOS
        # TODO will need to hide some header files
        src/algos/weights_measures_scores.hpp
        src/algos/base_algo.hpp
        src/algos/algos_ItemCentric.hpp
        src/algos/algos_BinCentric.hpp
        src/algos/algos_MultiBin.hpp
        src/algos/lower_bounds.hpp
        src/algos/utils.hpp
    )
endif()

add_library(${lib_name} STATIC
    ${HEADER_LIB}
    ${SOURCE_LIB}
    ${HEADER_ALGOS}
    ${SOURCE_ALGOS}
)


target_include_directories(${lib_name}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/lib>
    PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/algos># TODO will need to hide some header files in the algos

    PUBLIC $<INSTALL_INTERFACE:lib>
    PUBLIC $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)
#set_target_properties(${lib_name} PROPERTIES OUTPUT_NAME "Vectorpack")
set_target_properties(${lib_name} PROPERTIES PUBLIC_HEADER "${HEADER_LIB};${HEADER_ALGOS}")


### if build executable
if (build_executable)
    add_executable(${exec_name} src/main_vectorpack.cpp)
    target_link_libraries(${exec_name}
        PRIVATE ${lib_name}
    )
endif()
###

# Install rule
# TODO
### Depending whether only executable should be installed
### or the basic library only
if (build_executable)
    # Install only the executable
    install(TARGETS ${exec_name} DESTINATION bin)
else()
    # Else, install the Vectorpack library
    install(TARGETS ${lib_name}
        EXPORT ${lib_name}Config
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include/${PROJECT_NAME}
    )
    install(EXPORT ${lib_name}Config DESTINATION lib/cmake/${PROJECT_NAME})
endif()
