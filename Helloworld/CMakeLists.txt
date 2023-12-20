set(Helloworld_SOURCE_FILES
  main.cpp
)
add_executable(helloworld ${Helloworld_SOURCE_FILES})
target_include_directories(helloworld PUBLIC SDL2-static)
target_link_libraries(helloworld SDL2-static)