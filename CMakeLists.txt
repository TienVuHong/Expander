cmake_minimum_required(VERSION 3.0)

############################################################################## Set Compiler
if (DEFINED RASPBERRY_PI)
    message("Set Compiler for RASPBERRY_PI")
    ##########################################################################
    # # set cross-compiler for arm / raspberry pi
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_VERSION 1)
    set(CMAKE_C_COMPILER ${CMAKE_SOURCE_DIR}/../compiler/cross-gcc-8.3.0-pi_3+/cross-pi-gcc-8.3.0-2/bin/arm-linux-gnueabihf-gcc)
    set(CMAKE_CXX_COMPILER ${CMAKE_SOURCE_DIR}/../compiler/cross-gcc-8.3.0-pi_3+/cross-pi-gcc-8.3.0-2/bin/arm-linux-gnueabihf-g++)

    # ## set searching rules for cross-compiler
    set(CMAKE_SYSTEM_PROCESSOR arm)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    # #*/-----------------------------------------------------armv8----------------------------------------------------
    set(CMAKE_CXX_FLAGS "-std=c++14 -march=armv8-a -mfloat-abi=hard -mfpu=neon-fp-armv8")
    ##########################################################################
else()
    message("Set Compiler for UBUNTU")
    ##########################################################################
    # C++ std 14
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_EXE_LINKER_FLAGS "-no-pie")
    ##########################################################################
endif()
############################################################################## Set Compiler


project(PiServer)


############################################################################## Set Libraries
if (DEFINED RASPBERRY_PI)
    #libevent ### needed: libevent_core-2.1.so.7
    set(LIBEVENT libevent.so libevent_core-2.1.so)
    link_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_pi/lib_event-2.1.12/lib)
    include_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_pi/lib_event-2.1.12/include)

    #sqlite3 ### needed: libz.so.1 libz.so - build libz.so.1 to fix
    set(SQLITE3 libsqlite3.so libdl.so.2 libz.so.1)
    link_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_pi/lib_sqlite-3.36/lib)
    include_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_pi/lib_sqlite-3.36/include)

    #curl ### needed: libssl.so.1.1 libcrypto.so.1.1 - given by ssl and crypto below
    set(CURL libcurl.so)
    link_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_pi/lib_curl-7.79.1/lib)
    include_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_pi/lib_curl-7.79.1/include)

    #ssl
    set(SSL libssl.so)
    link_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_pi/lib_openssl-1.1.1/lib)
    include_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_pi/lib_openssl-1.1.1/include)

    #crypto
    set(CRYPTO libcrypto.so)
    link_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_pi/lib_openssl-1.1.1/lib)
    include_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_pi/lib_openssl-1.1.1/include)

    #cpp-jwt
    
    include_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_pi/lib_cpp-jwt-1.2.0/include)
    # set(cpp-jwt_DIR ${CMAKE_SOURCE_DIR}/../libs_pi/lib_cpp-jwt-1.2.0/lib/cmake/cpp-jwt)
    # find_package(cpp-jwt PATHS ${CMAKE_SOURCE_DIR}/../libs_pi/lib_cpp-jwt-1.2.0/lib/cmake/cpp-jwt NO_DEFAULT_PATH REQUIRED)

    #json
    link_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_pi/lib_json-3.9.1/lib)
    include_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_pi/lib_json-3.9.1/include)
    set(nlohmann_json_DIR ${CMAKE_SOURCE_DIR}/../compiler/libs_pi/lib_json-3.9.1/lib/cmake/nlohmann_json)
    find_package(nlohmann_json PATHS ${CMAKE_SOURCE_DIR}/../compiler/libs_pi/lib_json-3.9.1/lib/cmake/nlohmann_json NO_DEFAULT_PATH REQUIRED)

    #opencv
    # set(OPENCV_LIBS libopencv_world.so)
    link_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_pi/lib_opencv-4.5.2/lib)
    include_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_pi/lib_opencv-4.5.2/include/opencv4)
    # set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/../libs_pi/lib_opencv-4.5.2/lib/cmake/opencv4)
    # find_package(OpenCV REQUIRED)
    add_library(OPENCV SHARED IMPORTED)
    set_target_properties(OPENCV PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../compiler/libs_pi/lib_opencv-4.5.2/lib/libopencv_world.so)

    add_definitions(-DRASPBERRY_PI)
