set(ffplay_SOURCE_FILES
  ../FFmpeg/fftools/ffplay.c
  ../FFmpeg/fftools/cmdutils.c
  ../FFmpeg/fftools/opt_common.c
  ../FFmpeg/fftools/ffplay_renderer.c
)
add_executable(ffplay ${ffplay_SOURCE_FILES})
target_include_directories(ffplay PUBLIC SDL2-static external_ffmpeg)
target_link_libraries(ffplay SDL2-static external_ffmpeg bcrypt.lib)
# target_link_libraries(ffplay PRIVATE bcrypto.lib)