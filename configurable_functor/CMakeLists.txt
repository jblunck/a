cmake_minimum_required(VERSION 2.6)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

add_executable(functor_bind functor_bind.cpp)
add_executable(functor_funcpointer functor_funcpointer.cpp)
add_executable(functor_virt functor_virt.cpp functor_virtual.cpp)
add_executable(functor_template functor_template.cpp)
