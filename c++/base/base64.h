/*
 * 	base64.h
 *
 *  Created on: 2014-11-25
 *  Author: Administrator
 */

#ifndef BASE64_H_
#define BASE64_H_


#include <string>

//不需要释放内存的
//string生命周期后会自动释放所占用的内存
std::string base64_encode(unsigned char const* , unsigned int len);
//不需要释放内存的
//string生命周期后会自动释放所占用的内存
std::string base64_decode(std::string const& s);

#endif /* BASE64_H_ */
