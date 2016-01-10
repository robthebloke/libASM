#include "examples.h"
#include <math.h>

// Unsure as to why, but only single or zero arg funcs are working :(  (works in release builds though?)
// More than one arg seems to screw up the RBP register (and therefore RCX/RDX get screwed up on return)

void example07()
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
    { 1.6f, 1.6f, 1.6f, 1.6f, 1.6f, 1.6f, 1.6f, 1.6f },   // RCX + 544
    { 1.7f, 1.7f, 1.7f, 1.7f, 1.7f, 1.7f, 1.7f, 1.7f },   // RCX + 576
    { 1.8f, 1.8f, 1.8f, 1.8f, 1.8f, 1.8f, 1.8f, 1.8f },   // RCX + 608
    { 1.9f, 1.9f, 1.9f, 1.9f, 1.9f, 1.9f, 1.9f, 1.9f },   // RCX + 640
  }
  VPU_ALIGN_SUFFIX(32);


  vpu::IFunctionTable* functions = g_lib->createFunctionTable();
  functions->add_defaults();

  // our assembler
  vpu::IAssembler* a = g_lib->createAssembler();

  // start assembling
  a->begin();
  
    // I'm going to save RBP, just incase it gets modified by the function call we make. 
    a->push(vpu::RBP);

    // Now then, for reasons unknown, the stack is actually the wrong way around. We need to subtract from the stack pointer,
    // rather than add (odd yes, but there we go). So let's move the stack pointer back by the amount we need. 
    a->sub(vpu::RSP, 64);

    // Store the modified stack location (RSP) in the base pointer (RBP). 
    a->lea(vpu::RBP, vpu::RSP, 32);

    // prior to any function call, preserve the RCX, and RDX registers somewhere in a stack location 
    // These register values are volatile, so we need to restore them after any function call we make. 
    // You should also store any other registers you want to keep track of as well (they may be written over)
    a->mov64(vpu::RBP, 8, vpu::RCX);
    a->mov64(vpu::RBP, 16, vpu::RDX);

    //----------------------------------------------------------------------------
    // call sin()
    //----------------------------------------------------------------------------
    {
      // move 1 value into YMM0. 
      a->movaps(vpu::YMM0, vpu::RCX, 32);

      // call the function!
      a->call("sin", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 0, vpu::YMM0);

    printf("sin(%f) %f\n", argument_data[1][0], sin(argument_data[1][0]));

    //----------------------------------------------------------------------------
    // call cos()
    //----------------------------------------------------------------------------
    {
      // move 1 value into YMM0. 
      a->movaps(vpu::YMM0, vpu::RCX, 64);

      // call the function!
      a->call("cos", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 32, vpu::YMM0);

    printf("cos(%f) %f\n", argument_data[2][0], cos(argument_data[2][0]));

    //----------------------------------------------------------------------------
    // call tan()
    //----------------------------------------------------------------------------
    {
      // move 1 value into YMM0. 
      a->movaps(vpu::YMM0, vpu::RCX, 96);

      // call the function!
      a->call("tan", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 64, vpu::YMM0);

    printf("tan(%f) %f\n", argument_data[3][0], tan(argument_data[3][0]));
  
    //----------------------------------------------------------------------------
    // call exp()
    //----------------------------------------------------------------------------
    {
      // move 1 value into YMM0. 
      a->movaps(vpu::YMM0, vpu::RCX, 128);

      // call the function!
      a->call("exp", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 96, vpu::YMM0);

    printf("exp(%f) %f\n", argument_data[4][0], exp(argument_data[4][0]));

    //----------------------------------------------------------------------------
    // call log2()
    //----------------------------------------------------------------------------
    {
      // move 1 value into YMM0. 
      a->movaps(vpu::YMM0, vpu::RCX, 160);

      // call the function!
      a->call("log2", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 128, vpu::YMM0);

    printf("log2(%f) %f\n", argument_data[5][0], log2(argument_data[5][0]));
  
    //----------------------------------------------------------------------------
    // call asin()
    //----------------------------------------------------------------------------
    {
      // move 1 value into YMM0. 
      a->movaps(vpu::YMM0, vpu::RCX, 192);

      // call the function!
      a->call("asin", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 160, vpu::YMM0);

    printf("asin(%f) %f\n", argument_data[6][0], asin(argument_data[6][0]));

    //----------------------------------------------------------------------------
    // call acos()
    //----------------------------------------------------------------------------
    {
      // move 1 value into YMM0. 
      a->movaps(vpu::YMM0, vpu::RCX, 224);

      // call the function!
      a->call("acos", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 192, vpu::YMM0);

    printf("acos(%f) %f\n", argument_data[7][0], acos(argument_data[7][0]));

    //----------------------------------------------------------------------------
    // call atan()
    //----------------------------------------------------------------------------
    {
      // move 1 value into YMM0. 
      a->movaps(vpu::YMM0, vpu::RCX, 256);

      // call the function!
      a->call("atan", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 224, vpu::YMM0);

    printf("atan(%f) %f\n", argument_data[8][0], atan(argument_data[8][0]));

    //----------------------------------------------------------------------------
    // call pow2()
    //----------------------------------------------------------------------------
    {
      // move 1 value into YMM0. 
      a->movaps(vpu::YMM0, vpu::RCX, 288);

      // call the function!
      a->call("pow2", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 256, vpu::YMM0);

    printf("pow2(%f) %f\n", argument_data[9][0], pow(2.0f, argument_data[9][0]));

    //----------------------------------------------------------------------------
    // call sinh()
    //----------------------------------------------------------------------------
    {
      // move 1 value into YMM0. 
      a->movaps(vpu::YMM0, vpu::RCX, 320);

      // call the function!
      a->call("sinh", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 288, vpu::YMM0);

    printf("sinh(%f) %f\n", argument_data[10][0], sinh(argument_data[10][0]));

    //----------------------------------------------------------------------------
    // call cosh()
    //----------------------------------------------------------------------------
    {
      // move 1 value into YMM0. 
      a->movaps(vpu::YMM0, vpu::RCX, 352);

      // call the function!
      a->call("cosh", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 320, vpu::YMM0);

    printf("cosh(%f) %f\n", argument_data[11][0], cosh(argument_data[11][0]));

    //----------------------------------------------------------------------------
    // call tanh()
    //----------------------------------------------------------------------------
    {
      // move 1 value into YMM0. 
      a->movaps(vpu::YMM0, vpu::RCX, 384);

      // call the function!
      a->call("tanh", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 352, vpu::YMM0);

    printf("tanh(%f) %f\n", argument_data[12][0], tanh(argument_data[12][0]));

    //----------------------------------------------------------------------------
    // call asinh()
    //----------------------------------------------------------------------------
    {
      // move 1 value into YMM0. 
      a->movaps(vpu::YMM0, vpu::RCX, 416);

      // call the function!
      a->call("asinh", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 384, vpu::YMM0);

    printf("asinh(%f) %f\n", argument_data[13][0], asinh(argument_data[13][0]));

    //----------------------------------------------------------------------------
    // call acosh()
    //----------------------------------------------------------------------------
    {
      // move 1 value into YMM0. 
      a->movaps(vpu::YMM0, vpu::RCX, 448);

      // call the function!
      a->call("acosh", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 416, vpu::YMM0);

    printf("acosh(%f) %f\n", argument_data[14][0], acosh(argument_data[14][0]));

    //----------------------------------------------------------------------------
    // call atanh()
    //----------------------------------------------------------------------------
    {
      // move 1 value into YMM0. 
      a->movaps(vpu::YMM0, vpu::RCX, 480);

      // call the function!
      a->call("atanh", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 448, vpu::YMM0);

    printf("atanh(%f) %f\n", argument_data[15][0], atanh(argument_data[15][0]));

    //----------------------------------------------------------------------------
    // call cbrt()
    //----------------------------------------------------------------------------
    {
      // move 1 value into YMM0. 
      a->movaps(vpu::YMM0, vpu::RCX, 512);

      // call the function!
      a->call("cbrt", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 480, vpu::YMM0);

    printf("cbrt(%f) %f\n", argument_data[16][0], cbrt(argument_data[16][0]));

    //----------------------------------------------------------------------------
    //----------------------------------------------------------------------------
    // Funcs that take two arguments, load into YMM0 and YMM1.
    //----------------------------------------------------------------------------
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    // call pow()
    //----------------------------------------------------------------------------
    {
      // move 1 value into YMM0. 
      a->movaps(vpu::YMM0, vpu::RCX, 544);
      a->movaps(vpu::YMM1, vpu::RCX, 576);
      
      // call the function!
      a->call("pow", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 512, vpu::YMM0);

    printf("pow(%f, %f) %f\n", argument_data[17][0], argument_data[18][0], pow(argument_data[17][0], argument_data[18][0]));

    //----------------------------------------------------------------------------
    // call atan2()
    //----------------------------------------------------------------------------
    {
      // move 1 value into YMM0. 
      a->movaps(vpu::YMM0, vpu::RCX, 576);
      a->movaps(vpu::YMM1, vpu::RCX, 608);

      // call the function!
      a->call("atan2", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 544, vpu::YMM0);

    printf("atan2(%f, %f) %f\n", argument_data[18][0], argument_data[19][0], atan2(argument_data[18][0], argument_data[19][0]));
  
    // restore our stack pointer
    a->add(vpu::RSP, 64);
    a->pop(vpu::RBP);

    // don't forget to return!
    a->ret();

  // done assembling
  a->end();

  // print code, execute, and print results
  print_machine_code("07_calling_a_function", a);
  a->execute(argument_data, functions);
  print_args(argument_data, sizeof(argument_data) / (sizeof(float) * 8));

  a->release();
  functions->release();
}
