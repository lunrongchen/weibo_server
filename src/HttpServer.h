//This class is write to achieve restful api
//write by LB <libinghello@126.com>
//2015-8-18

#ifndef __WEIBOPROCESS__HTTPSERVER__H__
#define __WEIBOPROCESS__HTTPSERVER__H__

#include <string>
#include <map>
#include "HandleMongodb.h"
#include "mongoose.h"

using namespace std;

//存储每次搜索的参数
typedef struct SearchCacheData
{
	string session_id;
	string start_time;
	string end_time;
	string content_type;
}SData;

class HttpServer
{
public:
	
	HttpServer(void);
	~HttpServer(void);
	
	//获得一个HttpServer实例
	//port:监听的端口
	static HttpServer* Create(const string &port);
	
	//初始化,被Create函数自动调用
	bool Init(const string &port);
	
	//事件响应函数，由mg_create_server调用
	//conn:客户端请求的一些数据
	//ev:事件类别
	static int EventHandler(struct mg_connection *conn, enum mg_event ev);	//响应事件
	
	//restful api处理，详见api文档
	static SData HandlePostCall(struct mg_connection *conn);
	static SData HandleGetCall(struct mg_connection *conn);
	
	//生成客户端需要的数据
	static string ResponseCall(const int page_id, vector<pgContent> *search_data, const string &session_id); 
	
	//生成每次搜索的sessionId
	static string SetSessionId();
	
	//根据请求从数据库获得数据
	static vector<pgContent>* GetDataFromDB(const string &start_time,const string &end_time,const string &content_type, const int page_id);
	
	//解析uri
	//return 0:第一次search请求
	//return 1:具有session_id和page_id
	//return -1:错误的uri;
	static int HandleUri(const string &uri);
	
	//从uri中获得请求数据的页码
	static int GetPageId(const string &uri);
	
	//从uri中获取请求的参数
	static SData GetSessionCache(const string &uri);
	
	//判断字符串是否为规定格式
	static bool IsFormatTime(const string &time_string);

private:
	
	struct mg_server *server;	//服务端实例
	static int session_flag_add;	//一个累加的数字，保证session_id的唯一性
	static map<string, SData> session_cache;	//缓存搜索参数
	static pthread_mutex_t mutex; 	//互斥锁，保证session_flag_add和session_cache同一时间只被一个线程调用


};

#endif //__WEIBOPROCESS__HTTPSERVER__H__
