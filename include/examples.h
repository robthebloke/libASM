#pragma once 
#include "lib_asm.h"
#include <cstdio>
#include <cstring>

inline void print_args(float data[][8], uint32_t count)
{
  printf("\n  result:\n");
  for (uint32_t i = 0; i < count; ++i)
  {
    printf("  %2.4f %2.4f %2.4f %2.4f  %2.4f %2.4f %2.4f %2.4f\n",
      data[i][0], data[i][1], data[i][2], data[i][3], data[i][4], data[i][5], data[i][6], data[i][7]);
  }
}

inline void print_args(float data[], uint32_t count)
{
  printf("\n  result:\n");
  for (uint32_t i = 0; i < count; ++i)
  {
    float* a = data + 8 * i;
    printf("  %2.4f %2.4f %2.4f %2.4f  %2.4f %2.4f %2.4f %2.4f\n",
      a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
  }
}


inline void print_machine_code(const char* title, const vpu::IAssembler* a)
{
  printf("\n%s\n", title);
  uint32_t n = (uint32_t)a->numBytes();
  const char* b = "0123456789ABCDEF";
  for (uint32_t i = 0; i < n; ++i)
  {
    if (i && ((i & 7) == 0)) printf(" ");
    if (i && ((i & 15) == 0)) printf("\n");
    printf(" 0x%c%c", b[a->bytecode()[i] >> 4], b[a->bytecode()[i] & 0xF]);
  }
}

// defined in main.cpp
extern vpu::AssemblerLib* g_lib;
