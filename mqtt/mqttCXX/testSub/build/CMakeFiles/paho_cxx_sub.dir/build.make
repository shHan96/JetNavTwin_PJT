# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/pi/mqtt/mqttCXX/testSub

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pi/mqtt/mqttCXX/testSub/build

# Include any dependencies generated for this target.
include CMakeFiles/paho_cxx_sub.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/paho_cxx_sub.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/paho_cxx_sub.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/paho_cxx_sub.dir/flags.make

CMakeFiles/paho_cxx_sub.dir/main.cpp.o: CMakeFiles/paho_cxx_sub.dir/flags.make
CMakeFiles/paho_cxx_sub.dir/main.cpp.o: /home/pi/mqtt/mqttCXX/testSub/main.cpp
CMakeFiles/paho_cxx_sub.dir/main.cpp.o: CMakeFiles/paho_cxx_sub.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/mqtt/mqttCXX/testSub/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/paho_cxx_sub.dir/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/paho_cxx_sub.dir/main.cpp.o -MF CMakeFiles/paho_cxx_sub.dir/main.cpp.o.d -o CMakeFiles/paho_cxx_sub.dir/main.cpp.o -c /home/pi/mqtt/mqttCXX/testSub/main.cpp

CMakeFiles/paho_cxx_sub.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/paho_cxx_sub.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/mqtt/mqttCXX/testSub/main.cpp > CMakeFiles/paho_cxx_sub.dir/main.cpp.i

CMakeFiles/paho_cxx_sub.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/paho_cxx_sub.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/mqtt/mqttCXX/testSub/main.cpp -o CMakeFiles/paho_cxx_sub.dir/main.cpp.s

# Object files for target paho_cxx_sub
paho_cxx_sub_OBJECTS = \
"CMakeFiles/paho_cxx_sub.dir/main.cpp.o"

# External object files for target paho_cxx_sub
paho_cxx_sub_EXTERNAL_OBJECTS =

paho_cxx_sub: CMakeFiles/paho_cxx_sub.dir/main.cpp.o
paho_cxx_sub: CMakeFiles/paho_cxx_sub.dir/build.make
paho_cxx_sub: CMakeFiles/paho_cxx_sub.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/pi/mqtt/mqttCXX/testSub/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable paho_cxx_sub"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/paho_cxx_sub.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/paho_cxx_sub.dir/build: paho_cxx_sub
.PHONY : CMakeFiles/paho_cxx_sub.dir/build

CMakeFiles/paho_cxx_sub.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/paho_cxx_sub.dir/cmake_clean.cmake
.PHONY : CMakeFiles/paho_cxx_sub.dir/clean

CMakeFiles/paho_cxx_sub.dir/depend:
	cd /home/pi/mqtt/mqttCXX/testSub/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pi/mqtt/mqttCXX/testSub /home/pi/mqtt/mqttCXX/testSub /home/pi/mqtt/mqttCXX/testSub/build /home/pi/mqtt/mqttCXX/testSub/build /home/pi/mqtt/mqttCXX/testSub/build/CMakeFiles/paho_cxx_sub.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/paho_cxx_sub.dir/depend

