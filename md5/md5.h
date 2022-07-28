/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-07-26 14:45:06
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-07-28 10:03:01
 * @FilePath: /tcp-server/md5/md5.h
 * @Description: 计算文件或者字符串的MD5
 */
#ifndef MD5_H 
#define MD5_H 
 
#ifdef __cplusplus 
extern "C" 
{ 
#endif             /* __cplusplus */ 
 
#ifdef HAVE_CONFIG_H 
# include "config.h" 
#endif 
 
#ifdef HAVE_STDINT_H 
  #include <stdint.h> 
  typedef uint32_t uint32; 
#else 
  /* A.Leo.: this wont work on 16 bits platforms ;) */ 
  typedef unsigned uint32; 
#endif 
 
#define MD5_FILE_BUFFER_LEN 1024 
 
struct MD5Context { 
  uint32 buf[4]; 
  uint32 bits[2]; 
  unsigned char in[64]; 
}; 
 
void md5Init(struct MD5Context *context); 
void md5Update(struct MD5Context *context, unsigned char const *buf, 
      unsigned len); 
void md5Final(unsigned char digest[16], struct MD5Context *context); 
void md5Transform(uint32 buf[4], uint32 const in[16]); 
 
int GetBytesMD5(const unsigned char* src, unsigned int length, char* md5); 
int GetStringMD5(const char* src, char* md5); 
int GetFileMD5(const char* path, char* md5); 
 
/* 
 * This is needed to make RSAREF happy on some MS-DOS compilers. 
 */ 
typedef struct MD5Context MD5_CTX; 
 
#ifdef __cplusplus 
} 
#endif             /* __cplusplus */ 
 
#endif /* !MD5_H */ 
