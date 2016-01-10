#include "examples.h"

// Here are a couple of pointless functions we want to register.
// Ensure the methods are using the vectorcall calling convention.
// For some reason, only zero or one arg methods are working :(

__m256 __vectorcall func0()
{
  return _mm256_set1_ps(69.0f);
}

__m256 __vectorcall func1(__m256 a)
{
  return _mm256_sqrt_ps(a);
}

__m256 __vectorcall func2(__m256 a, __m256 b)
{
  return _mm256_add_ps(a, b);
}

void example08()
{
  VPU_ALIGN_PREFIX(32)
  float argument_data[][8] =
  {
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },   // RCX
    { 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f },   // RCX + 32
    { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },   // RCX + 64
    { 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f },   // RCX + 96
  }
  VPU_ALIGN_SUFFIX(32);

  // register our custom function with the function map
  vpu::IFunctionTable* functions = g_lib->createFunctionTable();
  functions->addFunc("func0", func0);
  functions->addFunc("func1", func1);
  functions->addFunc("func2", func2);

  // our assembler
  vpu::IAssembler* a = g_lib->createAssembler();

  // start assembling
  a->begin();

    // standard function pre-amble
    a->push(vpu::RBP);
    a->sub(vpu::RSP, 64);
    a->lea(vpu::RBP, vpu::RSP, 32);

    // we will need to restore RCX and RDX after function calls
    a->mov64(vpu::RBP, 8, vpu::RCX);
    a->mov64(vpu::RBP, 16, vpu::RDX);

    //----------------------------------------------------------------------------
    // call the custom function
    //----------------------------------------------------------------------------
    {
      // call our function... (with no args)
      a->call("func0", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 0, vpu::YMM0);

    //----------------------------------------------------------------------------
    // call the custom function
    //----------------------------------------------------------------------------
    {
      // load argument into YMM0
      a->movaps(vpu::YMM0, vpu::RCX, 32);

      // call our function... (with no args)
      a->call("func1", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 32, vpu::YMM0);

    //----------------------------------------------------------------------------
    // call the custom function
    //----------------------------------------------------------------------------
    {
      // load argument into YMM0
      a->movaps(vpu::YMM0, vpu::RCX, 64);
      a->movaps(vpu::YMM1, vpu::RCX, 96);

      // call our function... (with no args)
      a->call("func2", functions);

      // after each call, restore the RCX/RDX registers.
      a->mov64(vpu::RCX, vpu::RBP, 8);
      a->mov64(vpu::RDX, vpu::RBP, 16);
    }

    // the result should now be in YMM0, so write it back out to RCX[0]
    a->movaps(vpu::RCX, 64, vpu::YMM0);

    // restore our stack pointer
    a->add(vpu::RSP, 64);
    a->pop(vpu::RBP);

    // don't forget to return!
    a->ret();

  // done assembling
  a->end();


  // print code, execute, and print results
  print_machine_code("08_custom_functions", a);
  a->execute(argument_data, functions);
  print_args(argument_data, sizeof(argument_data) / (sizeof(float) * 8));

  a->release();
  functions->release();
}
