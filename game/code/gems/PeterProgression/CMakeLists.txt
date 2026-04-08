add_library(PeterProgression STATIC
  src/ProgressionModule.cpp
)

target_include_directories(PeterProgression
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(PeterProgression PUBLIC cxx_std_20)

peter_enable_strict_warnings(PeterProgression)

add_custom_target(PeterProgression_tests_placeholder)
