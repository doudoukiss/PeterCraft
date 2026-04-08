add_library(PeterAI STATIC
  src/AiModule.cpp
)

target_include_directories(PeterAI
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(PeterAI PUBLIC cxx_std_20)

peter_enable_strict_warnings(PeterAI)

add_custom_target(PeterAI_tests_placeholder)
