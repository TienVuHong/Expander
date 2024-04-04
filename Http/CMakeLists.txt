project(Http)
message(">>>>>>>>>>>>>>>> Http")

set(HEADERS
    HttpServer.hpp
    HttpClient.hpp
    HttpHandle.hpp
    HttpForward.hpp
    SyncProfile/SyncProfile.hpp
)

set(SOURCES
    HttpServer.cpp
    HttpClient.cpp
    HttpHandle.cpp
    HttpForward.cpp
    SyncProfile/SyncProfile.cpp
)

add_library(${PROJECT_NAME}
    ${HEADERS}
    ${SOURCES}
)

target_link_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/../../compiler/libs_ubuntu/lib_openssl-1.1.1/lib
    ${PROJECT_SOURCE_DIR}/../../compiler/libs_ubuntu/lib_event-2.1.12/lib
    ${PROJECT_SOURCE_DIR}/../../compiler/libs_ubuntu/lib_curl-7.79.1/lib
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../../compiler/libs_ubuntu/lib_json-3.9.1/include
    ${PROJECT_SOURCE_DIR}/../../compiler/libs_ubuntu/lib_openssl-1.1.1/include
    ${PROJECT_SOURCE_DIR}/../../compiler/libs_ubuntu/lib_event-2.1.12/include
    ${PROJECT_SOURCE_DIR}/../../compiler/libs_ubuntu/lib_curl-7.79.1/include
)

target_link_libraries(${PROJECT_NAME}
    crypto
    ssl
    event
    curl
    json
    common
    base64
    datetime
    Authenticate
    Controller
    Database
    EventManager
    Log
)
