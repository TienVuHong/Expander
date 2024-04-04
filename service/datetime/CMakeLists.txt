project(datetime)
message(">>>>>>>>>>>>>>> service: datetime")

set(HEADERS
    ${PROJECT_NAME}.hpp
)

set(SOURCES
    ${PROJECT_NAME}.cpp
)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    common
)