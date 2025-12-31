file(GLOB_RECURSE SRC CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/example/caller/*.cc)

# 如果想排除某些文件（比如 exclude_me.cc），可以这样操作：
#list(REMOVE_ITEM SRC_LIST "${CMAKE_SOURCE_DIR}/src/exclude_me.cc")
add_executable(caller ${SRC})

target_link_libraries(caller mprpc protobuf)