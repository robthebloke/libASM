#include "examples.h"

void example05()
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

  // our assembler
  vpu::IAssembler* a = g_lib->createAssembler();

  // start assembling
  a->begin();
  
    // ok, so how do we make use of the stack within our applications?
    // First, a small problem. We are using 32byte YMM registers, and we possible want those to be 
    // aligned to 32bytes (so we can use movaps instead of movups). This poses a small problem, since the 
    // x64 stack will only give us 16byte alignment. As a result, this example will only use unaligned loads/stores
    // to the actual stack itself. 
    //
    // So, on entering our function, RSP will point to the current stack location (which will be 16byte aligned)
    // 
    // We will need to consume some memory for the stack for 2 YMM values (which we will store on the stack)
    const uint32_t required_stack_size = 64;

    // The RBP register is used by functions to provide a basis for where their stack space is located. We will be 
    // modifying this (to point to our stack allocation), so we will need to store that value, and restore it at 
    // the end of the call. So push the value, so we can retore later. 
    a->push(vpu::RBP);

    // Now then, for reasons unknown, the stack is actually the wrong way around. We need to subtract from the stack pointer,
    // rather than add (odd yes, but there we go). So let's move the stack pointer back by the amount we need. 
    a->sub(vpu::RSP, required_stack_size);

    // Store the modified stack location (RSP) in the base pointer (RBP). 
    a->lea(vpu::RBP, vpu::RSP, 0);
    
    // let's move two registers in from our array into YMM registers.
    a->movaps(vpu::YMM0, vpu::RCX, 32);
    a->movaps(vpu::YMM1, vpu::RCX, 64);

    // copy those onto the stack (NOTE: we must used unaligned moves!)
    a->movups(vpu::RBP, 0, vpu::YMM0);
    a->movups(vpu::RBP, 32, vpu::YMM1);

    // We now have two YMM values stored on our stack. Woot!

    // Now let's move those values back into registers. 
    a->movups(vpu::YMM2, vpu::RBP, 0);
    a->movups(vpu::YMM3, vpu::RBP, 32);

    // and move them back to our memory... (in the reverse order they came in)
    a->movaps(vpu::RCX, 32, vpu::YMM3);
    a->movaps(vpu::RCX, 64, vpu::YMM2);

    // finally, move the stack pointer back to where it was when the function was called
    a->add(vpu::RSP, required_stack_size);

    // and restore the RBP register.
    a->pop(vpu::RBP);

    // don't forget to return!
    a->ret();

  // done assembling
  a->end();

  // And that, is *almost* all you need to know about using the stack. The next example will demonstrate how to align 
  // the stack to 32bytes, so we can use aligned moves instead. 

  // print code, execute, and print results
  print_machine_code("05_using_the_stack", a);
  a->execute(argument_data);
  print_args(argument_data, sizeof(argument_data) / (sizeof(float) * 8));

  a->release();
}
