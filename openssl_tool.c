/* ********************************
 * Author:       DJ
 * License:	     NULL
 * Description:  use openssl base65 sha256 hmac
 *               For usage, check the openssl_tool.h file
 *
 *//** @file openssl_tool.h *//*
 *
 ********************************/
 
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/hmac.h> 

 int base64_encode(char *in_str, int in_len, char *out_str)
{
    BIO *b64, *bio;
    BUF_MEM *bptr = NULL;
    size_t size = 0;

    if (in_str == NULL || out_str == NULL)
        return -1;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, in_str, in_len);
    if ( BIO_flush(bio) );
    //BIO_flush(bio);

    BIO_get_mem_ptr(bio, &bptr);
    memcpy(out_str, bptr->data, bptr->length);
    out_str[bptr->length] = '\0';
    size = bptr->length;
    BIO_free_all(bio);
    return size;
}

int base64_decode(char *in_str, int in_len, char *out_str)
{
    BIO *b64, *bio;
    //BUF_MEM *bptr = NULL;
    //int counts;
    int size = 0;

    if (in_str == NULL || out_str == NULL)
        return -1;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    bio = BIO_new_mem_buf(in_str, in_len);
    bio = BIO_push(b64, bio);

    size = BIO_read(bio, out_str, in_len);
    out_str[size] = '\0';

    BIO_free_all(bio);
    return size;
}

void sha256(char* src,char * hash)
{
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, src, strlen(src));
    SHA256_Final(hash, &sha256);

}

// ---- sha256摘要哈希 ---- //    
void generateSAK(char * mac,  unsigned char * output)  
{
	char txt[128]={0x0};
	uint8_t hash[SHA256_DIGEST_LENGTH];
	char *sak_seed="1234567890123456789012345678901234567890";
	int i=0;
	
	sprintf(txt,"%s:%s",mac,sak_seed);
	printf("generateSAK:txt=%s\n",txt);
	sha256(txt,hash);
	printf("sha256:");
	for(i; i < SHA256_DIGEST_LENGTH; i++){
		printf("%02X",hash[i]);
	}
	printf("\n");
	base64_encode(hash,strlen(hash),output);
}


// ---- sha256摘要哈希 ---- //    
void hmacsha256(char * mac, char * input, unsigned int input_length,  
                unsigned char * output, unsigned int * output_length)  
{  
    // The secret key for hashing  
    char key[64]={0x0};
	generateSAK(mac,key);
	key[40]='\0';
	printf("SAK:%s\n",key);
  
    // Be careful of the length of string with the choosen hash engine. SHA1 needed 20 characters.  
    // Change the length accordingly with your choosen hash engine.   

    HMAC_CTX ctx;  
    HMAC_CTX_init(&ctx);  
    HMAC_Init_ex(&ctx, key, strlen(key), EVP_sha256(), NULL);  
    HMAC_Update(&ctx, (unsigned char*)input, strlen(input));        // input is OK; &input is WRONG !!!  

    HMAC_Final(&ctx, output, output_length);  
    HMAC_CTX_cleanup(&ctx);    
}
