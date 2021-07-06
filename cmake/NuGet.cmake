function(install_nuget config_file)

  find_program(NUGET nuget REQUIRED)
  configure_file(${config_file} ${CMAKE_BINARY_DIR}/packages.config)
  execute_process(COMMAND 
    ${NUGET} restore packages.config -SolutionDirectory ${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )

endfunction()