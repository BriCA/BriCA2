#ifndef __BRICA2_MACROS_HPP__
#define __BRICA2_MACROS_HPP__

// clang-format off
#if !defined(NAMESPACE_BEGIN)
#  define NAMESPACE_BEGIN(name) namespace name {
#endif
#if !defined(NAMESPACE_END)
#  define NAMESPACE_END(name) }
#endif
// clang-format on

#ifndef BRICA2_NAMESPACE
#define BRICA2_NAMESPACE brica2
#endif

#endif  // __BRICA2_MACROS_HPP__
