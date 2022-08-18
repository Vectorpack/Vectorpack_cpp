cmake_minimum_required(VERSION 3.2)
project("Vectorpack" CXX)

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
    message("The executable will be built. The Vectorpack library will not be installed")
endif()

# Where to find the Find*.cmake files
#set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

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
        # TODO will need to hide some header files
        src/algos/weights_measures_scores.hpp src/algos/weights_measures_scores.cpp
        src/algos/base_algo.hpp src/algos/base_algo.cpp
        src/algos/algos_ItemCentric.hpp src/algos/algos_ItemCentric.cpp
        src/algos/algos_BinCentric.hpp src/algos/algos_BinCentric.cpp
        src/algos/algos_MultiBin.hpp src/algos/algos_MultiBin.cpp
        src/algos/lower_bounds.hpp src/algos/lower_bounds.cpp
        src/algos/utils.hpp src/algos/utils.cpp
    )
endif()

add_library(Vectorpack_lib STATIC
    ${HEADER_LIB}
    ${SOURCE_LIB}
    ${SOURCE_ALGOS}
)

target_include_directories(Vectorpack_lib
    PUBLIC src/lib
    # TODO will need to hide some header files in the algos
    PUBLIC src/algos
)
set_target_properties(Vectorpack_lib PROPERTIES OUTPUT_NAME "Vectorpack") 

### if build executable
if (build_executable)
    add_executable(vectorpack src/main_vectorpack.cpp)
    target_link_libraries(vectorpack
        PRIVATE Vectorpack_lib
    )
endif()
###

# Install rule
# TODO
### Depending whether only executable should be installed
### or the basic library only
if (build_executable)
    # Install only the executable
    install(TARGETS vectorpack DESTINATION bin)
else()
    # Else, install the Vectorpack library
    install(TARGETS Vectorpack_lib
        DESTINATION lib
    )
endif()
