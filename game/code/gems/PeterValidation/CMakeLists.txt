add_library(PeterValidation STATIC
  src/ValidationModule.cpp
)

target_include_directories(PeterValidation
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PeterValidation
  PUBLIC
    PeterAdapters
    PeterAI
    PeterInventory
    PeterUI
    PeterWorld
)

target_compile_features(PeterValidation PUBLIC cxx_std_20)

peter_enable_strict_warnings(PeterValidation)

add_custom_target(PeterValidation_tests_placeholder)
