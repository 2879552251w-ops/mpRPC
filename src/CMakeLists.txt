#aux_source_directory(. SRC_LIST)
file(GLOB SRC_LIST CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
add_library(mprpc  ${SRC_LIST})
target_link_libraries(mprpc muduo_net muduo_base  pthread zookeeper_mt)