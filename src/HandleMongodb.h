//This class is write to handle mongodb
//write by LB <libinghello@126.com>
//2015-8-17

#ifndef __WEIBOPROCESS__HANDLEMONGOED__H__
#define __WEIBOPROCESS__HANDLEMONGOED__H__

#include <string>
#include <vector>
#include "mongo/client/dbclient.h"


using namespace std;
using namespace mongo;

//存储每条数据内容
typedef struct ResponseUniDataFormatPageContent
{
	string content_type;
	string time;
	string title;
	string label;
	string abstract;
	string content;
	string location;
	string url;
	string hot;
}pgContent;

class HandleMongodb
{
public:
	HandleMongodb(void);
	~HandleMongodb(void);
	
	//获得一个HangleMongodb实例
	static HandleMongodb* Create(void);
	static HandleMongodb* Create(const string &link_ip, const string &link_port);
	
	//初始化,被Create函数自动调用
	//link_ip:数据库ip地址
	//link_port:数据库监听端口
	bool Init(const string &link_ip, const string &link_port);
	
	//插入数据
	//postion:操作数据库的位置，"db_name.collection_name",如"weibo.predata"
	//content:插入的对象，例子：
	//－－－－－－－－－－－－－－HandleMongodb *test = HandleMongodb::Create();
	//－－－－－－－－－－－－－－BSONObjBuilder b_content;
	//－－－－－－－－－－－－－－b_content.append("title", "抗议");
	//－－－－－－－－－－－－－－b_content.append("location", "湖北省");
	//－－－－－－－－－－－－－－BSONObj content = b_content.obj();
	//－－－－－－－－－－－－－－string position = "weibo.test";
	//－－－－－－－－－－－－－－test->InsertToDB(position, content);
	bool InsertToDB(const string &position, const BSONObj &content);
	
	//取出数据
	//postion:同上
	//query:查询条件对象，例：Query condition = QUERY("time"<<GTE<<start_time<<LT<<end_time);
	//nToReturn:需要查询的条数
	//nToSkip:跳过的条数
	auto_ptr<DBClientCursor> ReseachFromDB(const string &position, const Query &query, int nToReturn, int nToSkip);
	
	//删除数据
	//position,query:同上
	//just_one:是否只删除第一个匹配的数据
	bool DeleteFromDB(const string &position, const Query &query, bool just_one);
	
	//修改数据
	//position,query,content:同上
	//upser:若不存在是否插入
	//multi:是否为符合文档
	bool UpdateToDB(const string &position, const Query &query, const BSONObj &content, bool upser, bool multi);
	
	
private:
	
	DBClientConnection *client;	//mongodb连接实例
};

#endif //__WEIBOPROCESS__HANDLEMONGOED__H__
