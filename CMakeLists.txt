cmake_minimum_required(VERSION 3.23)

project(audio-engine)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(audio-engine)

target_sources(audio-engine
  PRIVATE
    source/main.cpp
    source/file/file.cpp
    source/coreaudio/device/device.cpp
    source/engine/engine.cpp
    source/coreaudio/IOProc/IOProc.cpp
)

# Link Core Audio Libraries
find_library(CORE_AUDIO CoreAudio REQUIRED)
find_library(AUDIO_TOOLBOX AudioToolbox REQUIRED)
find_library(CORE_FOUNDATION CoreFoundation REQUIRED)
target_link_libraries(audio-engine PRIVATE 
    ${CORE_AUDIO} 
    ${AUDIO_TOOLBOX} 
    ${CORE_FOUNDATION}
)
