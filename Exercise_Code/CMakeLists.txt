# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

set(EXTRA_COMPONENT_DIRS 
        ${CMAKE_SOURCE_DIR}/components/BSP
        ${CMAKE_SOURCE_DIR}/components/Middlewares
)	# 告诉编译器额外组件搜索路径
add_compile_options(-fdiagnostics-color=always)		# 开启编译日志彩色输出

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(sample_demo)
