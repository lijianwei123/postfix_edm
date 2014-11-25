/*
 * test_base_encode.cpp  测试base64_encode
 *
 *  Created on: 2014-11-25
 *  Author: Administrator
 */

#include <iostream>
#include "base64.h"


using namespace std;




int main() {
  const std::string s = "李建伟123" ;

  std::string encoded = base64_encode(reinterpret_cast<const unsigned char*>(s.c_str()), s.length());
  std::string decoded = base64_decode(encoded);

  std::cout << "encoded: " << encoded << std::endl;
  std::cout << "decoded: " << decoded << std::endl;

  return 0;
}

