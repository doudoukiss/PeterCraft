add_library(PeterCombat STATIC
  src/EncounterSimulator.cpp
  src/CombatModule.cpp
)

target_include_directories(PeterCombat
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PeterCombat
  PUBLIC
    PeterAI
    PeterCore
)

target_compile_features(PeterCombat PUBLIC cxx_std_20)

peter_enable_strict_warnings(PeterCombat)

add_custom_target(PeterCombat_tests_placeholder)
