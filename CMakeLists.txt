# $Id: CMakeLists.txt $

#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.6 FATAL_ERROR)
project(AND_DAQ)

#----------------------------------------------------------------------------
# Setup include and lib directory for this project
#
#include_directories(${PROJECT_SOURCE_DIR}/include)
#include_directories(${PROJECT_SOURCE_DIR}/CAEN/include)
#link_directories(${PROJECT_SOURCE_DIR}/CAEN/lib)
#link_libraries(CAENVMELib.lib CAENComm.lib CAEN_PLULib.lib ftd2xx.lib)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.h)

set(ANDDAQ_SCRIPTS
  _readme.txt
  README.md
  config.txt
  pgnuplot.exe
  wgnuplot.exe
  )

foreach(_script ${ANDDAQ_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()

file(COPY ${PROJECT_SOURCE_DIR}/include DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${PROJECT_SOURCE_DIR}/CAEN DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${PROJECT_SOURCE_DIR}/output DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${PROJECT_SOURCE_DIR}/histo DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${PROJECT_SOURCE_DIR}/gnuplot DESTINATION ${PROJECT_BINARY_DIR})

set(CAEN_INCLUDE_DIR ${PROJECT_BINARY_DIR}/CAEN/include)
set(CAEN_LIB_DIR ${PROJECT_BINARY_DIR}/CAEN/lib)

file(COPY ${CAEN_LIB_DIR}/CAENVMELib.dll DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${CAEN_LIB_DIR}/CAENComm.dll DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${CAEN_LIB_DIR}/CAEN_PLULib.dll DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${CAEN_LIB_DIR}/ftd2xx.dll DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${CAEN_LIB_DIR}/PlxApi.dll DESTINATION ${PROJECT_BINARY_DIR})


#----------------------------------------------------------------------------
# Add the executable
#
add_executable(AND_DAQ AND_DAQ.c ${sources} ${headers})

target_include_directories(AND_DAQ PUBLIC ${PROJECT_BINARY_DIR}/include)

target_include_directories(AND_DAQ PUBLIC ${CAEN_INCLUDE_DIR})
target_link_directories(AND_DAQ PUBLIC ${CAEN_LIB_DIR})

target_link_libraries(AND_DAQ ${CAEN_LIB_DIR}/CAENVMELib.lib)
target_link_libraries(AND_DAQ ${CAEN_LIB_DIR}/CAENComm.lib)
target_link_libraries(AND_DAQ ${CAEN_LIB_DIR}/CAEN_PLULib.lib)
target_link_libraries(AND_DAQ ${CAEN_LIB_DIR}/ftd2xx.lib)