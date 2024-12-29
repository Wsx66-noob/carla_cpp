// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 防止头文件被重复包含
// 检查是否使用Microsoft Visual C++编译器
#if defined(_MSC_VER)
   // 定义LIBCARLA_FORCEINLINE宏为__forceinline，用于强制内联函数 
#  define LIBCARLA_FORCEINLINE __forceinline
   // 定义LIBCARLA_NOINLINE宏为__declspec(noinline)，用于禁止函数内联RCEINLINE __forceinline  
#  define LIBCARLA_NOINLINE __declspec(noinline)


  // 检查是否使用Clang或GCC编译器
// 如果编译器是Clang或者GNU GCC，则进入这个条件编译块
#elif defined(__clang__) || defined(__GNUC__)
  // 如果定义了NDEBUG宏（通常在非调试模式下定义），则定义LIBCARLA_FORCEINLINE宏
  // LIBCARLA_FORCEINLINE宏会使函数内联，并添加always_inline属性，强制编译器内联该函数
#  if defined(NDEBUG)
#    define LIBCARLA_FORCEINLINE inline __attribute__((always_inline))
  // 如果没有定义NDEBUG宏（即在调试模式下），则只将LIBCARLA_FORCEINLINE宏定义为inline
#  else
#    define LIBCARLA_FORCEINLINE inline
#  endif // NDEBUG
  // 定义LIBCARLA_NOINLINE宏，使用__attribute__((noinline))属性来禁止编译器内联该函数
#  define LIBCARLA_NOINLINE __attribute__((noinline))  

// 对于其他编译器，发出警告提示编译器不支持
#else
#  warning Compiler not supported.  // 编译器不支持的警告
  // 定义LIBCARLA_NOINLINE宏为空，表示在这些编译器上不提供禁止内联的功能
#  define LIBCARLA_NOINLINE 
#endif
