cmake_minimum_required(VERSION 3.22.1)
project("native_cpp_app")

add_library(app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)

add_library(native_cpp_app SHARED
    main.cpp
)

target_include_directories(native_cpp_app PRIVATE
    ${ANDROID_NDK}/sources/android/native_app_glue
)

find_library(log-lib log)
find_library(android-lib android)
find_library(egl-lib EGL)
find_library(gles-lib GLESv2)

target_link_libraries(native_cpp_app
    app_glue
    ${log-lib}
    ${android-lib}
    ${egl-lib}
    ${gles-lib}
)