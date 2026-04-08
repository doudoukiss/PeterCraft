add_library(PeterAdapters STATIC
  src/NullPlatformServices.cpp
)

target_include_directories(PeterAdapters
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(PeterAdapters PUBLIC cxx_std_20)

peter_enable_strict_warnings(PeterAdapters)
