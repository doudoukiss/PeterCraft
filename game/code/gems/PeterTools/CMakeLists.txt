add_library(PeterTools STATIC
  src/ScenarioHarness.cpp
  src/ToolsModule.cpp
)

target_include_directories(PeterTools
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PeterTools
  PUBLIC
    PeterAI
)

target_compile_features(PeterTools PUBLIC cxx_std_20)

peter_enable_strict_warnings(PeterTools)

add_custom_target(PeterTools_tests_placeholder)
