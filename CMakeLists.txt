cmake_minimum_required(VERSION 3.22)

project(
  cgride-executor
  VERSION 0.1.1
  DESCRIPTION "Task graph execution, process running and structured execution results for Cgride."
  HOMEPAGE_URL "https://github.com/cgride/executor"
  LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(CGRIDE_EXECUTOR_BUILD_TESTS "Build Cgride executor tests" ON)
option(CGRIDE_EXECUTOR_INSTALL "Install Cgride executor" ON)

if(NOT TARGET cgride::core)
  find_package(cgride-core CONFIG REQUIRED)
endif()

if(NOT TARGET cgride::graph)
  find_package(cgride-graph CONFIG REQUIRED)
endif()

add_library(cgride_executor
  src/process_result.cpp
  src/task_result.cpp
  src/execution_options.cpp
  src/process.cpp
  src/executor.cpp
)

add_library(cgride::executor ALIAS cgride_executor)

target_compile_features(cgride_executor
  PUBLIC
    cxx_std_23
)

target_include_directories(cgride_executor
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(cgride_executor
  PUBLIC
    cgride::core
    cgride::graph
)

set_target_properties(cgride_executor
  PROPERTIES
    OUTPUT_NAME cgride_executor
    EXPORT_NAME executor
    CXX_EXTENSIONS OFF
)

if(CGRIDE_EXECUTOR_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(CGRIDE_EXECUTOR_INSTALL)
  install(
    TARGETS cgride_executor
    EXPORT cgride-executor-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

  install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  install(
    EXPORT cgride-executor-targets
    NAMESPACE cgride::
    FILE cgride-executor-targets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cgride-executor
  )

  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cgride-executor-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cgride-executor-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cgride-executor
  )

  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/cgride-executor-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )

  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/cgride-executor-config.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/cgride-executor-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cgride-executor
  )
endif()
