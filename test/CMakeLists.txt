target_sources(slo_test PRIVATE 
  main.cpp
)

add_subdirectory(variant)
add_subdirectory(union)
add_subdirectory(util)

target_include_directories(slo_test PRIVATE .)