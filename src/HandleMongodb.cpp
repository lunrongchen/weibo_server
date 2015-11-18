
#include <iostream> 
#include "HandleMongodb.h"

using namespace std; 
using namespace mongo;

HandleMongodb::HandleMongodb()
{

}

HandleMongodb::~HandleMongodb()
{
	//释放堆内存
	if(client != NULL)
	{
		delete client;
		client = NULL;
	}
}

HandleMongodb* HandleMongodb::Create()
{
	HandleMongodb *pRet = new HandleMongodb();
	string link_ip = "localhost";
	string link_port = "27017";
	if(pRet && pRet->Init(link_ip, link_port))
	{
		return pRet;
	}
	else
	{
		cout<<"create HandleMongodb thread failed."<<"\n";
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}

HandleMongodb* HandleMongodb::Create(const string &link_ip, const string &link_port)
{
	HandleMongodb *pRet = new HandleMongodb();
	if(pRet && pRet->Init(link_ip, link_port))
	{
		return pRet;
	}
	else
	{
		cout<<"create HandleMongodb thread failed."<<"\n";
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}

bool HandleMongodb::Init(const string &link_ip, const string &link_port)
{
	bool bRet = false;
	do
	{
		//初始化client并连接数据库
		bool auto_connect = true;
		double so_timeout = 10;
		client = new DBClientConnection(auto_connect, 0, so_timeout);
		
		string link_addr = link_ip + ":" + link_port;
		string errmsg = "";
		if(!client->connect(link_addr, errmsg))
		{
			cout<<errmsg<<"\n";
			break;
		}
		else
		{
			cout<<"connect success"<<"\n";
		}
		
		bRet = true;
	}while(0);
	
	return bRet;
}

bool HandleMongodb::InsertToDB(const string &position, const BSONObj &content)
{
	bool bRet = false;
	
	do{
		try{
			client->insert(position, content, 0);
		}catch(DBException &e){
			cout<<e.what()<<"\n";
			break;
		}
		
		bRet = true;
	}while(0);
	
	return bRet;
}

auto_ptr<DBClientCursor> HandleMongodb::ReseachFromDB(const string &position, const Query &query, int nToReturn, int nToSkip)
{
	return client->query(position, query, nToReturn, nToSkip);
}

bool HandleMongodb::DeleteFromDB(const string &position, const Query &query, bool just_one)
{
	bool bRet = false;
	
	do{
		try{
			client->remove(position, query, just_one);
		}catch(DBException &e){
			cout<<e.what()<<"\n";
			break;
		}
		
		bRet = true;
	}while(0);
	
	return bRet;
}

bool HandleMongodb::UpdateToDB(const string &position, const Query &query, const BSONObj &obj, bool upser, bool multi)
{
	bool bRet = false;
	
	do{
		try{
			client->update(position, query, obj, upser, multi);
		}catch(DBException &e){
			cout<<e.what()<<"\n";
			break;
		}
		
		bRet = true;
	}while(0);
	
	return bRet;
}

