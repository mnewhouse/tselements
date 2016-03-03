cmake_minimum_required(VERSION 2.8.12)
project(tselements)

set(STATIC_STD_LIBS OFF)


if(MSVC)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
endif()
    
    
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

set(EXECUTABLE_NAME "tselements")

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_USE_MULTITHREADED ON)
set(SFML_STATIC_LIBRARIES OFF)

include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(${EXECUTABLE_NAME} ${SRC})

find_package(Boost REQUIRED COMPONENTS system filesystem locale)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    add_definitions("-DHAS_BOOST")
    target_link_libraries(${EXECUTABLE_NAME} ${Boost_LIBRARIES})
endif()

find_package(SFML 2 REQUIRED system window graphics network audio)
if(SFML_FOUND)
  include_directories(${SFML_INCLUDE_DIR})
  target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES})
endif()

if (MSVC)
  target_link_libraries(${EXECUTABLE_NAME} "winmm.lib")
endif()
