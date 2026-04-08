add_executable(PeterCraftApp
  Phase1Slice.cpp
  PeterCraftApp.cpp
  main.cpp
)

target_include_directories(PeterCraftApp
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(PeterCraftApp
  PRIVATE
    PeterAdapters
    PeterAI
    PeterCombat
    PeterCore
    PeterDebug
    PeterInventory
    PeterProgression
    PeterTelemetry
    PeterTools
    PeterTraversal
    PeterUI
    PeterValidation
    PeterWorkshop
    PeterWorld
)

target_compile_features(PeterCraftApp PRIVATE cxx_std_20)

peter_enable_strict_warnings(PeterCraftApp)
