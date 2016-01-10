#include "lib_asm.h"

vpu::AssemblerLib* g_lib = 0;

extern void example00();
extern void example01();
extern void example02();
extern void example03();
extern void example04();
extern void example05();
extern void example06();
extern void example07();
extern void example08();
extern void example09();
extern void example10();
extern void example11();
extern void example12();

int main()
{
  // library initialisation
  g_lib = new vpu::AssemblerLib("libASM.dll");
  if (g_lib->isOk())
  {
    // run examples
    example00();
    example01();
    example02();
    example03();
    example04();
    example05();
    example06();
    example07();
    example08();
    example09();
    example10();
    example11();
    example12();
  }
  // free library
  delete g_lib;
  return 0;
}