else()
    #OpenCV
    set(OPENCV libopencv_world.so)
    link_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_ubuntu/lib_opencv-4.5.2/lib)
    include_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_ubuntu/lib_opencv-4.5.2/include/opencv4)
    set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/../compiler/libs_ubuntu/lib_opencv-4.5.2/lib/cmake/opencv4)
    find_package(OpenCV REQUIRED)

    #Libevent
    set(LIBEVENT libevent.so)
    link_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_ubuntu/lib_event-2.1.12/lib)
    include_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_ubuntu/lib_event-2.1.12/include)

    #Sqlite3
    set(SQLITE3 libsqlite3.so)
    link_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_ubuntu/lib_sqlite-3.36/lib)
    include_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_ubuntu/lib_sqlite-3.36/include)

    #Curl
    set(CURL libcurl.so)
    link_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_ubuntu/lib_curl-7.79.1/lib)
    include_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_ubuntu/lib_curl-7.79.1/include)

    #JSon
    link_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_ubuntu/lib_json-3.9.1/lib)
    include_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_ubuntu/lib_json-3.9.1/include)
    set(nlohmann_json_DIR ${CMAKE_SOURCE_DIR}/../compiler/libs_ubuntu/lib_json-3.9.1/lib/cmake/nlohmann_json)
    find_package(nlohmann_json PATHS ${CMAKE_SOURCE_DIR}/../compiler/libs_ubuntu/lib_json-3.9.1/lib/cmake/nlohmann_json NO_DEFAULT_PATH REQUIRED)

    #JWT
    link_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_ubuntu/lib_cpp-jwt-1.2.0/lib)
    include_directories(${CMAKE_SOURCE_DIR}/../compiler/libs_ubuntu/lib_cpp-jwt-1.2.0/include)
    set(cpp-jwt_DIR ${CMAKE_SOURCE_DIR}/../compiler/libs_ubuntu/lib_cpp-jwt-1.2.0/lib/cmake/cpp-jwt)
    find_package(cpp-jwt PATHS ${CMAKE_SOURCE_DIR}/../compiler/libs_ubuntu/lib_cpp-jwt-1.2.0/lib/cmake/cpp-jwt NO_DEFAULT_PATH REQUIRED)
endif()
############################################################################## Set Libraries


#Setup
file(GLOB SOURCES 
    "driver/io/io.cpp"
    "service/timer/timer.cpp"
    "service/common/common.cpp"
    "service/base64/base64.cpp"
    "Database/Database.cpp"
    "Database/DatabaseEvent.cpp"
    "Database/DatabaseForward.cpp"
    "Log/Log.cpp"
    "Authenticate/Authenticate.cpp"
    "Controller/OutputController.cpp"
    "Controller/Controller.cpp"
    "Http/HttpServer.cpp"
    "Http/HttpHandle.cpp"
    "Http/HttpClient.cpp"
    "Http/HttpForward.cpp"
    "EventManager/EventManager.cpp"
    "Http/SyncProfile/SyncProfile.cpp"
    "main.cpp" 
)
add_executable(${PROJECT_NAME} ${SOURCES})
# add_library(${PROJECT_NAME} SHARED ${SOURCES})


############################################################################## Target Link
if (DEFINED RASPBERRY_PI)
    target_link_libraries(
        ${PROJECT_NAME}
        ${LIBEVENT}
        ${SQLITE3}
        ${CURL}
        ${SSL}
        ${CRYPTO}
        ${OPENCV}
        nlohmann_json::nlohmann_json
        -lpthread
        -Ofast -Wall -lm -lstdc++ -Wno-unused-result  -Wfatal-errors -Werror -Wpedantic
    )
else()
    target_link_libraries(
        ${PROJECT_NAME}
        ${LIBEVENT}
        ${SQLITE3}
        ${CURL}
        ${OPENCV}
        nlohmann_json::nlohmann_json
        cpp-jwt::cpp-jwt
        -lpthread
    )
endif()
############################################################################## Target Link
