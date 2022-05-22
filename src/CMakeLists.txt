
cmake_minimum_required(VERSION 3.0) # specifies the minimum version of CMake we need
cmake_policy(SET CMP0043 NEW) # avoids warnings related to a deprecated policy (optional, just for convenience)

project(final_project)

# here we specify where the libraries we want are located
set(LIBS_DIR "C:/libs")

set(VTK_DIR "${LIBS_DIR}/vtk 8.2.0/bin/") # vtk
set(CMAKE_PREFIX_PATH "${LIBS_DIR}/qt/5.7/msvc2015_64/") # qt
set(OpenCV_DIR "${LIBS_DIR}/opencv 4.3.0/") # opencv

# this tells the compiler to use multi-threaded builds (related to performance)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

# find vtk
find_package(VTK REQUIRED) # required means CMake should give an error if it can't find it
include(${VTK_USE_FILE})

# find qt
set(CMAKE_AUTOMOC ON)
find_package(Qt5Widgets REQUIRED QUIET) # quiet means suppress unneeded output text from CMake

# find opencv
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# collect all source files (.h and .cxx)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
file(GLOB H_FILES *.h)
file(GLOB CXX_FILES *.cxx)

# create executable project
add_executable(final_project ${CXX_FILES} ${H_FILES})

# tell qt which components we want (qt has lots of modules)
qt5_use_modules(final_project Core Gui Widgets Sql Network winextras)

# set the executable to be the primary project (the one that runs)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT final_project)

# link libraries (qt is automatically linked by the qt5_use_modules statement earlier)
target_link_libraries(final_project ${VTK_LIBRARIES}) # vtk
target_link_libraries(final_project ${OpenCV_LIBS}) # opencv

# set the path so the DLLs can be found at runtime
set_target_properties(final_project PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=${OpenCV_DIR}/x64/vc15/bin;${VTK_DIR}/bin/$(Configuration);${CMAKE_PREFIX_PATH}/bin;%PATH%")
