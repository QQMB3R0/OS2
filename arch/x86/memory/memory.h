#define MEMORY_H
#ifdef MEMORY_H
#include "../inc/types.h"
void* memset(void* pointer, uint8 value, uint16 num);
void* memcpy(void* dst, const void* src, const uint32 len);
int strncmp(const char* str1, const char* str2, int n);
int strlen(const char* str);
char *strncpy(char *s1, const char *s2, int n);
int strcmp(const char *s1, const char *s2);
char *strcpy(char* s1, const char* s2);
char* upper_case(char* in);
void* __rawmemchr (const void* s, int c_in) ;
char* strpbrk (const char* s, const char* accept) ;
uint32 strspn(const char* s, const char* accept) ;
char* strtok (char* string, const char* delim);
int strstr(const char* haystack, const char* needle) ;
#endif