cmake_minimum_required(VERSION 2.8.12)
project(tselements)

set(STATIC_STD_LIBS OFF)
set(TSELEMENTS_DEBUG_INFO ON)
add_definitions(-DTS_GL_DEBUG)

if(MSVC)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
    add_definitions(-DUNICODE -D_UNICODE)

	if(TSELEMENTS_DEBUG_INFO)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Z7")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF")	
    endif()	
endif()    
    
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

set(EXECUTABLE_NAME "tselements")

source_group(main REGULAR_EXPRESSION src/main\\..pp)
source_group(stdinc REGULAR_EXPRESSION src/stdinc\\..pp)
source_group(audio REGULAR_EXPRESSION src/audio/[^/]+)
source_group(client REGULAR_EXPRESSION src/client/[^/]+)
source_group(client\\states REGULAR_EXPRESSION src/client/states/[^/]+)
source_group(core REGULAR_EXPRESSION src/core/[^/]+)
source_group(components REGULAR_EXPRESSION src/components/[^/]+)
source_group(controls REGULAR_EXPRESSION src/controls/[^/]+)
source_group(cup REGULAR_EXPRESSION src/cup/[^/]+)
source_group(editor REGULAR_EXPRESSION src/editor/[^/])
source_group(fonts REGULAR_EXPRESSION src/fonts/[^/]+)
source_group(game REGULAR_EXPRESSION src/game/[^/]+)
source_group(graphics REGULAR_EXPRESSION src/graphics/[^/]+)
source_group(menu REGULAR_EXPRESSION src/menu/[^/]+)
source_group(messages REGULAR_EXPRESSION src/messages/[^/]+)
source_group(resources REGULAR_EXPRESSION src/resources/[^/]+)
source_group(scene REGULAR_EXPRESSION src/scene/[^/]+)
source_group(race REGULAR_EXPRESSION src/race/[^/]+)
source_group(server REGULAR_EXPRESSION src/server/[^/]+)
source_group(stage REGULAR_EXPRESSION src/stage/[^/]+)
source_group(user_interface REGULAR_EXPRESSION src/user_interface/[^/]+)
source_group(utility REGULAR_EXPRESSION src/utility/[^/]+)
source_group(world REGULAR_EXPRESSION src/world/[^/]+)

set(LIB_SOURCES
    src/audio/sound_playback_controller.cpp
    src/audio/sound_sample.cpp
	src/audio/sound_sample_loader.cpp

	src/client/states/local_cup_state.cpp
	src/client/states/local_action_state.cpp

	src/client/client_viewport_arrangement.cpp
	src/client/key_settings.cpp
	src/client/local_client.cpp
	src/client/local_message_dispatcher.cpp
	src/client/local_player_roster.cpp
	src/client/control_event_translator.cpp

	src/controls/control_center.cpp
	src/controls/controllable.cpp

	src/cup/cup.cpp
	src/cup/cup_synchronizer.cpp
	src/cup/cup_messages.cpp

	src/editor/tools/editor_path_tool.cpp
	src/editor/tools/editor_terrain_tool.cpp
	src/editor/editor_scene.cpp
	src/editor/height_map.cpp
	src/editor/render_scene_3d.cpp
	src/editor/texture_library_3d.cpp
	src/editor/terrain_scene_3d.cpp
	src/editor/track_3d.cpp
	src/editor/track_edit_state.cpp
	src/editor/track_editor_interface_state.cpp
	src/editor/track_editor_menu.cpp
	src/editor/track_loader_3d.cpp

	src/fonts/bitmap_font.cpp
	src/fonts/font_library.cpp
	src/fonts/font_renderer.cpp
	src/fonts/text_geometry.cpp

	src/game/loading_thread.cpp

	src/graphics/geometry.cpp
	src/graphics/geometry_renderer.cpp
	src/graphics/gl_context.cpp
	src/graphics/gl_check.cpp
	src/graphics/image.cpp
	src/graphics/render_window.cpp
	src/graphics/shader.cpp	
	src/graphics/texture.cpp
	src/graphics/vertex_buffer.cpp

	src/menu/update_input_state.cpp
	src/menu/main_menu.cpp
	src/menu/main_menu_state.cpp

	src/race/lap_tracker.cpp
	src/race/race.cpp

	src/resources/car_loader.cpp
	src/resources/car_store.cpp
	src/resources/collision_mask.cpp
	src/resources/include_path.cpp
	src/resources/resource_store.cpp
	src/resources/settings.cpp
	src/resources/pattern.cpp
	src/resources/pattern_builder.cpp
	src/resources/pattern_loader.cpp
	src/resources/terrain_library.cpp
	src/resources/texture_library.cpp
	src/resources/tile_library.cpp
	src/resources/track.cpp
	src/resources/track_loader.cpp

	src/scene/camera.cpp
	src/scene/dynamic_scene.cpp
	src/scene/dynamic_scene_generator.cpp
	src/scene/car_sound_controller.cpp
	src/scene/particle_generator.cpp
	src/scene/scene.cpp
	src/scene/scene_loader.cpp
	src/scene/scene_renderer.cpp
	src/scene/sound_effect_controller.cpp
	src/scene/track_scene.cpp
	src/scene/track_scene_generator.cpp
	src/scene/track_scene_generator_detail.cpp
	src/scene/viewport.cpp
	src/scene/viewport_arrangement.cpp
	
	src/server/server.cpp
	src/server/server_cup_essentials.cpp
	src/server/server_cup_controller.cpp	
	src/server/server_interaction_host.cpp
	src/server/server_message_dispatcher.cpp
	src/server/server_message_forwarder.cpp
	src/server/server_stage_essentials.cpp
	src/server/remote_client.cpp
	src/server/remote_client_map.cpp	

	src/stage/stage.cpp
	src/stage/stage_creation.cpp
	src/stage/stage_loader.cpp
	src/stage/stage_regulator.cpp

	src/user_interface/gui_context.cpp
	src/user_interface/gui_renderer.cpp
	src/user_interface/gui_geometry.cpp

	src/utility/logger.cpp
	src/utility/random.cpp
	src/utility/sha256.cpp
	src/utility/stream_utilities.cpp
	src/utility/string_utilities.cpp
	src/utility/texture_atlas.cpp

	src/world/car.cpp
	src/world/collisions.cpp
	src/world/control_point_manager.cpp
	src/world/entity.cpp
	src/world/handling_physics.cpp
	src/world/world.cpp
)

