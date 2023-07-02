cmake_minimum_required (VERSION 3.21.00)

#function to make it easier to 
function(collect_and_filter_source_files folder out_files)
	
	#glob the files
	file(GLOB_RECURSE files CONFIGURE_DEPENDS
		"${folder}/*.h"
		"${folder}/*.cpp"
		"${folder}/*.inl")
	message("${files}")
	set(${out_files} ${files} PARENT_SCOPE)
	
	#create filters for the files by taking their path relative to the project root
	set(root_path "${CMAKE_CURRENT_SOURCE_DIR}/${folder}")
	
	foreach(source IN LISTS files)
		#path of the source file, relative to project root path
		get_filename_component(source_path "${source}" PATH)
		string(REPLACE ${root_path} "" source_relative "${source_path}")
		string(REPLACE "/" "\\" filter_path "${source_relative}")
		
		#set filter
		source_group("Files/${filter_path}" FILES "${source}")
			
	endforeach()
	
endfunction()

#solution start
project(FbxAnimation)

collect_and_filter_source_files("FbxAnimation/FbxAnimation" FbxAnimationFiles)

add_executable(FBXAnimation "${FbxAnimationFiles}")
