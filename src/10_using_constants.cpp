#include "examples.h"

void example10()
{
  // This data will be available within your assembly code. 
  // Each row is 32bytes in size (the same size as a YMM register).
  // When your assembly is executed, the address of the array will be loaded into the RCX register.
  // I've written the addresses you'll need to remember down the right hand side
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
  
    // specify some constant vaue
    uint32_t constant0 = a->set1_ps(4.5f);

    // specify 8 differing constants
    uint32_t constant1 = a->set_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);

    // load the two constants into registers
    a->load_const(vpu::YMM1, constant0);
    a->load_const(vpu::YMM2, constant1);

    // load value from args
    a->movaps(vpu::YMM0, vpu::RCX, 32);

    // multiply both constants
    a->mulps(vpu::YMM1, vpu::YMM1, vpu::YMM0);
    a->mulps(vpu::YMM2, vpu::YMM2, vpu::YMM0);

    // write results out to memory
    a->movaps(vpu::RCX, vpu::YMM1);
    a->movaps(vpu::RCX, 32, vpu::YMM2);

    // We must always include a call return at the end of our code! 
    // (otherwise the processor will just keep executing beyond the end of our array!)
    a->ret();

  a->end();

  // write out the assembled machine code, just incase we crash and need to debug. 
  print_machine_code("10_using_constants", a);

  // Cool! function has been assebled, so let's execute this bad boy, and see what we get!
  a->execute(argument_data);

  // now output what we ended up with in the array above
  print_args(argument_data, sizeof(argument_data) / (sizeof(float) * 8));

  a->release();
}
