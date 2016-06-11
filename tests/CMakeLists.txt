project(tselements)

set(STATIC_STD_LIBS OFF)

set(SOURCES
	${PROJECT_SOURCE_DIR}/main.cpp

	${PROJECT_SOURCE_DIR}/state_machine.cpp
	${PROJECT_SOURCE_DIR}/texture_atlas.cpp	
	${PROJECT_SOURCE_DIR}/car_loading.cpp
    ${PROJECT_SOURCE_DIR}/track_loading.cpp
	${PROJECT_SOURCE_DIR}/collision_mask.cpp
	${PROJECT_SOURCE_DIR}/cup_infrastructure.cpp
)

add_executable(test_suite ${SOURCES})
target_link_libraries(test_suite libtselements)

enable_testing()
add_test(NAME RunTests COMMAND test_suite WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests)

add_custom_target(copy_test_files ALL
    COMMAND cmake -E copy_directory ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/assets)