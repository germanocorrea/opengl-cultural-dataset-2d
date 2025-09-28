cmake_minimum_required(VERSION 3.31)
project(CG_TD1)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)


set(INCLUDE_DIRS ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} include)
set(LIBRARIES ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

include_directories(${INCLUDE_DIRS})

file(GLOB INCLUDE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/lib/SOIL*.h)
add_executable(CG_TD1 main.cpp ${INCLUDE_FILES}
        src/Entity.cpp
        src/Entity.h
        src/EntityPosition.cpp
        src/EntityPosition.h
        src/Types.h
        src/CollisionChecker.cpp
        src/CollisionChecker.h)


target_link_libraries(CG_TD1 ${LIBRARIES} m)
