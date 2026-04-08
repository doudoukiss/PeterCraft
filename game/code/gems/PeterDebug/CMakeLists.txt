add_library(PeterDebug STATIC
  src/DebugModule.cpp
  src/DebugOverlay.cpp
)

target_include_directories(PeterDebug
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(PeterDebug PUBLIC cxx_std_20)

peter_enable_strict_warnings(PeterDebug)

add_custom_target(PeterDebug_tests_placeholder)
