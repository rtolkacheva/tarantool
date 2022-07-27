set(LPM_TARGET external.protobuf_mutator)
set(LPM_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${LPM_TARGET})
set(LPM_REPO https://github.com/google/libprotobuf-mutator.git)

set(LPM_INCLUDE_DIRS ${LPM_INSTALL_DIR}/include)
include_directories(${LPM_INCLUDE_DIRS})
include_directories(${LPM_INCLUDE_DIRS}/libprotobuf-mutator)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(LPM_LIBRARIES protobuf-mutator-libfuzzer protobuf-mutator) # this exact order
foreach(lib ${LPM_LIBRARIES})
  if (MSVC)
    set(LIB_PATH ${LPM_INSTALL_DIR}/lib/lib${lib}.lib)
  else()
    set(LIB_PATH ${LPM_INSTALL_DIR}/lib/lib${lib}.a)
  endif()
  list(APPEND LPM_BUILD_BYPRODUCTS ${LIB_PATH})
  add_library(${lib} STATIC IMPORTED)
  set_property(TARGET ${lib} PROPERTY IMPORTED_LOCATION
               ${LIB_PATH})
  add_dependencies(${lib} ${LPM_TARGET}) # ??????????
endforeach(lib)

include (ExternalProject)
ExternalProject_Add(${LPM_TARGET}
  PREFIX ${LPM_TARGET}
  GIT_REPOSITORY ${LPM_REPO}
  GIT_TAG master
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND ${CMAKE_COMMAND} ${LPM_INSTALL_DIR}/src/${LPM_TARGET}
        -G${CMAKE_GENERATOR}
        -DCMAKE_INSTALL_PREFIX=${LPM_INSTALL_DIR}
        -DCMAKE_INSTALL_LIBDIR=lib
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
        -DCMAKE_C_FLAGS=${LPM_CFLAGS}
        -DCMAKE_CXX_FLAGS=${LPM_CXXFLAGS}
        -DLIB_PROTO_MUTATOR_DOWNLOAD_PROTOBUF=ON
        -DLIB_PROTO_MUTATOR_WITH_ASAN=ON
  BUILD_BYPRODUCTS ${LPM_BUILD_BYPRODUCTS}
)