add_library(PeterWorkshop STATIC
  src/WorkshopTuning.cpp
  src/WorkshopModule.cpp
)

target_include_directories(PeterWorkshop
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PeterWorkshop
  PUBLIC
    PeterValidation
)

target_compile_features(PeterWorkshop PUBLIC cxx_std_20)

peter_enable_strict_warnings(PeterWorkshop)

add_custom_target(PeterWorkshop_tests_placeholder)
