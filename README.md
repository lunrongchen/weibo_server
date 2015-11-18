1.配置好环境，以及运行mongdb，详细见《INSTALL.md》

2.代码结构分析：

【１】HandleMongodb.h,HandleMongodb.cpp封装了对mongodb的操作

【２】HttpServer.h,HttpServer.cpp封装了服务器操作，处理客户端的请求

【３】main.cpp运行程序

【４】详细内容见代码注释

【５】客户端数据请求格式等问题见《WebAPI文档》

3.可能遇到的问题：

【１】输入格式正确但请求失败：确认是否在英文输入法下输入的；由于程序设置了返回空内容之后便销毁该次session_id，确认是否接收过空数据

【２】程序中的互斥锁并未经过多线程并发测试，若在并发环境下出错请自行调试修改

【３】请注意原始数据库与predata数据库时间格式的差异！！！

