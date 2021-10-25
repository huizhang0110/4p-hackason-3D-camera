set(TARGET camerawrapper)

project(${TARGET})

include_directories(
    /home/zhui/Desktop/3d/OpenNI-Linux-x64-2.3.0.66/Include
    /home/zhui/Desktop/3d/OpenNI-Linux-x64-2.3.0.66/demo_deps/include
    /usr/local/include
)

link_directories(
    /home/zhui/Desktop/3d/OpenNI-Linux-x64-2.3.0.66/Redist
    /usr/local/lib
)

file(GLOB SRC "*.h" "*.cpp")
#set(SRC CameraWrapper.cpp)

add_subdirectory(pybind11) 

# dd_executable(${TARGET} ${SRC})
set(LIBARY_OUTPUT_PATH, /home/zhui/Desktop/3d/d2c/lib) 
# add_library(camerawrapper SHARED ${SRC})

pybind11_add_module(camerawrapper SHARED ${SRC})

target_link_libraries(${TARGET} OpenNI2 opencv_core opencv_imgcodecs opencv_highgui opencv_imgproc opencv_calib3d)
