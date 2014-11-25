//g++ -g -I../base test_mysql.cpp -L/usr/lib/mysql -lmysqlclient ../base/libbase.a -o  test_mysql
#include <iostream>
#include <assert.h>
#include "mysql.h"

using namespace std;

int main()
{
	mysql_connect_info_t  mysql_connect_info;
	MYSQL conn;
	int retCode = -1;

	//测试db连接
	mysql_connection_info_init(&mysql_connect_info);
	mysql_connect_info.host = const_cast<char *>("168.192.122.30");
	mysql_connect_info.user = const_cast<char *>("efoncheng");
	mysql_connect_info.pwd = const_cast<char *>("efoncheng");


	retCode = mysql_user_connect(&conn, &mysql_connect_info);
	cout << "retCode" << retCode << endl;

	assert(retCode == 0);


}
