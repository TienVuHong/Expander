project(EventManager)
message(">>>>>>>>>>>>>>>> EventManager")

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

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    json
    Database
)