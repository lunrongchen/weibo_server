
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>

#include "HttpServer.h"

using namespace std;

int HttpServer::session_flag_add = 4096;
map<string, SData> HttpServer::session_cache;
pthread_mutex_t HttpServer::mutex;

HttpServer::HttpServer()
{

}

HttpServer::~HttpServer()
{
	if(server != NULL)
	{
		mg_destroy_server(&server);
	}
}

HttpServer* HttpServer::Create(const string &port)
{
	HttpServer *pRet = new HttpServer();
	if(pRet && pRet->Init(port))
	{
		return pRet;
	}
	else
	{
		cout<<"create HttpServer thread failed."<<"\n";
		delete pRet;
		pRet = NULL;
		return NULL;
	}	
}

bool HttpServer::Init(const string &port)
{
	bool bRet = false;
	do
	{
		//初始化互斥锁
		pthread_mutex_init (&mutex,NULL);
		
		//初始化服务端实例并运行
		server = mg_create_server(NULL, EventHandler);
		if(server == NULL)
		{
			cout<<"mg_create_server() failed, maybe memory is not enough."<<"\n";
			break;
		}
		mg_set_option(server, "listening_port", "8080");
  		printf("Starting on port %s\n", mg_get_option(server, "listening_port"));
  		for (;;) {
    		mg_poll_server(server, 1000);
  		}
		////////////////////
		
		bRet = true;
		
	}while(0);
	
	return bRet;
}

int HttpServer::EventHandler(struct mg_connection *conn, enum mg_event ev)
{
	switch (ev) {
    case MG_AUTH: 
    	return MG_TRUE;
    case MG_REQUEST:
    {
    	string f_s_uri = conn->uri;
    	int status_code = HandleUri(f_s_uri);
      	if (status_code == 0)
      	{
      		SData flag = HandlePostCall(conn);
      		if(flag.content_type == "wrong")
      		{
      			return MG_FALSE;
      		}
      		else
      		{
      			//从数据库搜索数据并发送
      			vector<pgContent> *search_data = GetDataFromDB(flag.start_time, flag.end_time, flag.content_type, 1);
      			mg_printf_data(conn, "%s", ResponseCall(1, search_data, flag.session_id).c_str());
      			return MG_TRUE;
      		}
      	}
      	else if(status_code == 1)
      	{
      		SData flag = HandleGetCall(conn);
      		if(flag.content_type == "wrong")
      		{
      			return MG_FALSE;
      		}
      		else
      		{
      			//从数据库搜索数据并发送
      			vector<pgContent> *search_data = GetDataFromDB(flag.start_time, flag.end_time, flag.content_type, GetPageId(f_s_uri));
      			mg_printf_data(conn, "%s", ResponseCall(GetPageId(f_s_uri), search_data, flag.session_id).c_str());
      			return MG_TRUE;
      		}
      	}
      	else
      	{
      		return MG_FALSE;
      	}
	}
    default:
    	return MG_FALSE;
  }
}

SData HttpServer::HandlePostCall(struct mg_connection *conn)
{
	//若出错则返回错误标识数据
	SData wrong_flag;
	wrong_flag.content_type = "wrong";
	//////////////////////
	
	//获得参数:start_time，end_time，content_type
	char c_start_time[100];
	char c_end_time[100];
	char c_content_type[100];
	mg_get_var(conn, "start_time", c_start_time, sizeof(c_start_time));
	mg_get_var(conn, "end_time", c_end_time, sizeof(c_end_time));
	mg_get_var(conn, "content_type", c_content_type, sizeof(c_content_type));
	//////////////////////////////////////////
	
	if(c_start_time == NULL || c_end_time == NULL || c_content_type == NULL)
	{
		return wrong_flag;
	}
	
	string start_time = c_start_time;
	string end_time = c_end_time;
	string content_type = c_content_type;
	if(start_time == "" || end_time == "" || content_type == "")
	{
		return wrong_flag;
	}
	
	
	//判断数据格式是否正确
	if((!IsFormatTime(start_time)) || (!IsFormatTime(end_time)))
	{
		return wrong_flag;
	}
	if(!(content_type == "weibo" || content_type == "forum" 
		|| content_type == "news" || content_type == "others"))
	{
		return wrong_flag;
	}
	/////////////////////
	
	//返回正确数据，并将数据存入session_cache
	pthread_mutex_lock (&mutex);
	
	string session_flag = SetSessionId();
	SData flag;
	flag.session_id = session_flag;
	flag.start_time = start_time;
	flag.end_time = end_time;
	flag.content_type = content_type;
	//－－－－－－－－－－插入操作
	session_cache.insert(map<string, SData>::value_type(session_flag, flag));
	
	pthread_mutex_unlock(&mutex);
	
	return flag;
}

