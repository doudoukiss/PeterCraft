add_library(PeterInventory STATIC
  src/InventoryState.cpp
  src/InventoryModule.cpp
)

target_include_directories(PeterInventory
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PeterInventory
  PUBLIC
    PeterCore
)

target_compile_features(PeterInventory PUBLIC cxx_std_20)

peter_enable_strict_warnings(PeterInventory)

add_custom_target(PeterInventory_tests_placeholder)
