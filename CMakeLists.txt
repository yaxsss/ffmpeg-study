cmake_minimum_required(VERSION 3.16)

add_library(ffmpeg INTERFACE IMPORTED GLOBAL)

set(project_langs C CXX)

project(ffmpeg
  LANGUAGES ${project_langs}
  VERSION 1.0
  DESCRIPTION "ffmpeg study"
)

add_library(external_ffmpeg INTERFACE IMPORTED GLOBAL)
set(ffmpeg_lib_list)
set(ffmpeg_lib_loc ./FFmpeg)
target_include_directories(external_ffmpeg SYSTEM
INTERFACE 
  ${ffmpeg_lib_loc}
)
list(APPEND ffmpeg_lib_list
  ${ffmpeg_lib_loc}/libavformat/libavformat.a
  ${ffmpeg_lib_loc}/libavcodec/libavcodec.a
  ${ffmpeg_lib_loc}/libswresample/libswresample.a
  ${ffmpeg_lib_loc}/libswscale/libswscale.a
  ${ffmpeg_lib_loc}/libavutil/libavutil.a
)
target_link_libraries(external_ffmpeg 
  INTERFACE 
  ${ffmpeg_lib_list}
)

set(SDL_SHARED OFF)
set(SDL_STATIC ON)
add_subdirectory(SDL)
add_subdirectory(sdl2-compat)
add_subdirectory(Helloworld)
add_subdirectory(simplest_player)