add_library(PeterUI STATIC
  src/MenuModel.cpp
  src/SlicePresentation.cpp
  src/UiModule.cpp
)

target_include_directories(PeterUI
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PeterUI
  PUBLIC
    PeterAdapters
    PeterAI
    PeterCore
    PeterProgression
    PeterWorkshop
    PeterWorld
)

target_compile_features(PeterUI PUBLIC cxx_std_20)

peter_enable_strict_warnings(PeterUI)

add_custom_target(PeterUI_tests_placeholder)
