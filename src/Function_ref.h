//==============================================================================
// LLVM Release License
//==============================================================================
// University of Illinois/NCSA
//    Open Source License
//
// Copyright (c) 2003-2016 University of Illinois at Urbana-Champaign.
// All rights reserved.
//
// Developed by:
//
// LLVM Team
//
// University of Illinois at Urbana-Champaign
//
//    http://llvm.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal with the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
//* Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimers.
//
//* Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimers in the
// documentation and/or other materials provided with the distribution.
//
//* Neither the names of the LLVM Team, University of Illinois at
// Urbana-Champaign, nor the names of its contributors may be used to
// endorse or promote products derived from this Software without specific
// prior written permission.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// WITH THE SOFTWARE.

/// An efficient, type-erasing, non-owning reference to a callable. This is
/// intended for use as the type of a function parameter that is not used
/// after the function in question returns.
///
/// This class does not own the callable, so it is not in general safe to store
/// a function_ref.

/// @file  Function_ref.h from llvm/ADT/STLExtras.h
/// @brief Efficient, type-erasing, non-owning function reference
/// @author https://llvm.org

#ifndef SRC_FUNCTION_REF_H_
#define SRC_FUNCTION_REF_H_

#include <cstdint>
#include <type_traits>
#include <utility>
template <typename Fn>
class function_ref;

template <typename Ret, typename... Params>
class function_ref<Ret(Params...)>
{
  Ret (*callback)(intptr_t callable, Params... params);
  intptr_t callable;

  template <typename Callable>
  static Ret callback_fn(intptr_t callable, Params... params)
  {
    return (*reinterpret_cast<Callable*>(callable))(
        std::forward<Params>(params)...);
  }

 public:
  template <typename Callable>
  function_ref(Callable&& callable,
               typename std::enable_if<
                   !std::is_same<typename std::remove_reference<Callable>::type,
                                 function_ref>::value>::type* = nullptr)
      : callback(callback_fn<typename std::remove_reference<Callable>::type>)
      , callable(reinterpret_cast<intptr_t>(&callable))
  {
  }

  Ret operator()(Params... params) const
  {
    return callback(callable, std::forward<Params>(params)...);
  }
};

#endif  // SRC_FUNCTION_REF_H_
