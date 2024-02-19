cmake_minimum_required(VERSION 3.27)
project(LinuxVideoCaptures)

set(CMAKE_CXX_STANDARD 17)

# /usr/include/SDL2/SDL_image.h
#set(CMAKE_PREFIX_PATH "/usr/include/SDL2/SDL_image.h")
#set(SDL2_IMAGE_DIR "/usr/include/SDL2/SDL_image.h")

# -lSDL2_image

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lSDL2_image")
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(SDL2 REQUIRED)
#find_package(SDL2_image REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

# Find JPEG
find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIR})

add_executable(LinuxVideoCaptures
        main.cpp
        V4L2Camera.cpp
        V4L2Camera.h
        VideoFrame.cpp
        VideoFrame.h
        VIdeoFramesStructs.h
        WindowInterface.cpp
        WindowInterface.h
)

target_link_libraries(LinuxVideoCaptures
        ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES}
        ${JPEG_LIBRARIES}
        -lSDL2_image
)
