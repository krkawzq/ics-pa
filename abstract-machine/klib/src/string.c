#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  assert(s != NULL);
  size_t len = 0;
  while (s[len] != '\0') {
    len++;
  }
  return len;
}

char *strcpy(char *dst, const char *src) {
  assert(dst != NULL);
  size_t i = 0;
  while (src[i] != '\0') {
    dst[i] = src[i];
    i++;
  }
  dst[i] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  assert(dst != NULL);
  assert(src != NULL);
  size_t i = 0;
  while (i < n && src[i] != '\0') {
    dst[i] = src[i];
    i++;
  }
  dst[i] = '\0';
  return dst;
}

char *strcat(char *dst, const char *src) {
  assert(dst != NULL);
  assert(src != NULL);
  size_t i = 0;
  while (dst[i] != '\0') {
    i++;
  }
  size_t j = 0;
  while (src[j] != '\0') {
    dst[i] = src[j];
    i++;
    j++;
  }
  dst[i] = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  assert(s1 != NULL);
  assert(s2 != NULL);
  size_t i = 0;
  while (s1[i] != '\0' && s2[i] != '\0') {
    if (s1[i] != s2[i]) {
      return s1[i] - s2[i];
    }
    i++;
  }
  return s1[i] - s2[i];
}

int strncmp(const char *s1, const char *s2, size_t n) {
  assert(s1 != NULL);
  assert(s2 != NULL);
  size_t i = 0;
  while (i < n && s1[i] != '\0' && s2[i] != '\0') {
    if (s1[i] != s2[i]) {
      return s1[i] - s2[i];
    }
    i++;
  }
  if (i == n) return 0;
  return s1[i] - s2[i];
}

void *memset(void *s, int c, size_t n) {
  assert(s != NULL);
  size_t i = 0;
  while (i < n) {
    ((char *)s)[i] = c;
    i++;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  assert(dst != NULL);
  assert(src != NULL);
  size_t i = 0;
  while (i < n) {
    ((char *)dst)[i] = ((char *)src)[i];
    i++;
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  assert(out != NULL);
  assert(in != NULL);
  size_t i = 0;
  while (i < n) {
    ((char *)out)[i] = ((char *)in)[i];
    i++;
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  assert(s1 != NULL);
  assert(s2 != NULL);
  size_t i = 0;
  while (i < n) {
    if (((char *)s1)[i] != ((char *)s2)[i]) {
      return ((char *)s1)[i] - ((char *)s2)[i];
    }
    i++;
  }
  return 0;
}

#endif
