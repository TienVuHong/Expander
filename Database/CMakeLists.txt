project(Database)
message(">>>>>>>>>>>>>>>> Database")

set(HEADERS
    ${PROJECT_NAME}.hpp
)

set(SOURCES
    ${PROJECT_NAME}.cpp
    DatabaseEvent.cpp
    DatabaseForward.cpp
)

add_library(${PROJECT_NAME}
    ${HEADERS}
    ${SOURCES}
)

target_link_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/../../compiler/libs_ubuntu/lib_sqlite-3.36/lib
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../../compiler/libs_ubuntu/lib_sqlite-3.36/include
)

target_link_libraries(${PROJECT_NAME}
    common
    json
    base64
    sqlite3
)