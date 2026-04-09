set(PETER_ADAPTERS_SOURCES
  src/NullPlatformServices.cpp
)

if(PETERCRAFT_ENABLE_PLAYABLE_RUNTIME AND WIN32)
  list(APPEND PETER_ADAPTERS_SOURCES
    o3de/O3DEPlatformServices.cpp
  )
endif()

add_library(PeterAdapters STATIC
  ${PETER_ADAPTERS_SOURCES}
)

target_include_directories(PeterAdapters
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(PeterAdapters PUBLIC cxx_std_20)
target_compile_definitions(PeterAdapters
  PUBLIC
    PETERCRAFT_ENABLE_PLAYABLE_RUNTIME=$<BOOL:${PETERCRAFT_ENABLE_PLAYABLE_RUNTIME}>
)

peter_enable_strict_warnings(PeterAdapters)
