add_library(PeterTelemetry STATIC
  src/JsonlTelemetrySink.cpp
  src/QualityMetrics.cpp
  src/TelemetryModule.cpp
)

target_include_directories(PeterTelemetry
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PeterTelemetry
  PUBLIC
    PeterCore
)

if(WIN32)
  target_link_libraries(PeterTelemetry PRIVATE Psapi)
endif()

target_compile_features(PeterTelemetry PUBLIC cxx_std_20)

peter_enable_strict_warnings(PeterTelemetry)

add_custom_target(PeterTelemetry_tests_placeholder)
