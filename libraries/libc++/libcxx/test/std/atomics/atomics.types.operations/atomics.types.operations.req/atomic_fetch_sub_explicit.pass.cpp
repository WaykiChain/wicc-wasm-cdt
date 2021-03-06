//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads
//  ... test crashes clang

// <atomic>

// template <class Integral>
//     Integral
//     atomic_fetch_sub_explicit(volatile atomic<Integral>* obj, Integral op,
//                               memory_order m);
// template <class Integral>
//     Integral
//     atomic_fetch_sub_explicit(atomic<Integral>* obj, Integral op,
//                               memory_order m);
//
// template <class T>
//     T*
//     atomic_fetch_sub_explicit(volatile atomic<T*>* obj, ptrdiff_t op,
//                               memory_order m);
// template <class T>
//     T*
//     atomic_fetch_sub_explicit(atomic<T*>* obj, ptrdiff_t op, memory_order m);

#include <atomic>
#include <type_traits>
#include <cassert>

#include "atomic_helpers.h"

template <class T>
struct TestFn {
  void operator()() const {
    {
        typedef std::atomic<T> A;
        A t;
        std::atomic_init(&t, T(3));
        assert(std::atomic_fetch_sub_explicit(&t, T(2),
                                            std::memory_order_seq_cst) == T(3));
        assert(t == T(1));
    }
    {
        typedef std::atomic<T> A;
        volatile A t;
        std::atomic_init(&t, T(3));
        assert(std::atomic_fetch_sub_explicit(&t, T(2),
                                            std::memory_order_seq_cst) == T(3));
        assert(t == T(1));
    }
  }
};

template <class T>
void testp()
{
    {
        typedef std::atomic<T> A;
        typedef typename std::remove_pointer<T>::type X;
        A t;
        std::atomic_init(&t, T(3*sizeof(X)));
        assert(std::atomic_fetch_sub_explicit(&t, 2,
                                  std::memory_order_seq_cst) == T(3*sizeof(X)));
        assert(t == T(1*sizeof(X)));
    }
    {
        typedef std::atomic<T> A;
        typedef typename std::remove_pointer<T>::type X;
        volatile A t;
        std::atomic_init(&t, T(3*sizeof(X)));
        assert(std::atomic_fetch_sub_explicit(&t, 2,
                                  std::memory_order_seq_cst) == T(3*sizeof(X)));
        assert(t == T(1*sizeof(X)));
    }
}

int main()
{
    TestEachIntegralType<TestFn>()();
    testp<int*>();
    testp<const int*>();
}
