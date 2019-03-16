# -------------------------------------------------------------------------------- #
# -------------------------------- FLAGS AND PATHS ------------------------------- #
# -------------------------------------------------------------------------------- #

set(ProjectName "LoggerClient")

message("#------------------------------- " ${ProjectName} " ------------------------------- #")

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
cmake_minimum_required(VERSION 3.3)

set(TARGET_WINDOWS 0)
set(TARGET_LINUX 0)
set(TARGET_OSX 0)


project(${ProjectName})

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(TARGET_WINDOWS 1)
    message("Building for Windows")

    add_definitions(-DBOOST_ASIO_DISABLE_IOCP)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(TARGET_LINUX 1)
    message("Building for Linux")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(TARGET_OSX 1)
    message("Building for OSX")

endif()


find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
#    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    message("Found CCache")

endif(CCACHE_FOUND)



# -------------------------------------------------------------------------------- #
# --------------------------------- LIBRARY PATHS -------------------------------- #
# -------------------------------------------------------------------------------- #

#if(TARGET_WINDOWS OR TARGET_LINUX OR TARGET_OSX)
#    set(OPENSSL_ROOT_DIR ${path_to_OpenSSL})
#    set(OPENSSL_LIBRARIES ${path_to_OpenSSL}/lib)
##    find_package(OpenSSL REQUIRED)
#    message("OpenSSL libraries:\t${OPENSSL_LIBRARIES}")

#endif()
#set(OPENSSL_USE_STATIC_LIBS ON)

find_package(Qt5 COMPONENTS Widgets Network REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)



# -------------------------------------------------------------------------------- #
# ------------------------------------ SOURCES ----------------------------------- #
# -------------------------------------------------------------------------------- #

file(GLOB_RECURSE SRC_FILES
    ${PROJECT_SOURCE_DIR}/*.cpp
    ${PROJECT_SOURCE_DIR}/*.h
    ${PROJECT_SOURCE_DIR}/../resources/*.qrc
    )

file(GLOB_RECURSE EXCLUDE_LOGGER_CPPDK
    ${PROJECT_SOURCE_DIR}/log/*
    )
if(EXCLUDE_LOGGER_CPPDK)
    list(REMOVE_ITEM SRC_FILES ${EXCLUDE_LOGGER_CPPDK})
endif(EXCLUDE_LOGGER_CPPDK)

if(TARGET_WINDOWS)
    add_executable(${PROJECT_NAME} WIN32 ${SRC_FILES})

elseif(TARGET_LINUX OR TARGET_OSX)
    add_executable(${PROJECT_NAME} ${SRC_FILES})

endif()

#message("SRC_FILES: ${SRC_FILES}")



# -------------------------------------------------------------------------------- #
# ----------------------------------- INCLUDES ----------------------------------- #
# -------------------------------------------------------------------------------- #

set(TARGET_INCLUDES
    ${PROJECT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
    )

if(TARGET_WINDOWS OR TARGET_LINUX OR TARGET_OSX)
    set(TARGET_INCLUDES ${TARGET_INCLUDES}
#        ${OPENSSL_INCLUDE_DIR}
        )

endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${TARGET_INCLUDES})



# -------------------------------------------------------------------------------- #
# ---------------------------------- COMPILATION --------------------------------- #
# -------------------------------------------------------------------------------- #

set(CXX_FLAGS_COMMON "${CMAKE_CXX_FLAGS} -fno-operator-names -fmessage-length=0 -std=c++14 -pedantic -DBOOST_NO_LONG_LONG -U_GLIBCXX_HAVE_ISWBLANK")

if (TARGET_WINDOWS OR TARGET_LINUX OR TARGET_OSX)
    set(CXX_FLAGS_COMMON "${CXX_FLAGS_COMMON}")

endif()

message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
if( CMAKE_BUILD_TYPE STREQUAL "Release" )
    set (CMAKE_CXX_FLAGS "${CXX_FLAGS_COMMON} -O3 -w")
#    add_definitions(-DQT_NO_DEBUG_OUTPUT)

elseif( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set (CMAKE_CXX_FLAGS "${CXX_FLAGS_COMMON} -O0 -g2 -w")
    add_definitions(-DDEBUG_STUFF)

else()
    set (CMAKE_CXX_FLAGS "${CXX_FLAGS_COMMON} -O2")

endif( CMAKE_BUILD_TYPE STREQUAL "Release" )

message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")



# -------------------------------------------------------------------------------- #
# ------------------------------------ LINKING ----------------------------------- #
# -------------------------------------------------------------------------------- #

set(TARGET_LIBRARIES
    Qt5::Widgets
    Qt5::Network
    )

if(TARGET_WINDOWS)
    set(TARGET_LIBRARIES ${TARGET_LIBRARIES}
#        ${OPENSSL_LIBRARIES}
        -lws2_32
        -lgdi32
        -lpsapi
        )

elseif(TARGET_LINUX)
    set(TARGET_LIBRARIES ${TARGET_LIBRARIES}
#        ${OPENSSL_LIBRARIES}
        )

elseif(TARGET_OSX)
    set(TARGET_LIBRARIES ${TARGET_LIBRARIES}
#        ${OPENSSL_LIBRARIES}
        -liconv
        )

endif()

target_link_libraries(${PROJECT_NAME} ${TARGET_LIBRARIES})


set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
    ")

if(TARGET_WINDOWS OR TARGET_LINUX)
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
        -Wl,--warn-unresolved-symbols \
        -Wl,--no-undefined \
        -Wl,--no-allow-shlib-undefined \
        -Wl,--gc-sections \
        ")

endif()

#   -Wl,-L\"${path_to_lib}/lib\" \
#    -Wl,-s \
#        -Wl,--as-needed \
#        -Wl,--print-gc-sections \
#        -nodefaultlibs \
#        -static \
#        -Wl,-shared_libgcc \
#        -Wl,-Bdynamic \

#message("CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")



# -------------------------------------------------------------------------------- #
# ---------------------------------- OTHER FILES --------------------------------- #
# -------------------------------------------------------------------------------- #

file(COPY ${PROJECT_SOURCE_DIR}/../resources/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


