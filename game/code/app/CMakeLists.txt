add_executable(PeterCraftApp
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
    PeterCore
    PeterDebug
    PeterTelemetry
    PeterTools
    PeterUI
    PeterValidation
    PeterWorld
)

target_compile_features(PeterCraftApp PRIVATE cxx_std_20)

peter_enable_strict_warnings(PeterCraftApp)
