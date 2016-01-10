#include "examples.h"

void example11()
{
  VPU_ALIGN_PREFIX(32)
  float argument_data[][8] =
  {
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },   // RCX
    { 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f },   // RCX + 32
    { 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f },   // RCX + 64
    { 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f },   // RCX + 96
  }
  VPU_ALIGN_SUFFIX(32);

  // our assembler
  vpu::IAssembler* a = g_lib->createAssembler();

  a->begin();

    // load value from args
    a->movaps(vpu::YMM0, vpu::RCX, 0);
    a->movaps(vpu::YMM1, vpu::RCX, 32);
    a->movaps(vpu::YMM2, vpu::RCX, 64);
    a->movaps(vpu::YMM3, vpu::RCX, 96);

    // call our subroutine (which have not yet written)
    a->call_prodecure("vec2_add");

    // write results out to memory
    a->movaps(vpu::RCX, vpu::YMM0);
    a->movaps(vpu::RCX, 32, vpu::YMM1);

    // We must always include a call return at the end of our code! 
    // (otherwise the processor will just keep executing beyond the end of our array!)
    a->ret();

  // And here is the definition of our function. Note: Since we exist within ASM at this point, 
  // the calling convention can be whatever the hell you want. This func assumes arguments passed
  // in YMM0 -> YMM3, and returns the result in YMM0 & YMM1. 
  a->prodecure("vec2_add");
    a->addps(vpu::YMM0, vpu::YMM0, vpu::YMM2);
    a->addps(vpu::YMM1, vpu::YMM1, vpu::YMM3);
    a->ret();

  a->end();

  // write out the assembled machine code, just incase we crash and need to debug. 
  print_machine_code("11_defining_sub_routines", a);

  // Cool! function has been assebled, so let's execute this bad boy, and see what we get!
  a->execute(argument_data);

  // now output what we ended up with in the array above
  print_args(argument_data, sizeof(argument_data) / (sizeof(float) * 8));

  a->release();
}
