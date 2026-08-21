cmake_minimum_required(VERSION 3.16) (CMake 4.x exige ≥3.5; 3.16 es seguro)
project(Gawain ...) 
Estándar C++17 obligatorio
set(CMAKE_AUTOMOC ON) — necesario porque la ventana usará la macro Q_OBJECT (Qt genera código extra automáticamente al compilar)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
add_executable con los 5 .cpp (los 4 actuales + main_window.cpp)
target_link_libraries con Qt6::Widgets y sodium