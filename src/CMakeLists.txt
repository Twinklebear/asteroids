add_executable(Asteroids main.cpp util.cpp model.cpp components/controllable.cpp
	systems/movement_system.cpp systems/asteroid_system.cpp systems/input_system.cpp
	level.cpp deleters.cpp texture_atlas.cpp texture_atlas_array.cpp gl_core_3_3.c)
target_link_libraries(Asteroids ${lfwatch_LIBRARY} ${SDL2_LIBRARY} ${OPENGL_LIBRARIES}
	${entityx_LIBRARY} ${tinyxml2_LIBRARY})

install(TARGETS Asteroids DESTINATION ${Asteroids_INSTALL_DIR})

