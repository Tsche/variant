target_sources(slo_test PRIVATE 
  lifetime.cpp 
  visit.cpp
  special_members.cpp
  constructors.cpp
  padding.cpp)

add_subdirectory(variant.helper)
add_subdirectory(variant.get)
add_subdirectory(variant.bad.access)