file(GLOB_RECURSE LIB_HEADERS *.hpp *.inl)

set(PRECOMPILED_SOURCE "src/stdinc.cpp")
if(MSVC)
    set(PRECOMPILED_BINARY "$(IntDir)/${PROJECT_NAME}.pch")
    
    SET_SOURCE_FILES_PROPERTIES(${LIB_SOURCES}
                                PROPERTIES COMPILE_FLAGS "/Yu\"stdinc.hpp\" /FI\"stdinc.hpp\" /Fp\"${PRECOMPILED_BINARY}\""
                                           OBJECT_DEPENDS "${PRECOMPILED_BINARY}")  
                                           
    SET_SOURCE_FILES_PROPERTIES(${PRECOMPILED_SOURCE}
                                PROPERTIES COMPILE_FLAGS "/Yc\"stdinc.hpp\" /Fp\"${PRECOMPILED_BINARY}\""
                                           OBJECT_OUTPUTS "${PRECOMPILED_BINARY}")    
endif()

add_library(libtselements STATIC ${LIB_SOURCES} ${PRECOMPILED_SOURCE} ${LIB_HEADERS})
add_executable(tselements src/main.cpp)
add_executable(editor src/editor_main.cpp)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_USE_MULTITHREADED ON)

set(ZLIB_PATH "dependencies/zlib-1.2.8")
set(PNG_PATH "dependencies/libpng-1.2.51")

add_subdirectory("${PROJECT_SOURCE_DIR}/${ZLIB_PATH}")
add_subdirectory("${PROJECT_SOURCE_DIR}/${PNG_PATH}")

set(ZLIB_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/${ZLIB_PATH}" "${PROJECT_BINARY_DIR}/${ZLIB_PATH}")
set(PNG_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/${PNG_PATH}" "${PROJECT_BINARY_DIR}/${PNG_PATH}")

set(ZLIB_LIBRARY zlib)
set(PNG_LIBRARY png12)

find_package(Boost REQUIRED COMPONENTS system filesystem locale)
find_package(SFML 2 REQUIRED system window graphics network audio)
find_package(GLEW REQUIRED)
add_definitions("-DHAS_BOOST")

include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/dependencies/catch/single_include")
include_directories("${PROJECT_SOURCE_DIR}/dependencies/glm")

include_directories(${Boost_INCLUDE_DIR})
include_directories(${SFML_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${ZLIB_INCLUDE_DIR})
include_directories(${PNG_INCLUDE_DIR})

if(MSVC)
  target_link_libraries(libtselements "winmm.lib")
  target_link_libraries(libtselements "opengl32.lib")
endif()

enable_testing()

target_link_libraries(libtselements ${Boost_LIBRARIES})
target_link_libraries(libtselements ${SFML_LIBRARIES})
target_link_libraries(libtselements ${GLEW_LIBRARIES})

target_link_libraries(libtselements ${ZLIB_LIBRARY})
target_link_libraries(libtselements ${PNG_LIBRARY})

target_link_libraries(tselements libtselements)
target_link_libraries(editor libtselements)

add_subdirectory("${PROJECT_SOURCE_DIR}/tests")
