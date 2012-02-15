cmake_minimum_required(VERSION 2.0)
project(magicgrader)
add_executable(magicgrader main.c signal.c)
INSTALL(TARGETS magicgrader DESTINATION bin)
