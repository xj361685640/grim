add_library(physics physics.cpp physics.hpp riemannsolver.cpp tetrads.cpp)
target_link_libraries(physics reconstruction grid geometry problem)

set_source_files_properties(physicsPy.pyx PROPERTIES CYTHON_IS_CXX TRUE)
cython_add_module(physicsPy physicsPy.pyx)
target_link_libraries(physicsPy physics grid geometry ${ArrayFire_LIBRARIES})
