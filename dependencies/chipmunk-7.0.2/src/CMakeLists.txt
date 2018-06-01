file(GLOB chipmunk_source_files "*.c")
file(GLOB chipmunk_public_header "${chipmunk_SOURCE_DIR}/include/chipmunk/*.h")

include_directories(${chipmunk_SOURCE_DIR}/include)

# Chipmunk2D 7.0.2
set(CHIPMUNK_VERSION_MAJOR 7)
set(CHIPMUNK_VERSION_MINOR 0)
set(CHIPMUNK_VERSION_PATCH 2)
set(CHIPMUNK_VERSION "${CHIPMUNK_VERSION_MAJOR}.${CHIPMUNK_VERSION_MINOR}.${CHIPMUNK_VERSION_PATCH}")
message("Configuring Chipmunk2D version ${CHIPMUNK_VERSION}")

if(BUILD_STATIC)
  add_library(chipmunk_static STATIC
    ${chipmunk_source_files}
  )
  # Tell MSVC to compile the code as C++.
  if(MSVC)
    set_source_files_properties(${chipmunk_source_files} PROPERTIES LANGUAGE CXX)
    set_target_properties(chipmunk_static PROPERTIES LINKER_LANGUAGE CXX)
  endif(MSVC)
  # Sets chipmunk_static to output "libchipmunk.a" not "libchipmunk_static.a"
  set_target_properties(chipmunk_static PROPERTIES OUTPUT_NAME chipmunk)
  if(INSTALL_STATIC)
    install(TARGETS chipmunk_static ARCHIVE DESTINATION ${LIB_INSTALL_DIR})
  endif(INSTALL_STATIC)
endif(BUILD_STATIC)

if(BUILD_SHARED OR INSTALL_STATIC)
  # FIXME: change to PUBLIC_HEADER to allow building frameworks
  install(FILES ${chipmunk_public_header} DESTINATION include/chipmunk)
  install(FILES ${chipmunk_constraint_header} DESTINATION include/chipmunk/constraints)
endif(BUILD_SHARED OR INSTALL_STATIC)
