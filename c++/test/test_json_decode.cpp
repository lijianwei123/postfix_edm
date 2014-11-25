//http://blog.csdn.net/hzyong_c/article/details/7163589
//http://blog.csdn.net/dotphoenix/article/details/27081377

//g++ -g test_json_decode.cpp  -Wno-deprecated -I../base ../base/libbase.a  -o test_json_decode

/*
 * test_json_decode.cpp
 *
 *  Created on: 2014-11-18
 *  Author: Administrator
 */
#include <iostream>
#include "util.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace boost;
using namespace boost::property_tree;

typedef struct server_status_info {
	uint16_t status;    //服务器的状态   start  stop

	uint16_t delay;
	uint16_t multi;

	uint64_t totalNum;
	uint16_t online_client_num;


	char *json_str;

	/**
	 * 序列化  注意释放返回值   free()
	 */
	char *serialize()
	{
		cJSON *root = NULL;
		root = cJSON_CreateObject();
		cJSON_AddNumberToObject(root, "status", status);
		cJSON_AddNumberToObject(root, "delay", delay);
		cJSON_AddNumberToObject(root, "multi", multi);
		cJSON_AddNumberToObject(root, "totalNum", totalNum);
		cJSON_AddNumberToObject(root, "online_client_num", online_client_num);
		json_str = cJSON_Print(root);
		cJSON_Delete(root);

		return json_str;
	}

	/**
	 * 反序列化
	 */
	int unserialize(char *json_str)
	{
		stringstream ss(json_str);
		ptree pt;

		try {
		    read_json(ss, pt);
		} catch(ptree_error & e) {
		    return -1;
		}


		try {
		    int code = pt.get<int>("code");   // 得到"code"的value
		    ptree image_array = pt.get_child("images");  // get_child得到数组对象

		    // 遍历数组
		    BOOST_FOREACH(ptree::value_type &v, image_array)
		    {
		      stringstream s;
		      ptree p = v.second;
		      string to = p.get<string>("to");
		
		    }
		} catch (ptree_error & e) {
		    return -1;
		}
		return 0;
	}


	server_status_info()
	{
		json_str = NULL;
	}

	~server_status_info()
	{
		free(json_str);
	}

} server_status_info_t;

int main()
{
	server_status_info  *status_info = new server_status_info();
	status_info->status = 1;
	status_info->delay = 2;
	status_info->multi = 3;
	status_info->totalNum = 10;
	status_info->online_client_num = 20;

	cout << "status info  serialize" << status_info->serialize() << endl;

	return 0;
}

