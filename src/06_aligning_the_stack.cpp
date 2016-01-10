#include "examples.h"

void example06()
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

    // ok, so how do we deal with the fact the stack is aligned to 16bytes, yet we are using types that are 32byte aligned?
    // 
    // The simple option really, is to add an extra 32bytes to our stack allocation:
    const uint32_t required_stack_size = 64 + 32;

    // Again, ensure we store RBP  so we can restore it later. 
    a->push(vpu::RBP);

    // This time, move the stack pointer back the size we need (and an extra 32bytes).
    a->sub(vpu::RSP, required_stack_size);

    // ok, so modify the base pointer so it is 32 bytes infront of the stack pointer. This is our alignment fudge.
    a->lea(vpu::RBP, vpu::RSP, 32);

    // if we bitwise and RBP with -32, RBP will either be 16 or 32 bytes infront of RSP (but we will now be on a 32byte boundary).
    a->and(vpu::RBP, -32);

    // The rest is effectvely the same.... 

    // let's move two registers in from our array
    a->movaps(vpu::YMM0, vpu::RCX, 32);
    a->movaps(vpu::YMM1, vpu::RCX, 64);

    // and now let's stick those on the stack
    a->movaps(vpu::RBP, 0, vpu::YMM0);
    a->movaps(vpu::RBP, 32, vpu::YMM1);

    // pointless yes, but there we go (this may be needed if you wish to preserve register values when calling a function).

    // and now let's move those values back into registers. 
    a->movaps(vpu::YMM2, vpu::RBP, 0);
    a->movaps(vpu::YMM3, vpu::RBP, 32);

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

  // and there you have it, you can now use the stack to store intermediate values. I'm not sure how many values you can use
  // before you blow the top of the stack - that will depend on the stack size you specify when compiling. I'm also not sure how
  // chkstk comes into play here (the intel asm routine that is usually used to resize the stack when needed). 

  // print code, execute, and print results
  print_machine_code("06_aligning_the_stack", a);
  a->execute(argument_data);
  print_args(argument_data, sizeof(argument_data) / (sizeof(float) * 8));
  a->release();
}
