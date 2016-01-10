#include "examples.h"

void example03()
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

    // Thie example will treat the first 3 rows of the input array as 8x Vector3's (in SOA format).
    // Those 3 rows will be loaded into memory. 
    // We shall compute the length of each vector. 
    // and then normalise each vector by multiplying by 1/length. 
    // Finally, write the result back out. 
    // This is LITERALLY the fastest way to normalise a vec3 on an Intel/AMD CPU.

    // Ok, so I'm going to load 3 rows into memory. (which I will assume are 8 x Vector3, e.g. 8X, 8Y, 8Z)
    a->movaps(vpu::YMM0, vpu::RCX, 0);
    a->movaps(vpu::YMM1, vpu::RCX, 32);
    a->movaps(vpu::YMM2, vpu::RCX, 64);

    // square each value (and store in tempraries)
    a->mulps(vpu::YMM3, vpu::YMM0, vpu::YMM0);
    a->mulps(vpu::YMM4, vpu::YMM1, vpu::YMM1);
    a->mulps(vpu::YMM5, vpu::YMM2, vpu::YMM2);

    // YMM3 = YMM3 + YMM4 + YMM5
    a->addps(vpu::YMM4, vpu::YMM4, vpu::YMM5);
    a->addps(vpu::YMM3, vpu::YMM3, vpu::YMM4);

    // now compute  1 / sqrt(YMM3)
    a->rsqrtps(vpu::YMM3, vpu::YMM3);

    // divide YMM0 -> YMM2 through by the length (using multiply, because it's quicker than divide!)
    a->mulps(vpu::YMM0, vpu::YMM0, vpu::YMM3);
    a->mulps(vpu::YMM1, vpu::YMM1, vpu::YMM3);
    a->mulps(vpu::YMM2, vpu::YMM2, vpu::YMM3);

    // move the vector3 values back out to our float array above
    a->movaps(vpu::RCX, vpu::YMM0);
    a->movaps(vpu::RCX, 32, vpu::YMM1);
    a->movaps(vpu::RCX, 64, vpu::YMM2);

    // don't forget to return!
    a->ret();

  // done assembling
  a->end();

  // print code, execute, and print results
  print_machine_code("03_normalise_vec3", a);
  a->execute(argument_data);
  print_args(argument_data, sizeof(argument_data) / (sizeof(float) * 8));

  a->release();
}
