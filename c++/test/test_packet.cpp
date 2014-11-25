/* g++ -g -Wno-deprecated -I../base test_packet.cpp ../base/libbase.a -o test_packet  
 * test_packet.cpp
 *
 *  Created on: 2014-11-12
 * 	Author: Administrator
 */

#include "util.h"
#include "impdu.h"

int main()
{
    char *data = const_cast<char *>("hello 0world");
    uint32_t data_len = strlen(data);

    CImPduClientData pdu(data, data_len);
    printf("pdu lenght: %d\n", pdu.GetLength());

    uchar_t *buf = pdu.GetBuffer();

    int i = 0;
    for (i = 0; i < pdu.GetLength(); i++) {
    	printf("%X\n", buf[i]);
    }
    cout << endl;


	return 0;
}

