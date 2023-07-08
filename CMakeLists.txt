cmake_minimum_required (VERSION 3.21.00)

#functions to standardise project creation

function(collect_and_filter_source_files folder out_files)
	
	#glob the files
	file(GLOB_RECURSE files CONFIGURE_DEPENDS
		"${folder}/*.h"
		"${folder}/*.cpp"
		"${folder}/*.c"
		"${folder}/*.inl")
	set(${out_files} ${files} PARENT_SCOPE)
	
	#create filters for the files by taking their path relative to the project root
	set(root_path "${CMAKE_CURRENT_SOURCE_DIR}/${folder}")
	
	foreach(source IN LISTS files)
		#path of the source file, relative to project root path
		get_filename_component(source_path "${source}" PATH)
		string(REPLACE ${root_path} "" source_relative "${source_path}")
		string(REPLACE "/" "\\" filter_path "${source_relative}")
		
		#set filter
		source_group("${filter_path}" FILES "${source}")
		
	endforeach()
	
endfunction()

function(create_library name path)
	collect_and_filter_source_files("${path}/${name}" Files)
	add_library("${name}" STATIC "${Files}")
	set_target_properties("${name}" PROPERTIES LINKER_LANGUAGE CXX)
	target_include_directories("${name}" PUBLIC "${path}/${name}/include")
	target_include_directories("${name}" PRIVATE "${path}/${name}/source")
	target_include_directories("${name}" PRIVATE "${path}/${name}/include/${name}")
	
	#remaining elements are targets
	foreach(i RANGE 2 ${ARGC})
		target_link_libraries("${name}" "${ARGV${i}}")
	endforeach()
endfunction()

#global settings

set (CMAKE_CXX_STANDARD 20)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#solution start
project(FbxAnimation)

#create libraries

#third party without source
add_library(glfw INTERFACE)
target_include_directories(glfw INTERFACE "external/glfw-3.3.6/glfw-3.3.6/include")
target_link_libraries(glfw INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw-3.3.6/glfw-3.3.6/build/src/release/glfw3.lib")

add_library(FbxSdk INTERFACE)
target_include_directories(FbxSdk INTERFACE "external/FBX SDK/2020.0.1/include")
target_link_libraries(FbxSdk INTERFACE optimized "${CMAKE_CURRENT_SOURCE_DIR}/external/FBX SDK/2020.0.1/lib/vs2017/x64/release/libfbxsdk.lib")
target_link_libraries(FbxSdk INTERFACE debug "${CMAKE_CURRENT_SOURCE_DIR}/external/FBX SDK/2020.0.1/lib/vs2017/x64/debug/libfbxsdk.lib")
target_compile_definitions(FbxSdk INTERFACE "FBXSDK_SHARED")

#third party with source
create_library(glad "external")
create_library(imgui "external" glfw)

#own libraries
create_library(maths "source")
create_library("file" "source")
create_library(animation "source" maths)
create_library(graphics "source" maths glad)

#create executable
collect_and_filter_source_files("source/launch" LaunchFiles)
add_executable(launch "${LaunchFiles}")
target_link_libraries(launch
	animation maths imgui "file" graphics glad glfw FbxSdk)

#group projects
set_target_properties(glad imgui PROPERTIES FOLDER "ThirdPartyLibs")
set_target_properties(animation maths "file" graphics PROPERTIES FOLDER "Libraries")
set_target_properties(launch PROPERTIES FOLDER "Executables")