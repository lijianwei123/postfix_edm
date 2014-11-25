//g++ -g  -Wno-deprecated -I../base -I../client `mysql_config --libs` test_send_mail.cpp ../client/sendMail.cpp  ../base/libbase.a  -o test_send_mail
/*
 * test_send_mail.cpp   测试发送邮件
 *
 *  Created on: 2014-11-25
 *  Author: Administrator
 */
#include "util.h"
#include "sendMail.h"

using namespace std;

mysql_connect_info_t  mysql_connect_info;
queue<mail_info_t *> send_mail_queue;

#define PULL_SENT_MAIL_UNIT 100
const char *emailFromAddr = "no-reply@xz.mail.veryeast.com";

const char *LOG_FILE_NAME = "test.log";

void pullSentInfoFromMysql()
{
	result_data_t result_data;
	MYSQL conn;
	int retCode = -1;

	retCode = mysql_user_connect(&conn, &mysql_connect_info);
	assert(retCode == 0);

	retCode = mysql_select_db(&conn, mysql_connect_info.db);
	assert(retCode == 0);

	char select_sql[200] = {0};
	snprintf(select_sql, sizeof(select_sql), "select * from send_email where `status` = 1 and `from` = '%s' group by `to` limit %d", emailFromAddr, PULL_SENT_MAIL_UNIT);
	printf("%s\n", select_sql);
	retCode = mysql_select(&conn, select_sql, &result_data);

	printf("rows:%d, cols:%d\n", result_data.rows, result_data.columns);

	mysql_close(&conn);

	unsigned int i = 0, j = 0;
	mysql_field_value_t *pointer;
	mail_info_t *mail_info_ptr = NULL;

	for (i = 0; i < result_data.rows; i++) {
		mail_info_ptr = (mail_info_t *)calloc(1, sizeof(mail_info_t));
		pointer = *(result_data.data + i);
		for (j = 0; j < result_data.columns; j++) {
			if (!strncasecmp(pointer->next->fieldName, "to", sizeof("to")) && pointer->next->fieldValue) {
				mail_info_ptr->to = strdup(pointer->next->fieldValue);
			} else if (!strncasecmp(pointer->next->fieldName, "cc", sizeof("cc"))  && pointer->next->fieldValue) {
				mail_info_ptr->cc = strdup(pointer->next->fieldValue);
			} else if (!strncasecmp(pointer->next->fieldName, "subject", sizeof("subject"))  && pointer->next->fieldValue) {
				mail_info_ptr->subject = strdup(pointer->next->fieldValue);
			}else if (!strncasecmp(pointer->next->fieldName, "content", sizeof("content"))  && pointer->next->fieldValue) {
				mail_info_ptr->content = strdup(pointer->next->fieldValue);
			}else if (!strncasecmp(pointer->next->fieldName, "sender", sizeof("sender")) && pointer->next->fieldValue) {
				mail_info_ptr->sender = strdup(pointer->next->fieldValue);
			}else if (!strncasecmp(pointer->next->fieldName, "from", sizeof("from")) && pointer->next->fieldValue) {
				mail_info_ptr->from = strdup(pointer->next->fieldValue);
			}

			pointer = pointer->next;
		}
		send_mail_queue.push(mail_info_ptr);
	}
	free_result_data(&result_data);
}

int main()
{
	mysql_connect_info.host = const_cast<char *>("122.224.97.234");
	mysql_connect_info.db = const_cast<char *>("postfix_log");
	mysql_connect_info.port = 3305;
	mysql_connect_info.user = const_cast<char *>("postfix_log");
	mysql_connect_info.pwd = const_cast<char *>("postfix_log");

	cSendMail *_sendMail = cSendMail::instance;
	_sendMail->setConvert(true);
	mail_info_t *mail_info_ptr = NULL;
	int start = 0;

	while (true) {
		cout << "send mail start" << endl;
		pullSentInfoFromMysql();
		cout << "pull send info size" << send_mail_queue.size() << endl;
		cout << "start ?" << endl;
		cin >> start;

		if (start) {
			//有数据的话
			while (!send_mail_queue.empty()) {
				mail_info_ptr = send_mail_queue.front();
				send_mail_queue.pop();
				_sendMail->send(mail_info_ptr);
				free_mail_info(mail_info_ptr);
			}
		}
	}
}

