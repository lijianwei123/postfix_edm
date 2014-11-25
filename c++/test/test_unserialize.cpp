//g++ -g -I../base test_unserialize.cpp ../base/cJSON.c -o test_unserialize
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "cJSON.h"


using namespace boost;
using namespace boost::property_tree;
using namespace std;

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
		stringstream ss(json_str);
		ptree pt;

		try {
			read_json(ss, pt);
		} catch(ptree_error & e) {
			return -1;
		}

		try {
		   ip = pt.get<char *>("ip");
		   emailFromAddr = pt.get<char *>("emailFromAddr");
#ifdef DEBUG
		   printf("client_status_info unserialize ip:%s\n", ip);
#endif

		} catch (ptree_error & e) {
			return -1;
		}
		return 0;
	}

	client_status_info()
	{
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

	client_status_info status_info2;
	status_info2.unserialize(json_data);
	
	

	return 0;	
}
