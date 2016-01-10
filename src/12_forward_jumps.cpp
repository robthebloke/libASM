#include "examples.h"

__declspec(dllexport) void example12()
{
  // This example will implement an if/else statement using jump. This may be used for early-out codepaths 
  // (for example skipping the computation of lighting if all rays miss a surface)
  VPU_ALIGN_PREFIX(32)
  float pos_argument_data[][8] =
  {
    { 9.0f, 9.0f, 9.0f, 9.0f, 9.0f, 9.0f, 9.0f, 9.0f },   // RCX
  }
  VPU_ALIGN_SUFFIX(32);
  
  VPU_ALIGN_PREFIX(32)
  float neg_argument_data[][8] =
  {
    { -9.0f, -9.0f, -9.0f, -9.0f, -9.0f, -9.0f, -9.0f, -9.0f },   // RCX
  }
  VPU_ALIGN_SUFFIX(32);

  // our assembler
  vpu::IAssembler* a = g_lib->createAssembler();

  a->begin();
    // store RBX (we'll be modifiying it)
    a->push(vpu::RBX);

    // load value from args
    a->movaps(vpu::YMM0, vpu::RCX, 0);

    // let's see if they are all negative
    a->movemaskps(vpu::RBX, vpu::YMM0);

    // let's compare the movemask result with 0xFF (all bits set)
    a->cmp(vpu::RBX, 0xFF);

    // jump to the label
    a->jump_eq_label("all_negative");

    // multiply the arg together, store, and return
    a->mulps(vpu::YMM0, vpu::YMM0, vpu::YMM0);
    a->movaps(vpu::RCX, 0, vpu::YMM0);

    // restore RBX
    a->pop(vpu::RBX);
    a->ret();

  a->insert_label("all_negative");

    // add the arg to itself, store, and return
    a->addps(vpu::YMM0, vpu::YMM0, vpu::YMM0);
    a->movaps(vpu::RCX, 0, vpu::YMM0);

    // restore RBX
    a->pop(vpu::RBX);
    a->ret();

  a->end();

  // write out the assembled machine code, just incase we crash and need to debug. 
  print_machine_code("12_forward_jumps", a);

  // call with the +ve argument 
  a->execute(pos_argument_data);
  print_args(pos_argument_data, sizeof(pos_argument_data) / (sizeof(float) * 8));

  // call with the -ve argument 
  a->execute(neg_argument_data);
  print_args(neg_argument_data, sizeof(neg_argument_data) / (sizeof(float) * 8));

  a->release();
}
