include_guard(GLOBAL)

function(peter_enable_strict_warnings target_name)
  if(NOT PETERCRAFT_ENABLE_STRICT_WARNINGS)
    return()
  endif()

  if(MSVC)
    target_compile_options(${target_name} PRIVATE /W4 /WX /permissive- /EHsc /utf-8)
  else()
    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endif()
endfunction()
