add_library(PeterWorld STATIC
  src/ContentCatalog.cpp
  src/SceneShell.cpp
  src/SliceContent.cpp
  src/WorldModule.cpp
)

target_include_directories(PeterWorld
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PeterWorld
  PUBLIC
    PeterAI
    PeterAdapters
    PeterCore
    PeterInventory
    PeterWorkshop
)

target_compile_features(PeterWorld PUBLIC cxx_std_20)

peter_enable_strict_warnings(PeterWorld)

add_custom_target(PeterWorld_tests_placeholder)
