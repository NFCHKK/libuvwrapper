cmake_minimum_required(VERSION 2.8)
AUX_SOURCE_DIRECTORY(. DIR_SRCS)

SET (TRANS_SRCS ./libuvcpp/accept.cpp 
				./libuvcpp/connect.cpp
				./libuvcpp/loop.cpp
				./libuvcpp/tcpClient.cpp
				./libuvcpp/tcpServer.cpp
				./libuvcpp/timer.cpp
				./sample.cpp
				./util/safebuffer.cpp
	)
	

#添加头文件搜索路径
INCLUDE_DIRECTORIES(./include)
INCLUDE_DIRECTORIES(./util)
INCLUDE_DIRECTORIES(./libuvcpp)


#添加库文件搜索路径
LINK_DIRECTORIES(./libso)
#自动检测并添加目标文件夹
#add_custom_command(TARGET tbpsdk
#	PRE_BUILD 
#	if(NOT EXISTS bin)
#		COMMAND mkdir -m 777 bin
#	endif(NOT EXISTS bin)
#)
#自动检测并添加目标文件夹
if(NOT EXISTS bin)
	execute_process(COMMAND mkdir -m 777 bin)
endif(NOT EXISTS bin)

# 配置编译参数
SET(CMAKE_CXX_COMPILER, "/usr/local/bin/c++")

SET(CMAKE_CXX_FLAGS "-g -std=c++11 -fpermissive -fPIC ${CMAKE_CXX_FLAGS}")

SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

ADD_EXECUTABLE(TcpServer  ${DIR_SRCS} ${TRANS_SRCS})

target_link_libraries(TcpServer rt pthread dl uv)