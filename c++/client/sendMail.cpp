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
	_convert_ptr = new CodeConverter("utf-8", "gb2312");
	_isConvert = false;
}

cSendMail::~cSendMail()
{
	delete _convert_ptr;
}

//发送邮件
//@see http://www-01.ibm.com/support/knowledgecenter/ssw_aix_61/com.ibm.aix.cmds5/sendmail.htm?lang=zh
//@see php源码中的mail函数
int cSendMail::send(mail_info_t *mail_info_ptr)
{
	log("send mail start\n");

	FILE *sendMail = NULL;
	char sendMailCommand[] = "/usr/sbin/sendmail -t -i";

	//popen http://blog.chinaunix.net/uid-97185-id-4330827.html
	sendMail = popen(sendMailCommand, "w");
	if (sendMail == NULL) {
		log("popen error: %s\n", strerror(errno));
		return -1;
	}

	/**
	 *	邮箱格式  参照  http://php.net/manual/zh/function.mail.php
	 *	user@example.com
	 *	user@example.com, anotheruser@example.com
	 *	User <user@example.com>
	 *	User <user@example.com>, Another User <anotheruser@example.com>
	 */
	fprintf(sendMail, "To: %s\n", mail_info_ptr->to);
	if (mail_info_ptr->cc) {
		fprintf(sendMail, "Cc: %s\n", mail_info_ptr->cc);
	}
	//解决中文乱码  @see http://blog.catjia.com/linux-system/email-service/sendmailmail%E5%8F%91%E9%80%81%E9%82%AE%E4%BB%B6%E4%B8%BB%E9%A2%98%E4%B9%B1%E7%A0%81%E7%9A%84%E8%A7%A3%E5%86%B3%E6%96%B9%E6%B3%95.html
	string base64_subject, base64_sender;
	base64_subject = base64_encode(reinterpret_cast<const unsigned char *>(mail_info_ptr->subject), strlen(mail_info_ptr->subject));
	base64_sender = base64_encode(reinterpret_cast<const unsigned char *>(mail_info_ptr->sender), strlen(mail_info_ptr->sender));

	fprintf(sendMail, "Subject: =?UTF-8?B?%s?=\n", base64_subject.c_str());
	fprintf(sendMail, "From: =?UTF-8?B?%s?=<%s>\n", base64_sender.c_str(), mail_info_ptr->from);

	fprintf(sendMail, "Content-type: text/html; charset=gb2312\n");
	fprintf(sendMail, "Author: lijianwei\n");

	if (_isConvert) {
		//转换内容
		char *convert_content = (char *)calloc(1, strlen(mail_info_ptr->content) * 2);
		_convert_ptr->convert(mail_info_ptr->content, strlen(mail_info_ptr->content), convert_content, strlen(mail_info_ptr->content)*2);
		fprintf(sendMail, "\n%s\n", convert_content);
		free(convert_content);
	} else {
		fprintf(sendMail, "\n%s\n", mail_info_ptr->content);
	}

	pclose(sendMail);
	return 0;
}
