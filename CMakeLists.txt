cmake_minimum_required(VERSION 3.22.1)

project(expander)
message(">>>>>>>>>>>>>>> expander")

message("add_subdirectory")
add_subdirectory(service)
add_subdirectory(Authenticate)
add_subdirectory(Controller)
add_subdirectory(Database)
add_subdirectory(EventManager)
add_subdirectory(Http)
add_subdirectory(Log)

message("add_executable")
add_executable(${PROJECT_NAME}
    main.cpp
)

# set(ALL_LIB_DIRS
#     ${PROJECT_SOURCE_DIR}/../compiler/libs_ubuntu/lib_json-3.9.1/lib
#     ${PROJECT_SOURCE_DIR}/../compiler/libs_ubuntu/lib_cpp-jwt-1.2.0/lib
#     ${PROJECT_SOURCE_DIR}/../compiler/libs_ubuntu/lib_openssl-1.1.1/lib
#     ${PROJECT_SOURCE_DIR}/../compiler/libs_ubuntu/lib_event-2.1.12/lib
#     ${PROJECT_SOURCE_DIR}/../compiler/libs_ubuntu/lib_curl-7.79.1/lib
#     ${PROJECT_SOURCE_DIR}/../compiler/libs_ubuntu/lib_sqlite-3.36/lib
# )

# set(ALL_INCLUDE_DIRS
#     ${CMAKE_SOURCE_DIR}
#     ${PROJECT_SOURCE_DIR}
#     ${PROJECT_SOURCE_DIR}/../compiler/libs_ubuntu/lib_json-3.9.1/include
#     ${PROJECT_SOURCE_DIR}/../compiler/libs_ubuntu/lib_cpp-jwt-1.2.0/include
#     ${PROJECT_SOURCE_DIR}/../compiler/libs_ubuntu/lib_openssl-1.1.1/include
#     ${PROJECT_SOURCE_DIR}/../compiler/libs_ubuntu/lib_event-2.1.12/include
#     ${PROJECT_SOURCE_DIR}/../compiler/libs_ubuntu/lib_curl-7.79.1/include
#     ${PROJECT_SOURCE_DIR}/../compiler/libs_ubuntu/lib_sqlite-3.36/include
# )

# target_link_directories(${PROJECT_NAME} PUBLIC
#     ${ALL_LIB_DIRS}
# )

target_link_libraries(${PROJECT_NAME}
    Http
    pthread
)
