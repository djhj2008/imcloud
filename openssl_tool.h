/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __OPENSSL_TOOL_H__
#define __OPENSSL_TOOL_H__

/* =================================== API ======================================= */

int base64_encode(char *in_str, int in_len, char *out_str);
int base64_decode(char *in_str, int in_len, char *out_str);
void sha256(char* src,char * hash);
void generateSAK(char * mac,  unsigned char * output);
void hmacsha256(char * mac, char * input, unsigned int input_length,  
                unsigned char * output, unsigned int * output_length);

#endif
