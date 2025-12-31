# mpRPC
基于muduo和protobuf实现RPC框架，用zookeeper实现自动服务发现，客户端重写channel的callmethod，服务端重写服务函数（LOGIN），服务端用表存储注册服务信息，提供初始化，注册服务，运行接口
