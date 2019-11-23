cmake_minimum_required (VERSION 3.11)

add_subdirectory(third_party)

project(Pigami
  LANGUAGES CXX
  VERSION 1.0.0
)

add_executable(pigami
  src/Framebuffer.cpp
  src/Framebuffer.hpp
  src/Generator.cpp
  src/Generator.hpp
  src/Resources.cpp
  src/Resources.hpp
  src/activity/Activity.hpp
  src/activity/GenerateScreen.cpp
  src/activity/GenerateScreen.hpp
  src/activity/GeneratorScreen.cpp
  src/activity/GeneratorScreen.hpp
  src/activity/IntroScreen.cpp
  src/activity/IntroScreen.hpp
  src/activity/LevelScreen.cpp
  src/activity/LevelScreen.hpp
  src/activity/LevelSelector.cpp
  src/activity/LevelSelector.hpp
  src/activity/ResourceLoadingScreen.cpp
  src/activity/ResourceLoadingScreen.hpp
  src/index.html
  src/main.cpp
  src/plateau.cpp
  src/plateau.h
  src/save.cpp
  src/save.hpp
)
target_include_directories(pigami PRIVATE ${CMAKE_CURRENT_DIR}/src)
target_include_directories(pigami PRIVATE src)
target_link_libraries(pigami smk)
set_target_properties(pigami PROPERTIES CXX_STANDARD 17)

target_compile_options(pigami PRIVATE -Wall -Wextra -pedantic -Werror)
target_compile_options(pigami PRIVATE -fno-rtti)
target_compile_options(pigami PRIVATE -fno-exceptions)

# Detect emscripten is used.
if(CMAKE_CXX_COMPILER MATCHES "/em\\+\\+(-[a-zA-Z0-9.])?$") 
  target_link_libraries(pigami -lidbfs.js)

  #set_property(TARGET pigami APPEND_STRING PROPERTY LINK_FLAGS  " -s DEMANGLE_SUPPORT=1")
  set_property(TARGET pigami APPEND_STRING PROPERTY LINK_FLAGS  " -s DISABLE_EXCEPTION_CATCHING=2")
  set_property(TARGET pigami APPEND_STRING PROPERTY LINK_FLAGS  " -s WASM=1")
  set_property(TARGET pigami APPEND_STRING PROPERTY LINK_FLAGS  " -s TOTAL_MEMORY=134217728")

  # Release flags
  #set_property(TARGET pigami APPEND_STRING PROPERTY LINK_FLAGS  " --closure 1")
  set_property(TARGET pigami APPEND_STRING PROPERTY LINK_FLAGS  " --js-opts 3")
  set_property(TARGET pigami APPEND_STRING PROPERTY LINK_FLAGS  " --llvm-lto 3")
  set_property(TARGET pigami APPEND_STRING PROPERTY LINK_FLAGS  " --llvm-opts 3")
  set_property(TARGET pigami APPEND_STRING PROPERTY LINK_FLAGS  " -O3")

  # Analytics.
  option(ADD_GOOGLE_ANALYTICS "Add google analytics script" ON)
  if (ADD_GOOGLE_ANALYTICS)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/src/google-analytics.html google-analytics)
  endif(ADD_GOOGLE_ANALYTICS)

  # Copy the index.html file.
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/index.html
    ${CMAKE_CURRENT_BINARY_DIR}/index.html
  )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/style.css
    ${CMAKE_CURRENT_BINARY_DIR}/style.css
    COPYONLY
  )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/favicon.png
    ${CMAKE_CURRENT_BINARY_DIR}/favicon.png
    COPYONLY
  )

  # Allow some files to be fetched.
  file(GLOB files "./resources/*" "./resources/*/*")
  foreach(file ${files})
    file(RELATIVE_PATH relative_file ${CMAKE_SOURCE_DIR} ${file})
    set_property(TARGET pigami APPEND_STRING PROPERTY LINK_FLAGS  " --preload-file ${file}@/${relative_file}")
  endforeach()
else()
  target_link_libraries(pigami stdc++fs)
endif()

install(TARGETS pigami RUNTIME DESTINATION "bin")
install(DIRECTORY resources DESTINATION share/pigami)
