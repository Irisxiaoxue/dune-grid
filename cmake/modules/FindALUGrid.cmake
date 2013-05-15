#find_package(PkgConfig)

macro(_dune_set_alugrid val)
  set(ALUGRID_FOUND ${val})
  set(HAVE_ALUGRID ${val})
endmacro(_dune_set_alugrid val)

find_package(METIS)

if(ALUGRID_ROOT)
find_path(ALUGRID_PKGCONFIG_DIR alugrid.pc PATHS ${ALUGRID_ROOT}
  PATH_SUFFIXES lib/pkgconfig/ alugrid/lib/pkgconfig NO_DEFAULT_DIR)
endif(ALUGRID_ROOT)

find_path(ALUGRID_PKGCONFIG_DIR alugrid.pc PATH_SUFFIXES lib/pkgconfig/ alugrid/lib/pkgconfig)

get_filename_component(_GUESSED_ALUGRID_ROOT ${ALUGRID_PKGCONFIG_DIR}/../../ ABSOLUTE)
find_file(ALUGRID_VERSION alugridversion PATHS ${_GUESSED_ALUGRID_ROOT}/bin
  NO_DEFAULT_PATH)

if(ALUGRID_VERSION)
  set(ALUGRID_ROOT ${_GUESSED_ALUGRID_ROOT})
else(ALUGRID_VERSION_PATH)
  get_filename_component(_GUESSED_ALUGRID_ROOT ${ALUGRID_PKGCONFIG_DIR}/../../.. ABSOLUTE)
  find_file(ALUGRID_VERSION alugridversion PATHS ${_GUESSED_ALUGRID_ROOT} PATH_SUFFIXES bin
    NO_DEFAULT_PATH)
  if(ALUGRID_VERSION)
    set(ALUGRID_ROOT ${_GUESSED_ALUGRID_ROOT})
  endif(ALUGRID_VERSION)
endif(ALUGRID_VERSION)

set(ALUGRID_VERSION_REQUIRED 1.50)

if(NOT ALUGRID_VERSION)
  message( WARNING "Could not find ALUGrid.")
  _dune_set_alugrid(FALSE)
  return()
endif(NOT ALUGRID_VERSION)

if(ALUGRID_VERSION)
  execute_process(COMMAND ${ALUGRID_VERSION} -c ${ALUGRID_VERSION_REQUIRED} OUTPUT_VARIABLE ALUGRID_VERSION)
  if(ALUGRID_VERSION LESS 0)
    message(STATUS "ALUGrid version is less then ${ALUGRID_VERSION_REQUIRED}")
    _dune_set_alugrid(FALSE)
    return()
  else(ALUGRID_VERSION LESS 0)
    message(STATUS "ALUGrid version is compatible")
  endif(ALUGRID_VERSION LESS 0)
else(ALUGRID_VERSION)
  message(WARNING "Cannot determine ALUGrid version. Giving up.")
  _dune_set_alugrid(FALSE)
    return()
endif(ALUGRID_VERSION)


find_path(ALUGRID_INCLUDE_PATH alugrid_serial.h PATHS ${ALUGRID_ROOT} PATH_SUFFIXES include include/serial NO_DEFAULT_PATH DOC "Include path of serial alugrid headers.")
find_path(ALUGRID_INCLUDE_PATH alugrid_serial.h PATH_SUFFIXES include include/serial)

find_library(ALUGRID_LIB alugrid PATHS ${ALUGRID_ROOT} PATH_SUFFIXES lib lib32 lib64
  DOC "ALUGrid library" NO_DEFAULT_PATH)
find_library(ALUGRID_LIB alugrid PATH_SUFFIXES lib lib32 lib64)


if(NOT ALUGRID_LIB)
  message(WARNING "ALUGrid library not found.")
  _dune_set_alugrid(FALSE)
  return()
endif(NOT ALUGRID_LIB)

