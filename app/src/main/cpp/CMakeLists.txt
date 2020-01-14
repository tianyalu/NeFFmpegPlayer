cmake_minimum_required(VERSION 3.4.1)

set(ffmpeg ${CMAKE_SOURCE_DIR}/ffmpeg)
include_directories(${ffmpeg}/include)  # path;%JAVA_HOME%
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${ffmpeg}/libs/${CMAKE_ANDROID_ARCH_ABI}") #追加

file(GLOB src_files *.cpp)

add_library( # Sets the name of the library.
        native-lib
        SHARED
        ${src_files}) #相对路径

target_link_libraries( # Specifies the target library.
        native-lib
        -Wl,--start-group  # (WL)以group的形式链接库避免链接顺序的问题
        avcodec avfilter avformat avutil swresample swscale
        -Wl,--end-group
        z
        log # liblog.so
        android # libandroid.so
        OpenSLES
        )