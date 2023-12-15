set(Simplest_Player_SOURCE_FILES
  main.cpp
)
add_executable(simplest_player ${Simplest_Player_SOURCE_FILES})

target_include_directories(simplest_player PUBLIC SDL2 external_ffmpeg)
target_link_libraries(simplest_player SDL2 external_ffmpeg)