cmake_minimum_required(VERSION 3.15)

add_library(emacs-module-base STATIC
  "${CMAKE_CURRENT_LIST_DIR}/src/emacs-module.cpp")
target_include_directories(emacs-module-base PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>)
set_target_properties(emacs-module-base PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON)
target_compile_options(emacs-module-base PRIVATE "-fPIC")

function(mark_emacs_module target)
  set_target_properties(${target} PROPERTIES
	PREFIX "")
  target_link_libraries(${target} PRIVATE emacs-module-base)
endfunction ()
