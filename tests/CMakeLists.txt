function(cgride_executor_add_test name source)
  add_executable(${name}
    ${source}
  )

  target_link_libraries(${name}
    PRIVATE
      cgride::executor
  )

  target_compile_features(${name}
    PRIVATE
      cxx_std_23
  )

  set_target_properties(${name}
    PROPERTIES
      CXX_EXTENSIONS OFF
  )

  add_test(
    NAME ${name}
    COMMAND ${name}
  )
endfunction()

cgride_executor_add_test(cgride_executor_process_result_test process_result_test.cpp)
cgride_executor_add_test(cgride_executor_task_result_test task_result_test.cpp)
cgride_executor_add_test(cgride_executor_execution_options_test execution_options_test.cpp)
cgride_executor_add_test(cgride_executor_process_test process_test.cpp)
cgride_executor_add_test(cgride_executor_executor_test executor_test.cpp)
