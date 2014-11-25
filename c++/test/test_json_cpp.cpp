//g++ -g -I../base test_json_cpp.cpp ../base/libbase.a ../base/json/libjson.a -o test_json_cpp
#include <iostream>
#include <stdlib.h>

using namespace std;

#include "cJSON.h"
#include "json/json.h"


typedef struct client_status_info
{
	char *ip;
	char *emailFromAddr;

	char *json_str;

	/**
	 * 序列化  注意释放返回值   free()
	 */
	char *serialize()
	{
		cJSON *root = NULL;
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root, "ip", ip);
		cJSON_AddStringToObject(root, "emailFromAddr", emailFromAddr);

		json_str = cJSON_Print(root);
		cJSON_Delete(root);

		return json_str;
	}

	/**
	 * 反序列化
	 */
	int unserialize(const char *json_str)
	{
		Json::Reader reader;
		Json::Value value;

		string json_string(json_str);
		string ip_string;
		string email_string;	
		if (reader.parse(json_string, value)) {
			ip_string = value["ip"].asString();
			email_string = value["emailFromAddr"].asString();
		}
		
		ip = const_cast<char *>(ip_string.c_str());
		emailFromAddr = const_cast<char *>(email_string.c_str());
			
		return 0;
	}

	client_status_info()
	{
		ip = NULL;
		emailFromAddr = NULL;
		json_str = NULL;
	}

	~client_status_info()
	{
		if (json_str)
			free(json_str);
	}

} client_status_info_t;

int main()
{
	client_status_info status_info;
        status_info.ip = const_cast<char *>("168.192.122.31");
        status_info.emailFromAddr = const_cast<char *>("9first.com");

        char *json_data = status_info.serialize();

	cout << "json data" << json_data << endl;

       	client_status_info status_info2;
       	status_info2.unserialize(json_data);
	cout << status_info2.ip << endl;	
	return 0;
}
