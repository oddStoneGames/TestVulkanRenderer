cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Test-Vulkan-Renderer)

# VULKAN SDK
find_package(Vulkan)

if(NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
    set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
    STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
    set(VULKAN_LIB_LIST "vulkan-1")
    list(APPEND INCLUDES ${VULKAN_PATH}/Include)
    list(APPEND LINK_DIRS ${VULKAN_PATH}/Bin;${VULKAN_PATH}/Lib;)
    list(APPEND LIBS ${VULKAN_LIB_LIST})
endif()

if(NOT Vulkan_FOUND)
    message("Error: Unable to locate Vulkan SDK!")
endif()

# GLFW
add_subdirectory(vendor/glfw)
list(APPEND LIBS glfw)

# STB_IMAGE
list(APPEND INCLUDES vendor/stb)

# GLM
list(APPEND INCLUDES vendor/glm)

# Add extra libraries based on the operating system.
if(WIN32)
    list(APPEND gdi32 user32)
else()
    list(APPEND LIBS X11 Xxf86vm Xrandr pthread Xi dl Xinerama Xcursor)
endif()

set(SOURCE_FILES src/Engine.cpp src/Engine.hpp src/EntryPoint.cpp)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Set this project as startup project
if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()

# Define project properties
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/bin/Debug)
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/bin/Release)
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_CURRENT_BINARY_DIR}/bin/MinSizeRel)
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_BINARY_DIR}/bin/RelWithDebInfo)

# LINKER AND COMPILER OPTIONS
target_compile_definitions(${PROJECT_NAME} PUBLIC PROJECT_DIR="${PROJECT_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
target_link_directories(${PROJECT_NAME} PUBLIC ${LINK_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})