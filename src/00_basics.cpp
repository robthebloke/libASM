#include "examples.h"

// YMM registers
//--------------
//
// In total, there are 16 YMM registers you have available. 
// Each YMM register is 32bytes in size
//  - that means it can hold 8 floating point numbers  (32 / sizeof(float) == 8)
//
// The YMM registers are name YMM0, YMM1, YMM2, etc. 
// If possible, try to use YMM0 -> YMM8 only. Whilst you can use the higher registers if needed, 
// they tend to produce slightly longer machine code in some cases. 
//
// X64 registers
//--------------
// 
// These are general purpose registers, although some are reserved for specific uses. You can modify any of these 
// for any purpose you wish, with the exception of RCX and RSI. RSP and RBP need to be used carefully! If you are 
// using function maps, then avoid modifying RDX. All the others are fair game. 
//
//  RAX  - typically used for array access (use as you wish)
//  RBX  - general purpose
//  RCX  - first function argument (avoid)
//  RDX  - second function argument
//  RSP  - used to keep track of the stack (must be used properly!)
//  RBP  - used to keep track of where the function must return to. (must be used properly!)
//  RSI  - the current instruction (avoid)
//  RDI  - can't remember
//  R8   - general purpose (the third function argument)
//  R9   - general purpose (the fourth function argument)
//  R10  - general purpose
//  R11  - general purpose
//  R12  - general purpose
//  R13  - general purpose
//  R14  - general purpose
//  R15  - general purpose
//
// Generally speaking, the general purpose registers are used to control looping, if/else, and to store memory locations.
//

void example00()
{
  // This data will be available within your assembly code. 
  // Each row is 32bytes in size (the same size as a YMM register).
  // When your assembly is executed, the address of the array will be loaded into the RCX register.
  // I've written the addresses you'll need to remember down the right hand side
  VPU_ALIGN_PREFIX(32)
  float argument_data[][8] = 
  {
    { 0.0f,  0.0f,  0.0f,  0.0f,    0.0f,  0.0f,  0.0f,  0.0f },   // RCX
    { 0.1f,  0.1f,  0.1f,  0.1f,    0.1f,  0.1f,  0.1f,  0.1f },   // RCX + 32
    { 0.2f,  0.2f,  0.2f,  0.2f,    0.2f,  0.2f,  0.2f,  0.2f },   // RCX + 64
    { 0.3f,  0.3f,  0.3f,  0.3f,    0.3f,  0.3f,  0.3f,  0.3f },   // RCX + 96
    { 0.4f,  0.4f,  0.4f,  0.4f,    0.4f,  0.4f,  0.4f,  0.4f },   // RCX + 128
    { 0.5f,  0.5f,  0.5f,  0.5f,    0.5f,  0.5f,  0.5f,  0.5f },   // RCX + 160
    { 0.6f,  0.6f,  0.6f,  0.6f,    0.6f,  0.6f,  0.6f,  0.6f },   // RCX + 192
    { 0.7f,  0.7f,  0.7f,  0.7f,    0.7f,  0.7f,  0.7f,  0.7f },   // RCX + 224
    { 0.8f,  0.8f,  0.8f,  0.8f,    0.8f,  0.8f,  0.8f,  0.8f },   // RCX + 256
    { 0.9f,  0.9f,  0.9f,  0.9f,    0.9f,  0.9f,  0.9f,  0.9f },   // RCX + 288
    { 1.0f,  1.0f,  1.0f,  1.0f,    1.0f,  1.0f,  1.0f,  1.0f },   // RCX + 320
    { 1.1f,  1.1f,  1.1f,  1.1f,    1.1f,  1.1f,  1.1f,  1.1f },   // RCX + 352
    { 1.2f,  1.2f,  1.2f,  1.2f,    1.2f,  1.2f,  1.2f,  1.2f },   // RCX + 384
    { 1.3f,  1.3f,  1.3f,  1.3f,    1.3f,  1.3f,  1.3f,  1.3f },   // RCX + 416
    { 1.4f,  1.4f,  1.4f,  1.4f,    1.4f,  1.4f,  1.4f,  1.4f },   // RCX + 480
    { 1.5f,  1.5f,  1.5f,  1.5f,    1.5f,  1.5f,  1.5f,  1.5f },   // RCX + 512
  }
  VPU_ALIGN_SUFFIX(32);
  
  // our assembler
  vpu::IAssembler* a = g_lib->createAssembler();

  // start assembling
  a->begin();

    // first we need to move some data from the array above, into a YMM register.
    // Let's move the second row into the YMM0 register. (RCX stores the address of the argument_data array)
    // note: 32, because sizeof(YMM) is 32 bytes!
    a->movaps(vpu::YMM0, vpu::RCX, 32);

    // The movaps is an aligned move (so the memory location must be aligned to 32 bytes). 
    // If we want, we can also use an unaligned move.
    // Here we will move 4x 0.2 values, and 4x 0.3 values into YMM1. 
    a->movups(vpu::YMM1, vpu::RCX, 80);

    // And to go from registers back into memory, we reverse the movaps/movups direction. 
    // move YMM0 to the start of the array
    a->movaps(vpu::RCX, 0, vpu::YMM0);

    // Now again used the unaligned moves, to start writing from argument_data[2]
    a->movups(vpu::RCX, 8, vpu::YMM1);

    // We must always include a call return at the end of our code! 
    // (otherwise the processor will just keep executing beyond the end of our array!)
    a->ret(); 

  // done assembling
  a->end();

  // write out the assembled machine code, just incase we crash and need to debug. 
  print_machine_code("00_basics", a);

  // Cool! function has been assebled, so let's execute this bad boy, and see what we get!
  a->execute(argument_data);

  // now output what we ended up with in the array above
  print_args(argument_data, sizeof(argument_data) / (sizeof(float) * 8));

  a->release();
}
