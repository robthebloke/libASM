#include "examples.h"

void example09()
{
  VPU_ALIGN_PREFIX(32)
  float argument_data[][8] =
  {
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },   // RCX
    { 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f },   // RCX + 32
    { 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f },   // RCX + 64
    { 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f },   // RCX + 96
    { 0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f },   // RCX + 128
    { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f },   // RCX + 160
    { 0.6f, 0.6f, 0.6f, 0.6f, 0.6f, 0.6f, 0.6f, 0.6f },   // RCX + 192
    { 0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f },   // RCX + 224
    { 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f },   // RCX + 256
    { 0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f },   // RCX + 288
    { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },   // RCX + 320
    { 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f },   // RCX + 352
    { 1.2f, 1.2f, 1.2f, 1.2f, 1.2f, 1.2f, 1.2f, 1.2f },   // RCX + 384
    { 1.3f, 1.3f, 1.3f, 1.3f, 1.3f, 1.3f, 1.3f, 1.3f },   // RCX + 416
    { 1.4f, 1.4f, 1.4f, 1.4f, 1.4f, 1.4f, 1.4f, 1.4f },   // RCX + 480
    { 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f },   // RCX + 512
  }
  VPU_ALIGN_SUFFIX(32);

  // the function map
  vpu::IFunctionTable* functions = g_lib->createFunctionTable();
  functions->add_defaults();

  // our assembler
  vpu::IAssembler* a = g_lib->createAssembler();

  // start assembling
  a->begin();

    const uint32_t required_stack_size = 64 + 32;

    // function prologue
    a->push(vpu::RBP);
    a->sub(vpu::RSP, required_stack_size);
    a->lea(vpu::RBP, vpu::RSP, 32);
    a->and(vpu::RBP, -32);
  
    // I'm going to compute  (YMM0 + YMM1) * sin(YMM0 + YMM1)
    // So ideally, I want to reuse the  value (YMM0 + YMM1). 
    a->movaps(vpu::YMM0, vpu::RCX, 32);
    a->movaps(vpu::YMM1, vpu::RCX, 64);
    a->addps(vpu::YMM0, vpu::YMM0, vpu::YMM1);

    // lets store RCX, RDX, and YMM0 on the stack
    a->movaps(vpu::RBP, 0, vpu::YMM0);
    a->mov64(vpu::RBP, 32, vpu::RCX);
    a->mov64(vpu::RBP, 40, vpu::RDX);

    // call our func (YMM0 will store result)
    a->call("sin", functions);

    // restore RCX/RDX
    a->mov64(vpu::RCX, vpu::RBP, 32);
    a->mov64(vpu::RDX, vpu::RBP, 40);
  
    // perform final multiplication (using value stored on the stack)
    a->mulps(vpu::YMM0, vpu::YMM0, vpu::RBP, 0);

    // store result
    a->movaps(vpu::RCX, 0, vpu::YMM0);

    // function epilogue
    a->add(vpu::RSP, required_stack_size);
    a->pop(vpu::RBP);
    a->ret();

  // done assembling
  a->end();

  // print code, execute, and print results
  print_machine_code("09_restoring_registers_after_call", a);
  a->execute(argument_data, functions);
  print_args(argument_data, sizeof(argument_data) / (sizeof(float) * 8));

  a->release();
  functions->release();
}

