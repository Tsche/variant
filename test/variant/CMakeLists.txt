target_sources(slo_test PRIVATE 
  lifetime.cpp 
  special_members.cpp
  padding.cpp)

add_subdirectory(variant.bad.access)
add_subdirectory(variant.get)
add_subdirectory(variant.helper)
add_subdirectory(variant.visit)
add_subdirectory(variant.ctor)
add_subdirectory(variant.hash)