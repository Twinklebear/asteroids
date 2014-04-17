add_executable(Asteroids main.cpp gl_core_3_3.c)
target_link_libraries(Asteroids ${SDL2_LIBRARY} ${OPENGL_LIBRARIES} ${entityx_LIBRARY})
install(TARGETS Asteroids DESTINATION ${Asteroids_INSTALL_DIR})