SData HttpServer::HandleGetCall(struct mg_connection *conn)
{
	string f_s_uri = conn->uri;
	SData flag = GetSessionCache(f_s_uri);
	return flag;
}

string HttpServer::ResponseCall(const int page_id, vector<pgContent> *search_data, const string &session_id)
{	
	int content_count = search_data->size();
	int max_page = 10;
	
	
	//判断是否查找到数据
	bool content_flag = false;
	if(search_data->size() > 0 && page_id > 0)
	{
		content_flag = true;
	}
	
	//若已没有数据则删除session_cache中的这条记录
	if(search_data->size() <= 0)
	{
		pthread_mutex_lock (&mutex);
		
		session_cache.erase(session_id);
		
		pthread_mutex_unlock(&mutex);
	}
	
	
	//将int转换为string
	string s_count;
	string s_max_page;
	string s_page_id;
	string s_content_count;
	
	stringstream ss1,ss2,ss3,ss4;
	ss1 << search_data->size();
	ss1 >> s_count;
	ss2 << max_page;
	ss2 >> s_max_page;
	ss3 << page_id;
	ss3 >> s_page_id;
	ss4 << content_count;
	ss4 >> s_content_count;
	
	//整理要发送数据
	string response_data = "";
	if(content_flag)
	{
		response_data = response_data + "{\"status\":" + "\"success\",";
	}
	else
	{
		response_data = response_data + "{\"status\":" + "\"none\",";
	}
	
	response_data = response_data + "\"session_id\":\"" + session_id + "\",";
	response_data = response_data + "\"count\":\"" + s_count + "\",";
	response_data = response_data + "\"max_page\":\"" + s_max_page + "\",";
	response_data = response_data + "\"page_id\":\"" + s_page_id + "\",";
	response_data = response_data + "\"content_count\":\"" + s_content_count + "\",";
	response_data = response_data + "\"page_content\":[";
	
	if(content_flag)
	{
		for(int i=0; i<content_count; i++)
		{
			response_data += "{";
			response_data = response_data + "\"content_type\":" + "\"" + (*search_data)[i].content_type + "\",";
			response_data = response_data + "\"time\":" + "\"" + (*search_data)[i].time + "\",";
			response_data = response_data + "\"title\":" + "\"" + (*search_data)[i].title + "\",";
			response_data = response_data + "\"label\":" + "\"" + (*search_data)[i].label + "\",";
			response_data = response_data + "\"abstract\":" + "\"" + (*search_data)[i].abstract + "\",";
			response_data = response_data + "\"content\":" + "\"" + (*search_data)[i].content + "\",";
			response_data = response_data + "\"location\":" + "\"" + (*search_data)[i].location + "\",";
			response_data = response_data + "\"url\":" + "\"" + (*search_data)[i].url + "\",";
			response_data = response_data + "\"hot\":" + "\"" + (*search_data)[i].hot + "\"";
			response_data += "}";
			if(i + 1 < content_count)
			{
				response_data += ",";
			}
		}
	}
	
	response_data += "]}";
	
	//到此处search_data完成任务，释放内存
	delete search_data;
	search_data = NULL;
	
	return response_data;
}

string HttpServer::SetSessionId()
{
	//生成session_id
	//由进程id,线程id,当前时间(秒),session_flag_add组成
	int thread_id = pthread_self();
	int process_id = getpid();
	int t = time(NULL);
	char session_flag_copy[100];
	
	session_flag_add ++;
	if(session_flag_add < 4096 || session_flag_add > 65535)
	{
		session_flag_add = 4096;
	}
	sprintf(session_flag_copy, "%x%x%x%4x", process_id, thread_id, t, session_flag_add);
	
	return session_flag_copy;
}

vector<pgContent>* HttpServer::GetDataFromDB(const string &start_time, const string &end_time, const string &content_type, const int page_id)
{
	//从数据库里获得数据
	HandleMongodb *connect_db = HandleMongodb::Create();
	string position = content_type + ".predata";
	Query condition = QUERY("time"<<GTE<<start_time<<LTE<<end_time);
	
	auto_ptr<DBClientCursor> p_search_data = connect_db->ReseachFromDB(position, condition, 10, (page_id-1)*10);
	
	//将数据存入search_data中
	vector<pgContent> *search_data = new vector<pgContent>();
	pgContent flag;
	while(p_search_data->more())
	{	
		BSONObj p = p_search_data->next();
		flag.content_type = p["content_type"].String();
		flag.time = p["time"].String();
		flag.title = p["title"].String();
		flag.label = p["label"].String();
		flag.abstract = p["abstract"].String();
		flag.content = p["content"].String();
		flag.location = p["location"].String();
		flag.url = p["url"].String();
		flag.hot = p["hot"].String();
		search_data->push_back(flag);
	}
	
	return search_data;
}

