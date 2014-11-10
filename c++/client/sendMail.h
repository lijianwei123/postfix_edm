/*
 *  sendMail.h
 *
 *  Created on: 2014-10-25
 *  Author: Administrator
 */

#ifndef SENDMAIL_H_
#define SENDMAIL_H_

#include "util.h"

//邮件信息
typedef struct
{
	char *to;
	char *cc;
	char *subject;
	char *content;

	char *sender; //human
	char *from;
}mail_info_t;

#define free_mail_info(mail_info_ptr) \
	free(mail_info_ptr->to); \
	free(mail_info_ptr->cc); \
	free(mail_info_ptr->subject); \
	free(mail_info_ptr->content); \
	free(mail_info_ptr->sender); \
	free(mail_info_ptr->from); \
	free(mail_info_ptr)

class cSendMail : public CRefObject {
public:
	cSendMail();
	virtual ~cSendMail();

	int send(mail_info_t *mail_info_ptr);
public:
	static cSendMail *instance;
};

#endif /* SENDMAIL_H_ */