if(ALUGRID_INCLUDE_PATH)
  cmake_push_check_state() # store rquired flags
  set(ALUGRID_INCLUDES ${ALUGRID_INCLUDE_PATH} ${ALUGRID_INCLUDE_PATH}/serial
    ${ALUGRID_INCLUDE_PATH}/duneinterface)
  set(ALUGRID_DEFINITIONS -DENABLE_ALUGRID)
  set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${ALUGRID_INCLUDES})
  set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${ALUGRID_LIB})
  set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} ${ALUGRID_DEFINITIONS})
  check_include_file_cxx(stlheaders.h HAVE_ALUGRID_SERIAL_H)
  if(HAVE_ALUGRID_SERIAL_H)
    check_cxx_source_compiles("#include <alugrid_defineparallel.h>
                              #if ALU3DGRID_BUILD_FOR_PARALLEL == 0
                              #error
                              #endif
                              int main(){}"
      ALUGRID_PARALLEL_FOUND)
  else(HAVE_ALUGRID_SERIAL_H)
    message(FATAL_ERROR "brbrr")
  endif(HAVE_ALUGRID_SERIAL_H)
  cmake_pop_check_state()
else(ALUGRID_INCLUDE_PATH)
  message(WARNING "alugrid_serial.h not found in ${ALUGRID_INCLUDE_PATH}.")
endif(ALUGRID_INCLUDE_PATH)

if(ALUGRID_PARALLEL_FOUND AND MPI_FOUND)
  # check for parallel ALUGrid
  find_path(ALUGRID_PARALLEL_INCLUDE_PATH alugrid_parallel.h PATHS ${ALUGRID_ROOT} PATH_SUFFIXES include include/parallel NO_DEFAULT_PATH)
  find_path(ALUGRID_PARALLEL_INCLUDE_PATH alugrid_parallel.h PATH_SUFFIXES include include/parallel)

  if(ALUGRID_PARALLEL_INCLUDE_PATH)
    cmake_push_check_state() # store rquired flags
    list(APPEND ALUGRID_INCLUDES ${ALUGRID_INCLUDE_PATH}/parallel)
    set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${ALUGRID_INCLUDES})
    set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${ALUGRID_LIB})
    #set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} -DSOME_MORE_DEF)
    #check_include_file_cxx(alugrid_parallel.h ALUGRID_PARALLEL_HEADER)
    message(AUTHOR_WARNING "Include file alugrid_parallel.h is not checked because
the check produces linker errors.")

    set(HAVE_ALUGRID_PARALLEL_H 1 CACHE INTERNAL "Have include alugrid_parallel.h ")
    cmake_pop_check_state()
    if(NOT HAVE_ALUGRID_PARALLEL_H)
      message(WARNING "alugrid_parallel.h not found  in ${ALUGRID_PARALLEL_INCLUDE_PATH}")
    endif(NOT HAVE_ALUGRID_PARALLEL_H)
  else(ALUGRID_PARALLEL_INCLUDE_PATH)
    message(WARNING "alugrid_parallel.h not found.")
  endif(ALUGRID_PARALLEL_INCLUDE_PATH)
endif(ALUGRID_PARALLEL_FOUND AND MPI_FOUND)

check_library_exists(${ALUGRID_LIB} malloc "" _ALULIB_FUNCTIONAL)

if(HAVE_ALUGRID_SERIAL_H)
  set(ALUGRID_FOUND TRUE)
  set(HAVE_ALUGRID TRUE)
endif(HAVE_ALUGRID_SERIAL_H)

if(NOT METIS_LIBRARIES AND ALUGRID_FOUND)
  message(WARNING "Metis library not found but needed for linking with ALUGrid.")

  set(ALUGRID_FOUND FALSE)
  set(HAVE_ALUGRID FALSE)
  return()
endif(NOT METIS_LIBRARIES AND ALUGRID_FOUND)

if(ALUGRID_FOUND)
  include(GridType)
  dune_define_gridtype(GRID_CONFIG_H_BOTTOM GRIDTYPE ALUGRID_CONFORM
    DUNETYPE "Dune::ALUGrid< dimgrid, dimworld, simplex, conforming >"
    HEADERS dune/grid/alugrid.hh dune/grid/io/file/dgfparser/dgfalu.hh)
  dune_define_gridtype(GRID_CONFIG_H_BOTTOM GRIDTYPE ALUGRID_CUBE
    DUNETYPE "Dune::ALUGrid< dimgrid, dimworld, cube, nonconforming >"
    HEADERS dune/grid/alugrid.hh dune/grid/io/file/dgfparser/dgfalu.hh)
  dune_define_gridtype(GRID_CONFIG_H_BOTTOM GRIDTYPE ALUGRID_SIMPLEX
    DUNETYPE "Dune::ALUGrid< dimgrid, dimworld, simplex, nonconforming >"
    HEADERS dune/grid/alugrid.hh dune/grid/io/file/dgfparser/dgfalu.hh)

endif(ALUGRID_FOUND)



