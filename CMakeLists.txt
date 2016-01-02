cmake_minimum_required(VERSION 3.0)

project(Wallpaper-gen)
 
find_package(Qt5Widgets REQUIRED)
find_package(glm REQUIRED)
 
set(CMAKE_AUTOMOC ON)
 
 
include_directories(${GLM_INCLUDE_DIRS})

add_executable(wallpaper-gen GLWidget.cpp Window.cpp main.cpp tiny_obj_loader.cc lodepng.cpp)
 
 
target_link_libraries(wallpaper-gen Qt5::Widgets)
 
target_compile_features(wallpaper-gen PRIVATE
	cxx_constexpr
)
 
