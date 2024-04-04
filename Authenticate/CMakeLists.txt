project(Authenticate)
message(">>>>>>>>>>>>>>>> Authenticate")

set(HEADERS
    ${PROJECT_NAME}.hpp
)

set(SOURCES
    ${PROJECT_NAME}.cpp
)

add_library(${PROJECT_NAME}
    ${HEADERS}
    ${SOURCES}
)

target_link_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/../../compiler/libs_ubuntu/lib_openssl-1.1.1/lib
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../../compiler/libs_ubuntu/lib_json-3.9.1/include
    ${PROJECT_SOURCE_DIR}/../../compiler/libs_ubuntu/lib_openssl-1.1.1/include
    ${PROJECT_SOURCE_DIR}/../../compiler/libs_ubuntu/lib_cpp-jwt-1.2.0/include
)

target_link_libraries(${PROJECT_NAME}
    json
    crypto
    ssl
)
