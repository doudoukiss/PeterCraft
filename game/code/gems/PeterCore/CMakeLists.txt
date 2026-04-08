add_library(PeterCore STATIC
  src/CoreModule.cpp
  src/EventBus.cpp
  src/FeatureRegistry.cpp
  src/ProfileService.cpp
  src/StableId.cpp
)

target_include_directories(PeterCore
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PeterCore
  PUBLIC
    PeterAdapters
)

target_compile_features(PeterCore PUBLIC cxx_std_20)

peter_enable_strict_warnings(PeterCore)

add_custom_target(PeterCore_tests_placeholder)
