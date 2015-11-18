////////////////////////////////////////////////////////////////////////////////
操作mongodb:
--------------------------------------------------------------------------------
安装pcre:
sudo apt-get install libpcre3
--------------------------------------------------------------------------------
安装scons:
版本:2.3.0
sudo apt-get install scons
--------------------------------------------------------------------------------
安装boost:
版本:boost_1_58_0
1.解压缩
2.进入目录，输入命令(可能有多余的，但都输不会出错的): 
		     sudo ./bootstrap.sh
		     sudo ./bjam
		     sudo ./b2
		     sudo ./b2 install
--------------------------------------------------------------------------------
安装mongodb:
版本:mongodb-src-r3.0.5
1.解压缩
2.在usr/local/里建立目录/mongo/(目录可以自己选，记住就行)
3.进入目录，输入命令:
		     sudo scons all
		     sudo scons --prefix=/usr/local/mongo　install
4.看一下/usr/local/mongo/目录下有没有一个bin目录，有的话就好了
--------------------------------------------------------------------------------
使用mongodb:
1.进入/usr/local/mongo/目录，建立/data/db/目录
2.进入/usr/local/mongo/bin/目录，运行命令:
		     sudo ./mongod --dbpath /usr/local/mongo/data/db
		     sudo ./mongo
--------------------------------------------------------------------------------
安装C++ Drive:
http://dl.mongodb.org/dl/cxx-driver/
版本:mongo-cxx-driver-v2.4
1.解压缩
2.进入目录，运行命令:
		    　sudo scons --extrapath=/home/bdccl/Downloads/boost_1_58_0　　(PS:此为boost解压的地址)
		　　　　　sudo scons install
		     sudo ldconfig /usr/local/lib
--------------------------------------------------------------------------------
测试:
#include <iostream> 
#include "mongo/client/dbclient.h" 
using namespace std; 
using namespace mongo; 

void run() { 
DBClientConnection c; 
c.connect("localhost:27017"); //add port,c.connect("localhost:27017") 
} 

int main(void) 
{ 
	try { 
		run(); 
		cout<<"connected ok"<<endl; 
	}catch(DBException& e){ 
		cout<<"caught"<<e.what()<<endl; 
	} 
	return 0; 
}

报错1：xtime.hpp的错误,进入文件，修改权限为可以写入(sudo chomd 777 xtime.hpp)，将里面所有的TIME_UTC改为TIME_UTC_;
报错2：链接库错误(undefined reference to 'mongo::DBClientConnection::numConnections')编译命令中加入
		　　　　　-pthread -lmongoclient -lboost_thread -lboost_filesystem -lboost_program_options -lboost_system -lssl -lcrypto
--------------------------------------------------------------------------------




///////////////////////////////////////////////////////////////////////////////
调用python:

头文件引用(Python.h路径):
#include "python2.7/Python.h"

需要链接的包:
../../../../../../usr/lib/python2.7/config-x86_64-linux-gnu/libpython2.7.so
../../../../../../lib/x86_64-linux-gnu/libpthread-2.19.so
../../../../../../lib/x86_64-linux-gnu/libutil-2.19.so
../../../../../../usr/lib/x86_64-linux-gnu/libz.so
../../../../../../lib/x86_64-linux-gnu/libdl-2.19.so

基本调用方法:

------------------------------------------------------------------------------------------------------------
#include <iostream>
#include "python2.7/Python.h"
using namespace std;

int main(int arg, char **argc)
{
    Py_Initialize();
    PyRun_SimpleString("import sys; sys.path.insert(0, '/home/bdccl/Documents/lb/WeiboProcess/WeiboProcess')");
    if(!Py_IsInitialized())
    {
        cout<<"Init fail"<<"\n";
        return 1;
    }
    cout<<"Init success"<<"\n";

    PyObject *pModule = NULL;
    PyObject *pFunc = NULL;
    pModule = PyImport_ImportModule("test");
    if(!pModule)
    {
        cout<<"get test fail"<<"\n";
        return 1;
    }
    cout<<"get test success"<<"\n";

    pFunc = PyObject_GetAttrString(pModule, "HelloWorld");
    cout<<"pFunc test success"<<"\n";
    PyEval_CallObject(pFunc, NULL);
    cout<<"pyEval test success"<<"\n";


    Py_Finalize();
    return 0;
}
-----------------------------------------------------------------------------------------------------------


