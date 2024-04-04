project(timer)
message(">>>>>>>>>>>>>>> service: timer")

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