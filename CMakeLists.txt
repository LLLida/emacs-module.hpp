cmake_minimum_required(VERSION 3.15)

add_library(emacs-module-base STATIC
  "${CMAKE_CURRENT_LIST_DIR}/src/emacs-module.cpp")
target_include_directories(emacs-module-base PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>)
set_target_properties(emacs-module-base PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON)

function(add_emacs_module name sources)
  add_library(${name} SHARED
	${sources})
  set_target_properties(${name} PROPERTIES
	PREFIX "")
  target_link_libraries(${name} PRIVATE emacs-module-base)
endfunction ()
