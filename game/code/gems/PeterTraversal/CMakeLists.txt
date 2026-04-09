add_library(PeterTraversal STATIC
  src/TraversalProfile.cpp
  src/TraversalModule.cpp
)

target_include_directories(PeterTraversal
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PeterTraversal
  PUBLIC
    PeterAdapters
    PeterCore
)

target_compile_features(PeterTraversal PUBLIC cxx_std_20)

peter_enable_strict_warnings(PeterTraversal)

add_custom_target(PeterTraversal_tests_placeholder)
