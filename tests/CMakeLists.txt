add_executable(PeterCoreUnitTests
  unit/PeterCoreUnitTests.cpp
)

target_link_libraries(PeterCoreUnitTests
  PRIVATE
    PeterAI
    PeterCombat
    PeterCore
    PeterInventory
    PeterProgression
    PeterTelemetry
    PeterTraversal
    PeterUI
    PeterValidation
    PeterWorkshop
    PeterWorld
)

target_include_directories(PeterCoreUnitTests
  PRIVATE
    ${CMAKE_SOURCE_DIR}/game/code/common/include
)

target_compile_features(PeterCoreUnitTests PRIVATE cxx_std_20)
peter_enable_strict_warnings(PeterCoreUnitTests)
add_test(NAME unit_core_contracts COMMAND PeterCoreUnitTests)

add_executable(PeterFoundationIntegrationTests
  integration/PeterFoundationIntegrationTests.cpp
)

target_link_libraries(PeterFoundationIntegrationTests
  PRIVATE
    PeterAdapters
    PeterAI
    PeterCore
    PeterInventory
    PeterTelemetry
    PeterUI
    PeterValidation
    PeterWorkshop
    PeterWorld
)

target_include_directories(PeterFoundationIntegrationTests
  PRIVATE
    ${CMAKE_SOURCE_DIR}/game/code/common/include
)

target_compile_features(PeterFoundationIntegrationTests PRIVATE cxx_std_20)
peter_enable_strict_warnings(PeterFoundationIntegrationTests)
add_test(NAME integration_profile_and_telemetry COMMAND PeterFoundationIntegrationTests)

add_executable(PeterScenarioTests
  scenarios/PeterScenarioTests.cpp
)

target_link_libraries(PeterScenarioTests
  PRIVATE
    PeterAI
    PeterCombat
    PeterInventory
    PeterProgression
    PeterTools
    PeterValidation
    PeterWorld
)

target_include_directories(PeterScenarioTests
  PRIVATE
    ${CMAKE_SOURCE_DIR}/game/code/common/include
)

target_compile_features(PeterScenarioTests PRIVATE cxx_std_20)
peter_enable_strict_warnings(PeterScenarioTests)
add_test(NAME scenario_deterministic_shell COMMAND PeterScenarioTests)

add_test(
  NAME slice_happy_path
  COMMAND $<TARGET_FILE:PeterCraftApp> --scenario happy_path --profile-id player.test.happy --no-settings
)

add_test(
  NAME slice_failure_path
  COMMAND $<TARGET_FILE:PeterCraftApp> --scenario failure_path --profile-id player.test.failure --no-settings
)

add_test(
  NAME smoke_vertical_slice
  COMMAND $<TARGET_FILE:PeterCraftApp> --smoke-test --profile-id player.test.smoke --no-settings
)

add_test(
  NAME slice_artifact_recovery
  COMMAND $<TARGET_FILE:PeterCraftApp> --scenario artifact_recovery --profile-id player.test.artifact --no-settings
)

add_test(
  NAME slice_escort_support
  COMMAND $<TARGET_FILE:PeterCraftApp> --scenario escort_support --profile-id player.test.escort --no-settings
)
