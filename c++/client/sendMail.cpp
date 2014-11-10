/*
 * sendMail.cpp
 *
 *  Created on: 2014-10-25
 *  Author: lijianwei
 */

#include "sendMail.h"
#include "util.h"

cSendMail *cSendMail::instance = getInstance<cSendMail>();

cSendMail::cSendMail()
{
}

cSendMail::~cSendMail()
{
}

//发送邮件
int cSendMail::send(mail_info_t *mail_info_ptr)
{
	FILE *fp = NULL;
	char execCommand[500] = {0};

	snprintf(execCommand, 500, "mail -s '%s' -r '%s <%s>' '%s'", mail_info_ptr->subject,
			mail_info_ptr->sender, mail_info_ptr->from, mail_info_ptr->to);

	log("send mail command: %s ", execCommand);

	//popen http://blog.chinaunix.net/uid-97185-id-4330827.html
	fp = popen(execCommand, "w");
	if (fp == NULL) {
		log("popen error: %s ", strerror(errno));
		return -1;
	}
	fputs(mail_info_ptr->content, fp);
	fclose(fp);
	return 0;
}
