cmake_minimum_required(VERSION 3.15)

function(add_emacs_module name sources)
  add_library(${name} SHARED
	"${CMAKE_SOURCE_DIR}/src/emacs-module.cpp"
	${sources})
  set_target_properties(${name} PROPERTIES
	PREFIX ""
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON)
  target_include_directories(${name} PRIVATE
	"${CMAKE_SOURCE_DIR}/src/")
endfunction ()
