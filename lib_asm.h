/// \file   lib_asm.h
/// \author Rob Bateman
/// \date   Novemeber 2015
/// \brief  Defines an interface that allows you to runtime assemble AVX2 instructions - mainly those methods concerned with YMM registers. 
///         By no measure is this a feature complete assembler, but it's a lightweight assembler that might be useful to someone. 
///         Obviously, you should really ensure that the computer you are running this on actually supports AVX2. 
/// \note   Copyright Rob Bateman. I accept no liability for any damage done to you, your computer(s), your client(s), or any other hardware/software 
///         problem that may arise from using this software. Use at your own risk.

#pragma once
#include <cstdint>
#include <immintrin.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace vpu
{
#if defined(_WIN32)
# define VPU_ALIGN_PREFIX(X) __declspec(align(X))
# define VPU_ALIGN_SUFFIX(X)
#else
# define VPU_ALIGN_PREFIX(X)
# define VPU_ALIGN_SUFFIX(X) __attribute__ ((aligned (X)))
#endif

// forward decls
struct IFunctionTable;
struct IAssembler;

// DLL API
extern "C"
{
typedef IFunctionTable* (*CreateFunctionTableFn)();
typedef IAssembler* (*CreateAssemblerFn)(size_t);
typedef void (*InitLibFn)(void**);
}

/// \brief  An enum describing the prototype for a function
enum FunctionType 
{
  kNoArgs,
  kOneArg,
  kTwoArgs,
  kThreeArgs,
  kFourArgs,
  kFiveArgs,
  kNoArgsD,
  kOneArgD,
  kTwoArgsD,
  kThreeArgsD,
  kFourArgsD,
  kFiveArgsD
};

/// These are the only allowable prototypes for supported __vectorcall functions. 
/// Arguments are passed in using YMM0 -> YMM4. 
/// The return argument will be within YMM0. If you need to mix int/float/double args, then just cast the argument.
/// \code
/// __m256 __vectorcall some_func(__m256 a, __m256 b, __m256 c)
/// {
///   __m256i ia = _mm256_castps_si256(a) = 0;
///   __m256i ib = _mm256_castps_pd(b) = 0;
///  return c;
/// }
/// \endcode
/// The assembler is not going to validate what type of data you have loaded in the YMM registers, it willjust make the call.
/// So this mechanism can be abused somewhat (and this also means typesafety is left up to you).
typedef __m256 (__vectorcall *func_no_args_f)();
typedef __m256 (__vectorcall *func_one_arg_f)(__m256);
typedef __m256 (__vectorcall *func_two_args_f)(__m256, __m256);
typedef __m256 (__vectorcall *func_three_args_f)(__m256, __m256, __m256);
typedef __m256 (__vectorcall *func_four_args_f)(__m256, __m256, __m256, __m256);
typedef __m256 (__vectorcall *func_five_args_f)(__m256, __m256, __m256, __m256, __m256);

typedef __m256d (__vectorcall *func_no_args_d)();
typedef __m256d (__vectorcall *func_one_arg_d)(__m256d);
typedef __m256d (__vectorcall *func_two_args_d)(__m256d, __m256d);
typedef __m256d (__vectorcall *func_three_args_d)(__m256d, __m256d, __m256d);
typedef __m256d (__vectorcall *func_four_args_d)(__m256d, __m256d, __m256d, __m256d);
typedef __m256d (__vectorcall *func_five_args_d)(__m256d, __m256d, __m256d, __m256d, __m256d);

/// \brief  A class that stores a table of functions that can be called by the assembled code. 
struct IFunctionTable
{
protected:
  IFunctionTable(){}
  virtual ~IFunctionTable(){}
public:

  virtual void release() = 0;

  /// \brief  Adds a default set of (fairly bad) approximations to:
  ///         __m256 abs(__m256)
  ///         __m256 sin(__m256)
  ///         __m256 cos(__m256)
  ///         __m256 tan(__m256)
  ///         __m256 sinh(__m256)
  ///         __m256 cosh(__m256)
  ///         __m256 tanh(__m256)
  ///         __m256 asin(__m256)
  ///         __m256 acos(__m256)
  ///         __m256 atan(__m256)
  ///         __m256 atan2(__m256, __m256)
  ///         __m256 asinh(__m256)
  ///         __m256 acosh(__m256)
  ///         __m256 atanh(__m256)
  ///         __m256 exp(__m256)
  ///         __m256 log2(__m256)
  ///         __m256 log(__m256)
  ///         __m256 pow2(__m256)
  ///         __m256 pow(__m256, __m256)
  ///         __m256 cbrt(__m256)
  ///         __m256d absd(__m256d)
  ///         __m256d sind(__m256d)
  ///         __m256d cosd(__m256d)
  ///         __m256d tand(__m256d)
  ///         __m256d sinhd(__m256d)
  ///         __m256d coshd(__m256d)
  ///         __m256d tanhd(__m256d)
  ///         __m256d asind(__m256d)
  ///         __m256d acosd(__m256d)
  ///         __m256d atand(__m256d)
  ///         __m256d atan2d(__m256d, __m256d)
  ///         __m256d asinhd(__m256d)
  ///         __m256d acoshd(__m256d)
  ///         __m256d atanhd(__m256d)
  ///         __m256d expd(__m256d)
  ///         __m256d log2d(__m256d)
  ///         __m256d logd(__m256d)
  ///         __m256d pow2d(__m256d)
  ///         __m256d powd(__m256d, __m256d)
  ///         __m256d cbrtd(__m256d)
  virtual void add_defaults() = 0;

  /// \name   addFunc
  /// \brief  Allows you to add your own custom functions into the table. 
  ///         Only __vectorcall functions are supported.
  ///         The name must be unique for each function - funtion overloading is not supported.
  inline void addFunc(const char* name, func_no_args_f fn) { _addFunc(name, fn, kNoArgs); }
  inline void addFunc(const char* name, func_one_arg_f fn) { _addFunc(name, fn, kOneArg); }
  inline void addFunc(const char* name, func_two_args_f fn) { _addFunc(name, fn, kTwoArgs); }
  inline void addFunc(const char* name, func_three_args_f fn) { _addFunc(name, fn, kThreeArgs); }
  inline void addFunc(const char* name, func_four_args_f fn) { _addFunc(name, fn, kFourArgs); }
  inline void addFunc(const char* name, func_five_args_f fn) { _addFunc(name, fn, kFiveArgs); }
  inline void addFunc(const char* name, func_no_args_d fn) { _addFunc(name, fn, kNoArgsD); }
  inline void addFunc(const char* name, func_one_arg_d fn) { _addFunc(name, fn, kOneArgD); }
  inline void addFunc(const char* name, func_two_args_d fn) { _addFunc(name, fn, kTwoArgsD); }
  inline void addFunc(const char* name, func_three_args_d fn) { _addFunc(name, fn, kThreeArgsD); }
  inline void addFunc(const char* name, func_four_args_d fn) { _addFunc(name, fn, kFourArgsD); }
  inline void addFunc(const char* name, func_five_args_d fn) { _addFunc(name, fn, kFiveArgsD); }

  /// \brief  Query the type of function for a given name
  /// \param  name the name of the function to query
  /// \param  type the returned type of the function
  /// \return the registerd function index.
  virtual int32_t funcInfo(const char* name, FunctionType& type) const = 0;
private:
  virtual void _addFunc(const char* name, void* fn, FunctionType type) = 0;
};

/// there are 16 x 256bit YMM registers available (data type __m256).
/// Effectively 8 x float, or 8 x int32. 
enum AVXReg : uint8_t
{
  YMM0,
  YMM1,
  YMM2,
  YMM3,
  YMM4,
  YMM5,
  YMM6,
  YMM7,
  YMM8,
  YMM9,
  YMMA,
  YMMB,
  YMMC,
  YMMD,
  YMME,
  YMMF
};

/// 16 general purpose registers
enum Reg : uint8_t
{
  RAX,
  RCX, ///< Note, used for the 'data' argument
  RDX, ///< Note, used for the 'function table' argument
  RBX,
  RSP, ///< stack
  RBP, ///< base
  RSI, 
  RDI, 
  R8,  ///< Note, used for the 'extra' function argument
  R9,
  R10,
  R11,
  R12,
  R13,
  R14,
  R15,
};

/// Floating point comparison modes
enum cmp : uint8_t
{
  EQ_OQ     = 0x00,  /* Equal (ordered, nonsignaling)               */
  LT_OS     = 0x01,  /* Less-than (ordered, signaling)              */
  LE_OS     = 0x02,  /* Less-than-or-equal (ordered, signaling)     */
  UNORD_Q   = 0x03,  /* Unordered (nonsignaling)                    */
  NEQ_UQ    = 0x04,  /* Not-equal (unordered, nonsignaling)         */
  NLT_US    = 0x05,  /* Not-less-than (unordered, signaling)        */
  NLE_US    = 0x06,  /* Not-less-than-or-equal (unordered, signaling)          */
  ORD_Q     = 0x07,  /* Ordered (nonsignaling)                      */
  EQ_UQ     = 0x08,  /* Equal (unordered, non-signaling)            */
  NGE_US    = 0x09,  /* Not-greater-than-or-equal (unordered, signaling)       */
  NGT_US    = 0x0A,  /* Not-greater-than (unordered, signaling)     */
  FALSE_OQ  = 0x0B,  /* False (ordered, nonsignaling)               */
  NEQ_OQ    = 0x0C,  /* Not-equal (ordered, non-signaling)          */
  GE_OS     = 0x0D,  /* Greater-than-or-equal (ordered, signaling)  */
  GT_OS     = 0x0E,  /* Greater-than (ordered, signaling)           */
  TRUE_UQ   = 0x0F,  /* True (unordered, non-signaling)             */
  EQ_OS     = 0x10,  /* Equal (ordered, signaling)                  */
  LT_OQ     = 0x11,  /* Less-than (ordered, nonsignaling)           */
  LE_OQ     = 0x12,  /* Less-than-or-equal (ordered, nonsignaling)  */
  UNORD_S   = 0x13,  /* Unordered (signaling)                       */
  NEQ_US    = 0x14,  /* Not-equal (unordered, signaling)            */
  NLT_UQ    = 0x15,  /* Not-less-than (unordered, nonsignaling)     */
  NLE_UQ    = 0x16,  /* Not-less-than-or-equal (unordered, nonsignaling)       */
  ORD_S     = 0x17,  /* Ordered (signaling)                         */
  EQ_US     = 0x18,  /* Equal (unordered, signaling)                */
  NGE_UQ    = 0x19,  /* Not-greater-than-or-equal (unordered, nonsignaling)    */
  NGT_UQ    = 0x1A,  /* Not-greater-than (unordered, nonsignaling)  */
  FALSE_OS  = 0x1B,  /* False (ordered, signaling)                  */
  NEQ_OS    = 0x1C,  /* Not-equal (ordered, signaling)              */
  GE_OQ     = 0x1D,  /* Greater-than-or-equal (ordered, nonsignaling)        */
  GT_OQ     = 0x1E,  /* Greater-than (ordered, nonsignaling)        */
  TRUE_US   = 0x1F,  /* True (unordered, signaling)                 */
};

/// The rounding mode 
enum RoundMode : uint8_t
{
  FROUND_TO_NEAREST_INT = 0x00,
  FROUND_TO_NEG_INF = 0x01,
  FROUND_TO_POS_INF = 0x02,
  FROUND_TO_ZERO = 0x03,
  FROUND_CUR_DIRECTION = 0x04,
  FROUND_RAISE_EXC = 0x00,
  FROUND_NO_EXC = 0x08,
  FROUND_NINT = FROUND_TO_NEAREST_INT | FROUND_RAISE_EXC,
  FROUND_FLOOR = FROUND_TO_NEG_INF | FROUND_RAISE_EXC,
  FROUND_CEIL = FROUND_TO_POS_INF | FROUND_RAISE_EXC,
  FROUND_TRUNC = FROUND_TO_ZERO | FROUND_RAISE_EXC,
  FROUND_RINT = FROUND_CUR_DIRECTION | FROUND_RAISE_EXC,
  FROUND_NEARBYINT = FROUND_CUR_DIRECTION | FROUND_NO_EXC
};

/// the assembled code will resemble a function matching this prototype
typedef void (*FuncProtoype)(void*, void**);

/// A class that provides the ability to run-time assemble AVX2 code. 
/// It always assumes you're wanting to use YMM registers. 
/// This class by no means covers all AVX2 instructions, just enough of them to be useful. 
struct IAssembler
{
protected:
  IAssembler() {}
  virtual ~IAssembler() {}
public:

  /// \brief  dtor
  virtual void release() = 0;

  /// \name   General Usage
  /// \brief  Ensure you call begin() prior to any assembling of code, and end() when you are finished. This will produce some
  ///         bytecode, which you can inspect if you really want. 

  /// \brief  make sure you call this before doing any assembling...
  ///         This will reset the assembler to take new input. Any previous assembled code or constants will be lost
  virtual void begin() = 0;

  /// \brief  make sure you do this before executing any compiled code.
  ///         This will resolve any references to constants.
  virtual void end() = 0;

  /// \brief  query total size of bytecode
  virtual size_t numBytes() const = 0;

  /// \brief  query byte code
  virtual const uint8_t* bytecode() const = 0;

  /// \brief  execute the bytecode. 
  /// \param  data this pointer will be loaded into RCX
  virtual void execute(void* data) = 0;
  
  /// \brief  execute the bytecode with a function table
  /// \param  data this pointer will be loaded into RCX
  /// \param  map this function table will be loaded into RDX. It should be the same table used when calling call().
  virtual void execute(void* data, const IFunctionTable* map) = 0;

  /// \brief  call the function from the function table
  /// \param  name of the function to call
  /// \param  the table of functions to use (This must be the same table you pass to execute). 
  virtual bool call(const char* name, const IFunctionTable* func_map) = 0;

  /// \name   Constant Values
  /// \brief  To use a constant value in your ASM, first create the constant value:
  /// \code
  /// uint32_t pi = assembler.set1_ps(3.14159f) = 0;
  /// \endcode
  /// Then load whenever needed, e.g.
  /// \code
  /// assembler.load_const(vpu::YMM0, pi) = 0;
  /// assembler.load_const(vpu::YMM1, pi) = 0;
  /// assembler.load_const(vpu::YMM2, pi) = 0;
  /// assembler.load_const(vpu::YMM3, pi) = 0;
  /// \endcode
  /// 

  /// broadcast float across an entire YMM register
  virtual uint32_t set1_ps(float value) = 0;

  /// broadcast double across an entire YMM register
  virtual uint32_t set1_pd(double value) = 0;

  /// broadcast 32bit int across an entire YMM register
  virtual uint32_t set1_epi32(int32_t value) = 0;

  /// set a YMM register from 8 floats
  virtual uint32_t set_ps(float a0, float a1, float a2, float a3, float a4, float a5, float a6, float a7) = 0;

  /// set a YMM register from 4 doubles
  virtual uint32_t set_pd(double a0, double a1, double a2, double a3) = 0;

  /// set a YMM register from 8 32bit integers
  virtual uint32_t set_epi32(int32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6, int32_t a7) = 0;

  /// load the constant at the given location (the value returned from set1_ps, set_pd, etc) into the target YMM register.
  virtual void load_const(AVXReg target, uint32_t location) = 0;
  

  // shift bytes left 
  // https://www.google.co.uk/#q=_mm256_sri_si128
  virtual void lshift_u128(AVXReg target, AVXReg a, uint8_t num_bytes) = 0;
  
  // shift bytes right
  // https://www.google.co.uk/#q=_mm256_sli_si128
  virtual void rshift_u128(AVXReg target, AVXReg a, uint8_t num_bytes) = 0;
  
  // left shift, shift in zeros 
  // https://www.google.co.uk/#q=_mm256_slli_epi16
  virtual void lshift_u16(AVXReg target, AVXReg a, uint8_t num_bits) = 0;
  
  // https://www.google.co.uk/#q=_mm256_slli_epi32
  virtual void lshift_u32(AVXReg target, AVXReg a, uint8_t num_bits) = 0;

  // https://www.google.co.uk/#q=_mm256_slli_epi64
  virtual void lshift_u64(AVXReg target, AVXReg a, uint8_t num_bits) = 0;


  // right shift, shift in zeros 
  // https://www.google.co.uk/#q=_mm256_srli_epi16
  virtual void rshift_u16(AVXReg target, AVXReg a, uint8_t num_bits) = 0;

  // https://www.google.co.uk/#q=_mm256_srli_epi32
  virtual void rshift_u32(AVXReg target, AVXReg a, uint8_t num_bits) = 0;

  // https://www.google.co.uk/#q=_mm256_srli_epi64
  virtual void rshift_u64(AVXReg target, AVXReg a, uint8_t num_bits) = 0;
  
  // right shift, shift in sign bit
  // https://www.google.co.uk/#q=_mm256_srai_epi16
  virtual void rshift_i16(AVXReg target, AVXReg a, uint8_t num_bits) = 0;

  // https://www.google.co.uk/#q=_mm256_srai_epi32
  virtual void rshift_i32(AVXReg target, AVXReg a, uint8_t num_bits) = 0;
  
  // left shift, shift in zeros 
  // https://www.google.co.uk/#q=_mm256_sll_epi16
  virtual void lshift_u16(AVXReg target, AVXReg a, AVXReg num_bits) = 0;
  virtual bool lshift_u16(AVXReg target, AVXReg a, Reg num_bits, uint32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_sll_epi32
  virtual void lshift_u32(AVXReg target, AVXReg a, AVXReg num_bits) = 0;
  virtual bool lshift_u32(AVXReg target, AVXReg a, Reg num_bits, uint32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_sll_epi64
  virtual void lshift_u64(AVXReg target, AVXReg a, AVXReg num_bits) = 0;
  virtual bool lshift_u64(AVXReg target, AVXReg a, Reg num_bits, uint32_t disp) = 0;

  // right shift, shift in zeros 
  // https://www.google.co.uk/#q=_mm256_srl_epi16
  virtual void rshift_u16(AVXReg target, AVXReg a, AVXReg num_bits) = 0;
  virtual bool rshift_u16(AVXReg target, AVXReg a, Reg num_bits, uint32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_srl_epi32
  virtual void rshift_u32(AVXReg target, AVXReg a, AVXReg num_bits) = 0;
  virtual bool rshift_u32(AVXReg target, AVXReg a, Reg num_bits, uint32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_srl_epi64
  virtual void rshift_u64(AVXReg target, AVXReg a, AVXReg num_bits) = 0;
  virtual bool rshift_u64(AVXReg target, AVXReg a, Reg num_bits, uint32_t disp) = 0;
  
  // right shift, shift in sign bit
  // https://www.google.co.uk/#q=_mm256_sra_epi16
  virtual void rshift_i16(AVXReg target, AVXReg a, AVXReg num_bits) = 0;
  virtual bool rshift_i16(AVXReg target, AVXReg a, Reg num_bits, uint32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_sra_epi32
  virtual void rshift_i32(AVXReg target, AVXReg a, AVXReg num_bits) = 0;
  virtual bool rshift_i32(AVXReg target, AVXReg a, Reg num_bits, uint32_t disp) = 0;

  // left shift, shift in zeros 
  // https://www.google.co.uk/#q=_mm256_sllv_epi32
  virtual void lshiftv_u32(AVXReg target, AVXReg a, AVXReg num_bits) = 0;
  virtual bool lshiftv_u32(AVXReg target, AVXReg a, Reg num_bits, uint32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_sllv_epi64
  virtual void lshiftv_u64(AVXReg target, AVXReg a, AVXReg num_bits) = 0;
  virtual bool lshiftv_u64(AVXReg target, AVXReg a, Reg num_bits, uint32_t disp) = 0;

  // right shift, shift in zeros 
  // https://www.google.co.uk/#q=_mm256_srlv_epi32
  virtual void rshiftv_u32(AVXReg target, AVXReg a, AVXReg num_bits) = 0;
  virtual bool rshiftv_u32(AVXReg target, AVXReg a, Reg num_bits, uint32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_srlv_epi64
  virtual void rshiftv_u64(AVXReg target, AVXReg a, AVXReg num_bits) = 0;
  virtual bool rshiftv_u64(AVXReg target, AVXReg a, Reg num_bits, uint32_t disp) = 0;
  
  // right shift, shift in sign bit
  // https://www.google.co.uk/#q=_mm256_srav_epi32
  virtual void rshiftv_i32(AVXReg target, AVXReg a, AVXReg num_bits) = 0;
  virtual bool rshiftv_i32(AVXReg target, AVXReg a, Reg num_bits, uint32_t disp) = 0;
  
  // 32x int8
  // https://www.google.co.uk/#q=_mm256_shuffle_epi8
  virtual void shufflei8(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool shufflei8(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_broadcast_epi8
  virtual void broadcasti8(AVXReg target, AVXReg source) = 0;
  virtual bool broadcasti8(AVXReg target, Reg source, uint32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_movemask_epi8
  virtual void movemaski8(Reg target, AVXReg a) = 0;

  // https://www.google.co.uk/#q=_mm256_abs_epi8
  virtual void absi8(AVXReg target, AVXReg b) = 0;
  virtual bool absi8(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_avg_epi8
  virtual void avgi8(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool avgi8(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_add_epi8
  virtual void addi8(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool addi8(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_adds_epi8
  virtual void addsi8(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool addsi8(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_adds_epu8
  virtual void addsu8(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool addsu8(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_sub_epi8
  virtual void subi8(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool subi8(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_subs_epi8
  virtual void subsi8(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool subsi8(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_subs_epu8
  virtual void subsu8(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool subsu8(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_max_epu8
  virtual void maxu8(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool maxu8(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_min_epu8
  virtual void minu8(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool minu8(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_max_epi8
  virtual void maxi8(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool maxi8(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_min_epi8
  virtual void mini8(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool mini8(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // 16x int16
  // https://www.google.co.uk/#q=_mm256_broadcast_epi16
  virtual void broadcasti16(AVXReg target, AVXReg source) = 0;
  virtual bool broadcasti16(AVXReg target, Reg source, uint32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_abs_epi16
  virtual void absi16(AVXReg target, AVXReg b) = 0;
  virtual bool absi16(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_avg_epi16
  virtual void avgi16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool avgi16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_add_epi16
  virtual void addi16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool addi16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_adds_epi16
  virtual void addsi16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool addsi16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_adds_epu16
  virtual void addsu16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool addsu16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_hadd_epi16
  virtual void haddi16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool haddi16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_hadds_epi16
  virtual void haddsi16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool haddsi16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_hsub_epi16
  virtual void hsubi16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool hsubi16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_hsubs_epi16
  virtual void hsubsi16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool hsubsi16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_sub_epi16
  virtual void subi16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool subi16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_subs_epi16
  virtual void subsi16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool subsi16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_subs_epu16
  virtual void subsu16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool subsu16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_max_epi16
  virtual void maxi16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool maxi16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_min_epi16
  virtual void mini16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool mini16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_max_epu16
  virtual void maxu16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool maxu16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_min_epu16
  virtual void minu16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool minu16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_mullo_epi16
  virtual void mulli16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool mulli16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_mulhi_epi16
  virtual void mulhi16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool mulhi16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_mulhi_epu16
  virtual void mulhu16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool mulhu16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // 8x int32
  // https://www.google.co.uk/#q=_mm256_broadcast_epi32
  virtual void broadcasti32(AVXReg target, AVXReg source) = 0;
  virtual bool broadcasti32(AVXReg target, Reg source, uint32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_abs_epi32
  virtual void absi32(AVXReg target, AVXReg b) = 0;
  virtual bool absi32(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_add_epi32
  virtual void addi32(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool addi32(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_hadd_epi32
  virtual void haddi32(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool haddi32(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_hsub_epi32
  virtual void hsubi32(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool hsubi32(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_sub_epi32
  virtual void subi32(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool subi32(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_mullo_epi32
  virtual void mulli32(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool mulli32(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_mul_epi32
  virtual void muli32(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool muli32(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_max_epi32
  virtual void maxi32(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool maxi32(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_min_epi32
  virtual void mini32(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool mini32(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_max_epu32
  virtual void maxu32(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool maxu32(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_min_epu32
  virtual void minu32(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool minu32(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // 4x int64
  // https://www.google.co.uk/#q=_mm256_add_epi64
  virtual void addi64(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool addi64(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_sub_epi64
  virtual void subi64(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool subi64(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_broadcast_epi64
  virtual void broadcasti64(AVXReg target, AVXReg source) = 0;
  virtual bool broadcasti64(AVXReg target, Reg source, uint32_t disp) = 0;

  // 128bit
  virtual bool broadcasti128(AVXReg target, Reg source, uint32_t disp) = 0;
  virtual bool broadcastf128(AVXReg target, Reg source, uint32_t disp) = 0;
  virtual void extractf128(AVXReg target, AVXReg b) = 0;
  virtual bool extractf128(AVXReg target, Reg b, int32_t disp) = 0;
  virtual void insertf128(AVXReg target, AVXReg src, AVXReg in, uint8_t mask) = 0;
  virtual bool insertf128(AVXReg target, AVXReg src, Reg in, int32_t disp, uint8_t mask) = 0;
  virtual void inserti128(AVXReg target, AVXReg src, AVXReg in, uint8_t mask) = 0;
  virtual bool inserti128(AVXReg target, AVXReg src, Reg in, int32_t disp, uint8_t mask) = 0;
  virtual void permute2f128(AVXReg target, AVXReg src, AVXReg in, uint8_t mask) = 0;
  virtual bool permute2f128(AVXReg target, AVXReg src, Reg in, int32_t disp, uint8_t mask) = 0;
  virtual void permute2i128(AVXReg target, AVXReg src, AVXReg in, uint8_t mask) = 0;
  virtual bool permute2i128(AVXReg target, AVXReg src, Reg in, int32_t disp, uint8_t mask) = 0;

  // 8 x float

  // https://www.google.co.uk/#q=_mm256_broadcast_ps
  virtual void broadcastss(AVXReg target, AVXReg source) = 0;
  virtual bool broadcastss(AVXReg target, Reg source, uint32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_blendv_ps
  virtual void blendvps(AVXReg target, AVXReg fres, AVXReg tres, AVXReg cmp) = 0;
  virtual bool blendvps(AVXReg target, AVXReg fres, Reg tres, int32_t disp, AVXReg cmp) = 0;

  // https://www.google.co.uk/#q=_mm256_fmadd_ps
  virtual void fmaddps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool fmaddps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_fmsub_ps
  virtual void fmsubps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool fmsubps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_fnmadd_ps
  virtual void fnmaddps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool fnmaddps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_fnmadd_ps
  virtual void fnmsubps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool fnmsubps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_fmaddsub_ps
  virtual void fmaddsubps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool fmaddsubps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_fmsubadd_ps
  virtual void fmsubaddps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool fmsubaddps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_load_ps
  virtual void movaps(AVXReg to, AVXReg from) = 0;
  virtual bool movaps(AVXReg to, Reg from, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_store_ps
  virtual bool movaps(Reg to, AVXReg from) = 0;
  virtual bool movaps(Reg to, int32_t disp, AVXReg from) = 0;

  // https://www.google.co.uk/#q=_mm256_loadu_ps
  virtual void movups(AVXReg to, AVXReg from) = 0;
  virtual bool movups(AVXReg to, Reg from, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_storeu_ps
  virtual bool movups(Reg to, AVXReg from) = 0;
  virtual bool movups(Reg to, int32_t disp, AVXReg from) = 0;

  // https://www.google.co.uk/#q=_mm256_add_ps
  virtual void addps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool addps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_addsub_ps
  virtual void addsubps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool addsubps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_mul_ps
  virtual void mulps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool mulps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_and_ps
  virtual void andps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool andps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_andnot_ps
  virtual void andnotps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool andnotps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_or_ps
  virtual void orps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool orps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_xor_ps
  virtual void xorps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool xorps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_sub_ps
  virtual void subps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool subps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_min_ps
  virtual void minps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool minps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_max_ps
  virtual void maxps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool maxps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_div_ps
  virtual void divps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool divps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_cmp_ps
  virtual void cmpps(AVXReg target, AVXReg a, AVXReg b, cmp mode) = 0;
  virtual bool cmpps(AVXReg target, AVXReg a, Reg b, int32_t disp, cmp mode) = 0;

  // https://www.google.co.uk/#q=_mm256_hadd_ps
  virtual void haddps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool haddps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_hsub_ps
  virtual void hsubps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool hsubps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_sqrt_ps
  virtual void sqrtps(AVXReg target, AVXReg b) = 0;
  virtual bool sqrtps(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_rsqrt_ps
  virtual void rsqrtps(AVXReg target, AVXReg b) = 0;
  virtual bool rsqrtps(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_rcp_ps
  virtual void rcpps(AVXReg target, AVXReg b) = 0;
  virtual bool rcpps(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_shuffle_ps
  virtual void shuffleps(AVXReg target, AVXReg a, AVXReg b, uint8_t x, uint8_t y, uint8_t z, uint8_t w) = 0;
  virtual bool shuffleps(AVXReg target, AVXReg a, Reg b, int32_t disp, uint8_t x, uint8_t y, uint8_t z, uint8_t w) = 0;

  // https://www.google.co.uk/#q=_mm256_round_ps
  virtual void roundps(AVXReg target, AVXReg a, RoundMode mode) = 0;
  virtual bool roundps(AVXReg target, Reg a, int32_t disp, RoundMode mode) = 0;

  // https://www.google.co.uk/#q=_mm256_dp_ps
  virtual void dpps(AVXReg target, AVXReg a, AVXReg b, uint8_t mask) = 0;
  virtual bool dpps(AVXReg target, AVXReg a, Reg b, int32_t disp, uint8_t mask) = 0;

  // https://www.google.co.uk/#q=_mm256_movemask_ps
  virtual void movemaskps(Reg target, AVXReg a) = 0;

  // https://www.google.co.uk/#q=_mm256_unpacklo_ps
  virtual void unpacklops(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool unpacklops(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_unpackhi_ps
  virtual void unpackhips(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool unpackhips(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_movehdup_ps
  virtual void movehdupps(AVXReg target, AVXReg b) = 0;
  virtual bool movehdupps(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_moveldup_ps
  virtual void moveldupps(AVXReg target, AVXReg b) = 0;
  virtual bool moveldupps(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_permutevar_ps
  virtual void permutevar8ps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool permutevar8ps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_permutevar_ps
  virtual void permutevarps(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool permutevarps(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_permute_ps
  virtual void permuteps(AVXReg target, AVXReg b, uint8_t x, uint8_t y, uint8_t z, uint8_t w) = 0;
  virtual bool permuteps(AVXReg target, Reg b, int32_t disp, uint8_t x, uint8_t y, uint8_t z, uint8_t w) = 0;

  // conversion

  // https://www.google.co.uk/#q=_mm256_cvtps_pd
  virtual void cvtpspd(AVXReg target, AVXReg b) = 0;
  virtual bool cvtpspd(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_cvtps_epi32
  virtual void cvtpsdq(AVXReg target, AVXReg b) = 0;
  virtual bool cvtpsdq(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_cvtepi32_ps
  virtual void cvtdqps(AVXReg target, AVXReg b) = 0;
  virtual bool cvtdqps(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_cvtepi32_ss
  virtual void cvtsi2ss(AVXReg target, AVXReg b) = 0;
  virtual bool cvtsi2ss(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_cvttss_epi32
  virtual void cvttss2si(Reg target, AVXReg b) = 0;
  virtual bool cvttss2si(Reg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_cvtss_epi32
  virtual void cvtss2si(Reg target, AVXReg b) = 0;
  virtual bool cvtss2si(Reg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_cvtepi32_sd
  virtual void cvtsi2sd(AVXReg target, Reg b) = 0;
  virtual bool cvtsi2sd(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_cvttsd_epi32
  virtual void cvttsd2si(Reg target, AVXReg b) = 0;
  virtual bool cvttsd2si(Reg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_cvtsd_epi32
  virtual void cvtsd2si(Reg target, AVXReg b) = 0;
  virtual bool cvtsd2si(Reg target, Reg b, int32_t disp) = 0;

  // you probably don't want these convert functions. (old MMX/SSE?)
  virtual void cvtpi2ps(AVXReg target, AVXReg b) = 0;
  virtual bool cvtpi2ps(AVXReg target, Reg b, int32_t disp) = 0;
  virtual void cvtps2pi(AVXReg target, AVXReg b) = 0;
  virtual bool cvtps2pi(AVXReg target, Reg b, int32_t disp) = 0;
  virtual void cvtpi2pd(AVXReg target, AVXReg b) = 0;
  virtual bool cvtpi2pd(AVXReg target, Reg b, int32_t disp) = 0;
  virtual void cvtpd2pi(AVXReg target, AVXReg b) = 0;
  virtual bool cvtpd2pi(AVXReg target, Reg b, int32_t disp) = 0;
  virtual void cvttps2pi(AVXReg target, AVXReg b) = 0;
  virtual bool cvttps2pi(AVXReg target, Reg b, int32_t disp) = 0;
  virtual void cvttpd2pi(AVXReg target, AVXReg b) = 0;
  virtual bool cvttpd2pi(AVXReg target, Reg b, int32_t disp) = 0;
  
  // integer comparison
  // https://www.google.co.uk/#q=_mm256_cmpgt_epi8
  virtual void cmpgti8(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool cmpgti8(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_cmpgt_epi16
  virtual void cmpgti16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool cmpgti16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_cmpgt_epi32
  virtual void cmpgti32(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool cmpgti32(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_cmpgt_epi64
  virtual void cmpgti64(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool cmpgti64(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_cmpeq_epi8
  virtual void cmpeqi8(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool cmpeqi8(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_cmpeq_epi16
  virtual void cmpeqi16(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool cmpeqi16(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_cmpeq_epi32
  virtual void cmpeqi32(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool cmpeqi32(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_cmpeq_epi64
  virtual void cmpeqi64(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool cmpeqi64(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // 4x double
  // https://www.google.co.uk/#q=_mm256_broadcast_pd
  virtual void broadcastsd(AVXReg target, AVXReg source) = 0;
  virtual bool broadcastsd(AVXReg target, Reg source, uint32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_blendv_pd
  virtual void blendvpd(AVXReg target, AVXReg fres, AVXReg tres, AVXReg cmp) = 0;
  virtual bool blendvpd(AVXReg target, AVXReg fres, Reg tres, int32_t disp, AVXReg cmp) = 0;

  // https://www.google.co.uk/#q=_mm256_fmadd_pd
  virtual void fmaddpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool fmaddpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_fmsub_pd
  virtual void fmsubpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool fmsubpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_fmnadd_pd
  virtual void fnmaddpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool fnmaddpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_fmnsub_pd
  virtual void fnmsubpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool fnmsubpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_fmaddsub_pd
  virtual void fmaddsubpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool fmaddsubpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_fmsubadd_pd
  virtual void fmsubaddpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool fmsubaddpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_load_pd
  virtual void movapd(AVXReg to, AVXReg from) = 0;
  virtual bool movapd(AVXReg to, Reg from, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_store_pd
  virtual bool movapd(Reg to, AVXReg from) = 0;
  virtual bool movapd(Reg to, int32_t disp, AVXReg from) = 0;

  // https://www.google.co.uk/#q=_mm256_loadu_pd
  virtual void movupd(AVXReg to, AVXReg from) = 0;
  virtual bool movupd(AVXReg to, Reg from, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_storeu_pd
  virtual bool movupd(Reg to, AVXReg from) = 0;
  virtual bool movupd(Reg to, int32_t disp, AVXReg from) = 0;

  // https://www.google.co.uk/#q=_mm256_add_pd
  virtual void addpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool addpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_mul_pd
  virtual void mulpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool mulpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_and_pd
  virtual void andpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool andpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_andnot_pd
  virtual void andnotpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool andnotpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_or_pd
  virtual void orpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool orpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_xor_pd
  virtual void xorpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool xorpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_sub_pd
  virtual void subpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool subpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_min_pd
  virtual void minpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool minpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_max_pd
  virtual void maxpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool maxpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_div_pd
  virtual void divpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool divpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_cmp_pd
  virtual void cmppd(AVXReg target, AVXReg a, AVXReg b, cmp mode) = 0;
  virtual bool cmppd(AVXReg target, AVXReg a, Reg b, int32_t disp, cmp mode) = 0;

  // https://www.google.co.uk/#q=_mm256_sqrt_pd
  virtual void sqrtpd(AVXReg target, AVXReg b) = 0;
  virtual bool sqrtpd(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_shuffle_pd
  virtual void shufflepd(AVXReg target, AVXReg a, AVXReg b, uint8_t x, uint8_t y) = 0;
  virtual bool shufflepd(AVXReg target, AVXReg a, Reg b, int32_t disp, uint8_t x, uint8_t y) = 0;

  // https://www.google.co.uk/#q=_mm256_round_pd
  virtual void roundpd(AVXReg target, AVXReg a, RoundMode mode) = 0;
  virtual bool roundpd(AVXReg target, Reg a, int32_t disp, RoundMode mode) = 0;

  // WARNING: The Intel manual says that this instruction exists. 
  // Visual Studio begs to differ (disassembly is marked as garbage). 
  // I've not seen it exposed as an intrinsic anywehere, so I suspect it's not valid. 
  // I've not been brave enough to try executing it.
  virtual void dppd(AVXReg target, AVXReg a, AVXReg b, uint8_t mask) = 0;
  virtual bool dppd(AVXReg target, AVXReg a, Reg b, int32_t disp, uint8_t mask) = 0;

  // https://www.google.co.uk/#q=_mm256_hadd_pd
  virtual void haddpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool haddpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_hsub_pd
  virtual void hsubpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool hsubpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_movemask_pd
  virtual void movemaskpd(Reg target, AVXReg a) = 0;

  // https://www.google.co.uk/#q=_mm256_unpacklo_pd
  virtual void unpacklopd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool unpacklopd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_unpackhi_pd
  virtual void unpackhipd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool unpackhipd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_movedup_pd
  virtual void moveduppd(AVXReg target, AVXReg b) = 0;
  virtual bool moveduppd(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_permutevar_pd
  virtual void permutevarpd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool permutevarpd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm256_permute_pd
  virtual void permutepd(AVXReg target, AVXReg b, uint8_t x, uint8_t y) = 0;
  virtual bool permutepd(AVXReg target, Reg b, int32_t disp, uint8_t x, uint8_t y) = 0;

  // single single
  // https://www.google.co.uk/#q=_mm_load_ss
  virtual void movss(AVXReg to, AVXReg from) = 0;
  virtual bool movss(AVXReg to, Reg from, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm_store_ss
  virtual bool movss(Reg to, AVXReg from) = 0;
  virtual bool movss(Reg to, int32_t disp, AVXReg from) = 0;

  // https://www.google.co.uk/#q=_mm_add_ss
  virtual void addss(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool addss(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm_mul_ss
  virtual void mulss(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool mulss(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm_sub_ss
  virtual void subss(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool subss(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm_min_ss
  virtual void minss(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool minss(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm_max_ss
  virtual void maxss(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool maxss(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm_div_ss
  virtual void divss(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool divss(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm_cmp_ss
  virtual void cmpss(AVXReg target, AVXReg a, AVXReg b, cmp mode) = 0;
  virtual bool cmpss(AVXReg target, AVXReg a, Reg b, int32_t disp, cmp mode) = 0;

  // https://www.google.co.uk/#q=_mm_sqrt_ss
  virtual void sqrtss(AVXReg target, AVXReg b) = 0;
  virtual bool sqrtss(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm_rsqrt_ss
  virtual void rsqrtss(AVXReg target, AVXReg b) = 0;
  virtual bool rsqrtss(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm_rcp_ss
  virtual void rcpss(AVXReg target, AVXReg b) = 0;
  virtual bool rcpss(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm_round_ss
  virtual void roundss(AVXReg target, AVXReg a, RoundMode mode) = 0;
  virtual bool roundss(AVXReg target, Reg a, uint32_t disp, RoundMode mode) = 0;
  
  // single double
  // https://www.google.co.uk/#q=_mm_load_sd
  virtual void movsd(AVXReg to, AVXReg from) = 0;
  virtual bool movsd(AVXReg to, Reg from, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm_store_sd
  virtual bool movsd(Reg to, AVXReg from) = 0;
  virtual bool movsd(Reg to, int32_t disp, AVXReg from) = 0;

  // https://www.google.co.uk/#q=_mm_add_sd
  virtual void addsd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool addsd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm_mul_sd
  virtual void mulsd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool mulsd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm_sub_sd
  virtual void subsd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool subsd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm_min_sd
  virtual void minsd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool minsd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm_max_sd
  virtual void maxsd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool maxsd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm_div_sd
  virtual void divsd(AVXReg target, AVXReg a, AVXReg b) = 0;
  virtual bool divsd(AVXReg target, AVXReg a, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm_cmp_sd
  virtual void cmpsd(AVXReg target, AVXReg a, AVXReg b, cmp mode) = 0;
  virtual bool cmpsd(AVXReg target, AVXReg a, Reg b, int32_t disp, cmp mode) = 0;

  // https://www.google.co.uk/#q=_mm_sqrt_sd
  virtual void sqrtsd(AVXReg target, AVXReg b) = 0;
  virtual bool sqrtsd(AVXReg target, Reg b, int32_t disp) = 0;

  // https://www.google.co.uk/#q=_mm_round_sd
  virtual void roundsd(AVXReg target, AVXReg a, RoundMode mode) = 0;
  virtual bool roundsd(AVXReg target, Reg a, int32_t disp, RoundMode mode) = 0;

  // General Purpose register manipulation
  virtual void push(Reg reg) = 0;
  virtual void pop(Reg reg) = 0;
  virtual void add(Reg output, Reg input, int32_t offset) = 0;
  virtual void mov64(Reg output, Reg input, int32_t offset) = 0;
  virtual void mov64(Reg output, int32_t offset, Reg input) = 0;
  virtual void lea(Reg target, Reg b, int32_t offset) = 0;
  virtual void setzero(AVXReg r) = 0;
  virtual void loadcount(Reg r, uint32_t count) = 0;
  virtual void dec(Reg r) = 0;
  virtual void inc(Reg r) = 0;
  virtual void add(Reg r, int32_t immediate) = 0;
  virtual void or(Reg r, int32_t immediate) = 0;
  virtual void adc(Reg r, int32_t immediate) = 0;
  virtual void sbb(Reg r, int32_t immediate) = 0;
  virtual void and(Reg r, int32_t immediate) = 0;
  virtual void sub(Reg r, int32_t immediate) = 0;
  virtual void xor(Reg r, int32_t immediate) = 0;
  virtual void cmp(Reg r, int32_t immediate) = 0;

  // jump to a previous location within the code
  virtual void jump_eq_label(const char* label) = 0;
  virtual void jump_ne_label(const char* label) = 0;
  virtual void jump_lt_label(const char* label) = 0;
  virtual void jump_gt_label(const char* label) = 0;
  virtual void jump_le_label(const char* label) = 0;
  virtual void jump_ge_label(const char* label) = 0;

  // insert label at current location
  virtual void insert_label(const char* label) = 0;

  // jump to a previous location within the code
  virtual void jump_eq_to(uint32_t location) = 0;
  virtual void jump_ne_to(uint32_t location) = 0;
  virtual void jump_lt_to(uint32_t location) = 0;
  virtual void jump_gt_to(uint32_t location) = 0;
  virtual void jump_le_to(uint32_t location) = 0;
  virtual void jump_ge_to(uint32_t location) = 0;
  
  // jump relative to instruction pointer
  virtual void jump_eq(int32_t offset) = 0;
  virtual void jump_ne(int32_t offset) = 0;
  virtual void jump_lt(int32_t offset) = 0;
  virtual void jump_gt(int32_t offset) = 0;
  virtual void jump_le(int32_t offset) = 0;
  virtual void jump_ge(int32_t offset) = 0;

  virtual void call_prodecure(const char* str) = 0;
  virtual void prodecure(const char* str) = 0;

  /// copy register value
  virtual void mov(Reg target, Reg a) = 0;

  /// function return!
  virtual void ret() = 0;

  // gathering support
  // Note: If you want a standard (non masked) gather, then generate the mask with a simple cmpeq, e.g.
  // cmpeqi8(YMM0, YMM0, YMM0). Intrinsics make the distinction between the two operations, but ASM does not.

  // https://www.google.co.uk/#q=_mm256_mask_i32gather_ps
  virtual bool i32gatherps(AVXReg target, AVXReg indices, AVXReg mask, Reg address, uint32_t disp, uint8_t scale) = 0;

  // https://www.google.co.uk/#q=_mm256_mask_i64gather_ps
  virtual bool i64gatherps(AVXReg target, AVXReg indices, AVXReg mask, Reg address, uint32_t disp, uint8_t scale) = 0;

  // https://www.google.co.uk/#q=_mm256_mask_i32gather_pd
  virtual bool i32gatherpd(AVXReg target, AVXReg indices, AVXReg mask, Reg address, uint32_t disp, uint8_t scale) = 0;

  // https://www.google.co.uk/#q=_mm256_mask_i64gather_pd
  virtual bool i64gatherpd(AVXReg target, AVXReg indices, AVXReg mask, Reg address, uint32_t disp, uint8_t scale) = 0;
};

/// \brief  A little utility class which acts as the main entry point into the runtime assembler lib. 
///         It's main purpose is to load the DLL dynamically, and safely initialise the libs internals. 
class AssemblerLib 
{
public:

  /// \brief  constructor. 
  /// \param  path  the path to the libASM.dll 
  inline AssemblerLib(const char* path = "libASM.dll")
    {
      // initialise required Win32 information
      g_win32Ptrs[0] = GetProcessHeap();
      g_win32Ptrs[1] = GetCurrentProcess();
      g_win32Ptrs[2] = HeapAlloc;
      g_win32Ptrs[3] = HeapFree;
      g_win32Ptrs[4] = VirtualAllocEx;
      g_win32Ptrs[5] = VirtualFreeEx;
      g_win32Ptrs[6] = lstrcmpA;
      g_win32Ptrs[7] = lstrlenA;

      // load the dll
      m_dll = LoadLibraryA(path);
      if (m_dll) 
      {
        // extract API functions
        m_init = (InitLibFn)GetProcAddress(m_dll, "vpu_initLib");
        m_fnFn = (CreateFunctionTableFn)GetProcAddress(m_dll, "vpu_createFunctionTable");
        m_asmFn = (CreateAssemblerFn)GetProcAddress(m_dll, "vpu_createAssembler");
        if (m_init && m_fnFn && m_asmFn) 
        {
          // all ok, init
          m_init(g_win32Ptrs);
        }
        else 
        {
          // failure!
          m_fnFn = 0;
          m_asmFn = 0;
        }
      }
      else 
      {
        // DLL failed to load
        m_init = 0;
        m_fnFn = 0;
        m_asmFn = 0;
      }
    }

  /// \brief  dtor, unloads the DLL
  inline ~AssemblerLib() 
    {
      if (m_dll)
      {
        FreeLibrary(m_dll);
        m_dll = 0;
      }
    }

  /// \brief  utility to check that the DLL loaded ok.
  inline bool isOk() const 
    { return m_fnFn != 0; }

  /// \brief  create a new function table
  inline IFunctionTable* createFunctionTable()
    { return m_fnFn ? m_fnFn() : 0; }

  /// \brief  create a new assembler
  /// \param  page_size This param controls the size of the executable memory page allocated by the assembler. 
  ///         By default this is 4K, but you may require a larger memory page. 
  inline IAssembler* createAssembler(size_t page_size = 4096)
    { return m_asmFn ? m_asmFn(page_size) : 0; }

private:
  HMODULE m_dll;
  InitLibFn m_init;
  CreateFunctionTableFn m_fnFn;
  CreateAssemblerFn m_asmFn;
  void* g_win32Ptrs[8];
};

} // vpu
