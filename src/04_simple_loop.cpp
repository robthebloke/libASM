#include "examples.h"

void example04()
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

  // This example demonstrates a simple loop. The equivalent C++ code would look like:

#if 0

  __m256 YMM0 = _mm256_setzero_ps();
  __m256* R9 = (__m256*)(argument_data + 1);
  for (int i = 0; i < 10; ++i)
  {
    YMM0 = _mm256_add_ps(YMM0, R9[i]);
  }
  _mm256_store_ps(argument_data[0], YMM0);

#endif

  // start assembling
  a->begin();

    // we'll accumulate the sum of the array in YMM0
    a->setzero(vpu::YMM0);

    // load the R9 register with the integer value 10 (this will count doown to zero).
    // NOTE: The registers R8 -> R15 must be used if your loop counter is greater than 0xFFFFFFFF. 
    // for smaller counter values, then you might be better off using RDX or RBX. This will result in shorter 
    // byte code. (R8->R15 can load 64bit values, the rest load 32bit immediate values)
    a->loadcount(vpu::R9, 10);

    // lea is actually a simple addition (of sorts). There is also an 'add' operation, but it doesn't quite 
    // do what you might want. 
    //
    // LEA:  RAX  =  RCX + 32
    // ADD:  RAX  += *(RCX + 32)
    // 
    // Typically, if you're modifiying addresses, you'll use lea. 
    // So this instruction adds 32 to RCX, and stores the result in RAX (which is generally used for addresses). 
    // We can't simply modify RCX though, because that will modify it's value (and when we return from this code
    // to C++, that code will expect RCX to be the same value - possible crash!).
    a->lea(vpu::RAX, vpu::RCX, 32);

    // keep track of where our loop starts. 
    uint32_t loop_start = uint32_t(a->numBytes());

    // wrapping the loop in brackets for readability
    {
      // YMM0 = YMM0 + arg[RAX]
      a->addps(vpu::YMM0, vpu::YMM0, vpu::RAX, 0);

      // add 32bytes to RAX (so RAX refers to the next element)
      a->lea(vpu::RAX, vpu::RAX, 32);

      // decrement R9
      a->dec(vpu::R9);

      // Jump NE (not equal, which weirdly also works and not-zero) will look at the last operations result. 
      // If the value is not equal to zero, it will jump back to our loop point from earlier. 
      // If it is zero, the loop will exit. 
      // Note: The jump instructions take offsets relative to the end of the bytescode for the jump. 
      // (So the jump_ne() method takes those values). This however is a bit hard to figure out here, 
      // so I've added the jump_ne_to() variant that takes a location rather than an offset. Internally,
      // it just figures out the offset, and generates the correct byte code).
      a->jump_ne_to(loop_start);
    }

    // having summed up 10 values, store thee result the first row of the array data.
    a->movaps(vpu::RCX, vpu::YMM0);

    // don't forget to return!
    a->ret();

  // done assembling
  a->end();

  // print code, execute, and print results
  print_machine_code("04_simple_loop", a);
  a->execute(argument_data);
  print_args(argument_data, sizeof(argument_data) / (sizeof(float) * 8));

  a->release();
}