int HttpServer::HandleUri(const string &uri)
{
	if(uri.size() >= 7 && uri.compare(0,7,"/search") == 0)
	{
		if(uri.size() == 7)
		{
			return 0;
		}
		else if(uri.size() > 33 && uri[7] == '/' && uri[32] == '/')
		{	
			for(int i=33; i+1<=uri.size(); i++)
			{
				if(!(uri[i] >= '0' && uri[i] <= '9'))
				{
					return -1;
				}
			}
			return 1; 
		}
	}
	else
	{
		return -1;
	}
	return -1;
}

SData HttpServer::GetSessionCache(const string &uri)
{
	//从uri中解析出session_id,从缓存中找到对应的搜索信息
	string session = uri.substr(8,24);
	map<string, SData>::iterator iter;
	
	pthread_mutex_lock (&mutex);
	
	iter = session_cache.find(session);
	
	pthread_mutex_unlock(&mutex);
	
	//没找到则返回错误标示
	SData flag;
	flag.content_type = "wrong";
	if(iter == session_cache.end())
	{
		return flag;
	}
	
	return (SData)(iter->second);
}

int HttpServer::GetPageId(const string &uri)
{
	//从uri中解析出page_id
	string page_str = "";
	int page_num;
	for(int i=33; i+1<=uri.size(); i++)
	{
		page_str += uri[i];			
	}
	stringstream ss;
	ss << page_str;
	ss >> page_num;
	return page_num; 
}

bool HttpServer::IsFormatTime(const string &time_string)
{
	int year,month,day,hour,minute,second;
	string s_year="",s_month="",s_day="",s_hour="",s_minute="",s_second="";
	
	if(time_string.size() <= 0)
	{
		return false;
	}
	
	int a,b,c,d,e,f;
	
	//处理年份
	for( a=0; a+1<=time_string.size() && time_string[a] != '-'; ++a)
	{
		if(!(time_string[a] >= '0' && time_string[a] <= '9'))
		{
			return false;
		}
		s_year = s_year + time_string[a];
	}
	if(s_year == "")
	{
		return false;
	}
	//处理月份
	for( b=a+1; b+1<=time_string.size() && time_string[b] != '-'; ++b)
	{
		if(!(time_string[b] >= '0' && time_string[b] <= '9'))
		{
			return false;
		}
		s_month = s_month + time_string[b];
	}
	if(s_month == "")
	{
		return false;
	}
	//处理日
	for( c=b+1; c+1<=time_string.size() && time_string[c] != '-'; ++c)
	{
		if(!(time_string[c] >= '0' && time_string[c] <= '9'))
		{
			return false;
		}
		s_day = s_day + time_string[c];
	}
	if(s_day == "")
	{
		return false;
	}
	//处理时
	for( d=c+1; d+1<=time_string.size() && time_string[d] != ':'; ++d)
	{
		if(!(time_string[d] >= '0' && time_string[d] <= '9'))
		{
			return false;
		}
		s_hour = s_hour + time_string[d];
	}
	if(s_hour == "")
	{
		return false;
	}
	//处理分
	for( e=d+1; e+1<=time_string.size() && time_string[e] != ':'; ++e)
	{
		if(!(time_string[e] >= '0' && time_string[e] <= '9'))
		{
			return false;
		}
		s_minute = s_minute + time_string[e];
	}
	if(s_minute == "")
	{
		return false;
	}
	//处理秒
	for( f=e+1; f+1<=time_string.size(); ++f)
	{
		if(!(time_string[f] >= '0' && time_string[f] <= '9'))
		{
			return false;
		}
		s_second = s_second + time_string[f];
	}
	if(s_second == "")
	{
		return false;
	}
	
	stringstream ss1,ss2,ss3,ss4,ss5,ss6;
	ss1 << s_year;
	ss1 >> year;
	ss2 << s_month;
	ss2 >> month;
	ss3 << s_day;
	ss3 >> day;
	ss4 << s_hour;
	ss4 >> hour;
	ss5 << s_minute;
	ss5 >> minute;
	ss6 << s_second;
	ss6 >> second;

	
	if((2000 < year && year < 9999)
		&& (0 < month && month < 13)
		&& (0 < day && day < 32)
		&& (-1 < hour && hour < 25)
		&& (-1 < minute && minute < 61)
		&& (-1 < second && second < 61))
	{
		return true;
	}
	
	return false;
}