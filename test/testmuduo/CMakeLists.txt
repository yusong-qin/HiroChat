#g++		-o        server                        -g         muduo_server.cpp xxx.cpp   -l/usr/include           -l/usr/lib         -lmuduo_net -lmuduo_base -lphread 
#g++编译		   1.可执行文件名(.exe *.a *.so)	2.编译选项          3.编译的源文件     4.头文件搜索路径         5.库文件搜索路径            6.要链接的外部库                                    


# 4.配置头文件的搜索路径
#include_directories()
# 5.配置库文件搜索路径
#link_directories()


# 3.设置需要编译的源文件列表
set(SRC_LIST muduo_server.cpp)
# 把.指定路径下的所有源文件名字放入变量名SRC_LIST里面
# aux_source_directory(. SRC_LIST)

#设置可执行文件最终存储的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)


# 1.生成可执行文件   3.编译的源文件
add_executable(server ${SRC_LIST}) # .exe
#add_library()  生成库文件

# 6.要链接的外部库
target_link_libraries(server muduo_net muduo_base pthread)
# 表示server这个目标程序，需要链接muduo_net muduo_base pthread 这三个库文件