add_executable(Asteroids main.cpp renderbatch.cpp util.cpp model.cpp
	systems/movement_system.cpp systems/asteroid_system.cpp systems/input_system.cpp
	level.cpp deleters.cpp gl_core_3_3.c)
target_link_libraries(Asteroids ${SDL2_LIBRARY} ${OPENGL_LIBRARIES} ${entityx_LIBRARY})
install(TARGETS Asteroids DESTINATION ${Asteroids_INSTALL_DIR})

