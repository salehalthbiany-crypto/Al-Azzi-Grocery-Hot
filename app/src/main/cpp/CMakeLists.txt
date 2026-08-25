cmake_minimum_required(VERSION 3.22.1)
project("native_cpp_app")

# تجميع كود C++ والمكتبيات ذاتياً بدون أي مسارات خارجية
add_library(native_cpp_app SHARED
    main.cpp
    native_app_glue.c
)

target_include_directories(native_cpp_app PRIVATE .)

find_library(log-lib log)
find_library(android-lib android)
find_library(egl-lib EGL)
find_library(gles-lib GLESv2)

target_link_libraries(native_cpp_app
    ${log-lib}
    ${android-lib}
    ${egl-lib}
    ${gles-lib}
)