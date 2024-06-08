#include "memory.h"
void* memset(void* pointer, uint8 value, uint16 num) {
    uint8* u8Ptr = (uint8*)pointer;
    for (uint16 i = 0; i < num; i++)
        u8Ptr[i] = value;

    return pointer;
}
void* memcpy(void* dst, const void* src, const uint32 len){
    for (uint32 i = 0; i < len; i++)
        ((uint8 *)dst)[i] = ((uint8 *)src)[i];

    return dst;
}

int strncmp(const char* str1, const char* str2, int n) {
    for (int i = 0; i < n; ++i) 
        if (str1[i] != str2[i] || str1[i] == '\0' || str2[i] == '\0') 
            return (unsigned char)str1[i] - (unsigned char)str2[i];
        
    return 0;
}
int strlen(const char* str){
    int len = 0;
    for (len = 0; *str != '\0'; str++) len++;
    
    return len;
}
char *strncpy(char *s1, const char *s2, int n)
{
	unsigned int extern_iter = 0; //when s2's length is shorter than n, this allows the function to continue padding null characters

	unsigned int iterator = 0;
	for (iterator = 0; iterator < n; iterator++) //iterate through s2 up to char n, copying them to s1
	{
		if (s2[iterator] != '\0')
			s1[iterator] = s2[iterator];
		else //the end of s2 was found prematurely - copy the null character, update external iterator and quit for loop
		{
			s1[iterator] = s2[iterator];
			extern_iter = iterator + 1;
			break;
		}
	}

	while (extern_iter < n) //while there are still spaces that need to be filled with null characters, fill them
	{
		s1[extern_iter] = '\0';
		extern_iter++;
	}

	return s1;
}
char *strcpy(char* s1, const char* s2){
    strncpy(s1, s2, strlen(s2) + 1);
	s1[strlen(s2)] = '\0'; //tack on the null terminating character if it wasn't already done
	return s1;

}
char* upper_case(char* in){
	uint16 it;
	uint16 len = strlen(in);
	for (uint16 it = 0; it < len; it++)
	{
		if ((short)in[it] >= 97 && (short)in[it] <= 122)
			in[it] -=32;
		
	}
	return in;
}
void* __rawmemchr (const void* s, int c_in) {
  const unsigned char *char_ptr;
  const unsigned long int *longword_ptr;
  unsigned long int longword, magic_bits, charmask;
  unsigned char c;

  c = (unsigned char) c_in;

  /* Handle the first few characters by reading one character at a time.
     Do this until CHAR_PTR is aligned on a longword boundary.  */
  for (char_ptr = (const unsigned char *) s;
       ((unsigned long int) char_ptr & (sizeof (longword) - 1)) != 0;
       ++char_ptr)
    if (*char_ptr == c)
      return (void*) char_ptr;

  /* All these elucidatory comments refer to 4-byte longwords,
     but the theory applies equally well to 8-byte longwords.  */

  longword_ptr = (unsigned long int *) char_ptr;

#if LONG_MAX <= LONG_MAX_32_BITS
  magic_bits = 0x7efefeff;
#else
  magic_bits = ((unsigned long int) 0x7efefefe << 32) | 0xfefefeff;
#endif

  /* Set up a longword, each of whose bytes is C.  */
  charmask = c | (c << 8);
  charmask |= charmask << 16;
#if LONG_MAX > LONG_MAX_32_BITS
  charmask |= charmask << 32;
#endif

    while (1) {
        longword = *longword_ptr++ ^ charmask;

        /* Add MAGIC_BITS to LONGWORD.  */
        if ((((longword + magic_bits) ^ ~longword) & ~magic_bits) != 0) {
                const unsigned char *cp = (const unsigned char *) (longword_ptr - 1);

                if (cp[0] == c) return (void*)cp;
                if (cp[1] == c) return (void*)&cp[1];
                if (cp[2] == c) return (void*)&cp[2];
                if (cp[3] == c) return (void*)&cp[3];

                #if LONG_MAX > 2147483647
                    if (cp[4] == c) return (void*) &cp[4];
                    if (cp[5] == c) return (void*) &cp[5];
                    if (cp[6] == c) return (void*) &cp[6];
                    if (cp[7] == c) return (void*) &cp[7];
                #endif
            }
    }
}

char* strpbrk (const char* s, const char* accept) {
    while (*s != '\0') {
        const char *a = accept;
        while (*a != '\0')
	        if (*a++ == *s)
	            return (char *) s;

            ++s;
    }

  return NULL;
}

uint32 strspn(const char* s, const char* accept) {
    const char *p;
    const char *a;
    uint32 count = 0;

    for (p = s; *p != '\0'; ++p) {
        for (a = accept; *a != '\0'; ++a)
            if (*p == *a)
                break;

        if (*a == '\0') return count;
        else ++count;
    }

    return count;
}

static char* olds;
char* strtok (char* string, const char* delim){
    char* token;
    if (string == NULL) string = olds;

    string += strspn(string, delim);
    if (*string == '\0') {
        olds = string;
        return NULL;
    }

    token = string;
    string = strpbrk(token, delim);
    if (string == NULL) olds = (char*)__rawmemchr(token, '\0');
    else {
        *string = '\0';
        olds = string + 1;
    }

    return token;
}
int strstr(const char* haystack, const char* needle) {
    if (*needle == '\0')    // If the needle is an empty string, return 0 (position 0).
        return 0;
    
    int position = 0;       // Initialize the position to 0.
    while (*haystack) {
        const char* hay_ptr     = haystack;
        const char* needle_ptr  = needle;

        // Compare characters in the haystack and needle.
        while (*hay_ptr == *needle_ptr && *needle_ptr) {
            hay_ptr++;
            needle_ptr++;
        }

        // If we reached the end of the needle, we found a match.
        if (*needle_ptr == '\0') 
            return position;

        // Move to the next character in the haystack.
        haystack++;
        position++;
    }

    return -1;  // Needle not found, return -1 to indicate that.
}
