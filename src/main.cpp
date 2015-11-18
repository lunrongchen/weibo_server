
#include <iostream>
#include <string>
#include "HttpServer.h"

using namespace std;

int main(int arg, char *argv[])
{
	
	string port = "8080";
	HttpServer *_httpServer = HttpServer::Create(port);
	
	return 0;
}




