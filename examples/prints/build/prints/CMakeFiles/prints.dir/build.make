# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.15.4/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.15.4/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/xiaoyu/work/wasm.cdt/examples/prints/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/xiaoyu/work/wasm.cdt/examples/prints/build/prints

# Include any dependencies generated for this target.
include CMakeFiles/prints.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/prints.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/prints.dir/flags.make

CMakeFiles/prints.dir/prints.obj: CMakeFiles/prints.dir/flags.make
CMakeFiles/prints.dir/prints.obj: /Users/xiaoyu/work/wasm.cdt/examples/prints/src/prints.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/xiaoyu/work/wasm.cdt/examples/prints/build/prints/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/prints.dir/prints.obj"
	/Users/xiaoyu/work/wasm.cdt/build/bin/wasm-cpp  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/prints.dir/prints.obj -c /Users/xiaoyu/work/wasm.cdt/examples/prints/src/prints.cpp

CMakeFiles/prints.dir/prints.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/prints.dir/prints.i"
	/Users/xiaoyu/work/wasm.cdt/build/bin/wasm-cpp $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/xiaoyu/work/wasm.cdt/examples/prints/src/prints.cpp > CMakeFiles/prints.dir/prints.i

CMakeFiles/prints.dir/prints.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/prints.dir/prints.s"
	/Users/xiaoyu/work/wasm.cdt/build/bin/wasm-cpp $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/xiaoyu/work/wasm.cdt/examples/prints/src/prints.cpp -o CMakeFiles/prints.dir/prints.s

# Object files for target prints
prints_OBJECTS = \
"CMakeFiles/prints.dir/prints.obj"

# External object files for target prints
prints_EXTERNAL_OBJECTS =

prints.wasm: CMakeFiles/prints.dir/prints.obj
prints.wasm: CMakeFiles/prints.dir/build.make
prints.wasm: CMakeFiles/prints.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/xiaoyu/work/wasm.cdt/examples/prints/build/prints/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable prints.wasm"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/prints.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/prints.dir/build: prints.wasm

.PHONY : CMakeFiles/prints.dir/build

CMakeFiles/prints.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/prints.dir/cmake_clean.cmake
.PHONY : CMakeFiles/prints.dir/clean

CMakeFiles/prints.dir/depend:
	cd /Users/xiaoyu/work/wasm.cdt/examples/prints/build/prints && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/xiaoyu/work/wasm.cdt/examples/prints/src /Users/xiaoyu/work/wasm.cdt/examples/prints/src /Users/xiaoyu/work/wasm.cdt/examples/prints/build/prints /Users/xiaoyu/work/wasm.cdt/examples/prints/build/prints /Users/xiaoyu/work/wasm.cdt/examples/prints/build/prints/CMakeFiles/prints.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/prints.dir/depend
