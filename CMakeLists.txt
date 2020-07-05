cmake_minimum_required(VERSION 2.4)

project(drmtest)

set(SRC_FILES main.c getfd.c)
set(LIBS -ldrm)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries( ${PROJECT_NAME} ${LIBS} -lstdc++fs)
target_include_directories(${PROJECT_NAME} PRIVATE  "/usr/include/libdrm/")
add_definitions("-Wall -O2 -flto -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")


set(CMAKE_INSTALL_PREFIX "/usr")
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
