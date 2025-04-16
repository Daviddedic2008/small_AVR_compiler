# CMakeList.txt : CMake project for avrCompilerCmake, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("avrCompilerCmake")

# Add source to this project's executable.
add_executable (avrCompilerCmake "avrCompilerCmake.cpp" "avrCompilerCmake.h" "src/compiler.h" "src/headers/parser.h" "src/headers/tokenizer.h" "src/headers/assemblyGenerator.h" "src/headers/fileToString.h" "src/tokenizer.cpp" "src/parser.cpp" "src/fileToString.cpp" "src/assemblyGenerator.cpp" "src/headers/variableManager.h" "src/variableManager.cpp" "src/headers/opcodes.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET avrCompilerCmake PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
