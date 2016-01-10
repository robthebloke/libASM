# libASM 
## a small lightweight library for runtime assembly on AVX2/x64 instructions
------------------------

### First, let's explain what this library is NOT:

* This is not a full blown Intel Assembler (use something like NASM if that's what you need)
* This library only supports AVX2 instructions that use YMM registers. There is no support for XMM (with exception of single value instructions, e.g. ADDSS)
* This library does not provide support for assembling PE executable files.

### So what does this library do?

* Generates machine code that can be called via a function pointer in C++.
* Assembles (most) AVX2 instructions that use YMM registers, and (most) AVX2 instructions tha work on single values (e.g. ADDSS, MULSD, etc).
* Some of the simpler x64 instructions are also supported (e.g. LEA, MOV, CALL, CMP, etc).
* Provides some minimal support for defining custom procedures (calling convention is up to you).
* Provides some minimal support for calling C++ functions (via fastcall, but the procedures are limited)
* Provides some maths approximations for computing sin, cos, tan, etc (for 8xfloat, 4xdouble). [Accuracy of these methods does not come close to the C standard library. They are cheap approximations!]

### How does this library work?

In effect, this library will assemble a C function whose function prototype is:

  <code>void function(void* RCX_data, void** RDX_function_table);</code>

A single memory block (I'd recommend aligning this to 32bytes) can be used to provide upto 4Gb of arguments, or returned data. The function table is an optional array of function pointers that can call into some predefined C++ functions. The assembled code will be generated within an executable memory page (allocated via VirtualAllocEx), which is then called by casting the memory block to a function pointer, before being called.

##Are there any bugs?

I've tested this library extensively, and from what I can tell, all instructions are generated correctly. I'm reasonably confident you wont hit any bugs, however I'm also experienced enough to know that it's always possible that a few have slipped through my unit tests. I take no responsibility for how this library is used.

##Can I get the source code?

It's first worth considering my aims in building this lib:

1. I wanted it to be leightweight (i.e. less than 64Kb, although I'm currently at 65Kb).
2. I wanted it to be in an entirely self contained DLL with no dependencies.
3. To generate executable AVX2 instructions for another project I'm working on.
4. I wanted to have some fun.

The first two requirements make the code a little bit gnaarly in places (It's very easy to break the build). The 3rd and 4th means that this library is not my main aim, and is merely a stepping stone. I don't really have the time to sanitise the code for public release at this time. I may do in future, but please don't ask me to!

##Can I get a version for linux/Mac ?

I will eventually get around to building a linux version (don't ask for an ETA), but a mac version is extremely unlikely (because I don't have access to a mac!).

#Using the library
-----------------

There are 2 main components to this library:

* assembler_dll.h  -  the header file you'll need to include.
* libASM.dll  - the library itself.

Note: There is no import library for libASM.dll. The dll is always loaded dynamically via LoadLibrary.


##Initialising the library
------------------------

The class vpu::AssemblerLib performs all of the intialisation needed to load the library. A minimal example that loads and unloads the dll would be:

<pre><code>
#include "assembler_dll.h"

vpu::AssemblerLib* g_lib = 0;

int main()
{
  // load the dll from disk, and extract all dll entry points.
  g_lib = new vpu::AssemblerLib("libASM.dll");

  // test to see whether the library intialised.
  if (g_lib->isOk())
  {
    // You can now create an assembler, and begin executing code.
  }

  // unload the DLL
  delete g_lib;

  // done
  return 0;
}
</code></pre>

### Generating some machine code
---------------------------

So let's use the assembler to generate the worlds simplest function (one that simply returns!)

<pre><code>
#include "assembler_dll.h"

vpu::AssemblerLib* g_lib = 0;

int main()
{
  g_lib = new vpu::AssemblerLib("libASM.dll");
  if (g_lib->isOk())
  {
    // create an assembler to use
    vpu::IAssembler* assembler = g_lib->createAssembler();

    // start assembling
    assembler->begin();

      // Always ensure you have a ret() instruction at the end of your code!
      // If you omit this, the processor will just execute a load of random instructions.
      // That would be very bad indeed!
      assembler->ret();

    // done assembling
    assembler->end();

    // Now execute the instructions
    assembler->execute(0);

    // done with the assembler, so release it.
    assembler->release();
  }
  delete g_lib;
  return 0;
}
</code></pre>

## Working on some data...
-----------------

<pre><code>
#include "assembler_dll.h"

vpu::AssemblerLib* g_lib = 0;

int main()
{
  g_lib = new vpu::AssemblerLib("libASM.dll");
  if (g_lib->isOk())
  {
    // create an assembler to use
    vpu::IAssembler* assembler = g_lib->createAssembler();

    // start assembling
    assembler->begin();

      // load the first 8 floats into YMM0 (data is always accessed via RCX register).
      assembler->movaps(vpu::YMM0, vpu::RCX, 0);

      // add YMM0 to 8 more floats found at (RCX + 32), store the result in YMM0.
      assembler->addps(vpu::YMM0, vpu::YMM0, vpu::RCX, 32);

      // move the result from YMM0 to the first 8 floats.
      assembler->movaps(vpu::RCX, 0, vpu::YMM0);

      // return!
      assembler->ret();

    // done assembling
    assembler->end();

    // this is the data array we will pass to the function when it's being called. This can be accessed via the RCX register.
    VPU_ALIGN_PREFIX(32)
    float argument_data[][8] =
    {
      { 0.2f,  0.2f,  0.2f,  0.2f,    0.2f,  0.2f,  0.2f,  0.2f },   // RCX
      { 0.1f,  0.1f,  0.1f,  0.1f,    0.1f,  0.1f,  0.1f,  0.1f }    // RCX + 32
    }
    VPU_ALIGN_SUFFIX(32);

    // Now execute our code (passing in the argument_data to process).
    // The first 8 values in argument_data should be 0.3 after execution.
    assembler->execute(argument_data);

    // done with the assembler, so release it.
    assembler->release();
  }
  delete g_lib;
  return 0;
}
</code></pre>

That's a basic introduction. The library does support a few more interesting things (such as function calls, custom procedures, etc). The best place to look for information on how they work, is within the code examples.
