#pragma once
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <utility>
namespace cpphack {
inline void vtablehook_unprotect(void *region) {
  int vtablehook_pagesize = sysconf(_SC_PAGE_SIZE);
  int vtablehook_pagemask = ~(vtablehook_pagesize - 1);
  mprotect((void *)((intptr_t)region & vtablehook_pagemask),
           vtablehook_pagesize, PROT_READ | PROT_WRITE | PROT_EXEC);
}

inline void vtablehook_protect(void *region) {
  int vtablehook_pagesize = sysconf(_SC_PAGE_SIZE);
  int vtablehook_pagemask = ~(vtablehook_pagesize - 1);
  mprotect((void *)((intptr_t)region & vtablehook_pagemask),
           vtablehook_pagesize, PROT_READ | PROT_EXEC);
}

template <class To, class F, F f>
struct wrapper_impl;
template <class To, class From, class R, class... Args, R (*f)(From *, Args...)>
struct wrapper_impl<To, R (*)(From *, Args...), f> {
  static R wrap(To *v, Args... args) {
    return f(dynamic_cast<From *>(v), args...);
  }
};

template <class To, class F, F f>
constexpr auto MethodThisWrapper = wrapper_impl<To, F, f>::wrap;
template <typename WhoseVtable, typename TArg, typename R, typename... Args>
auto hookVTableOf(TArg *obj, R (WhoseVtable::*fptr)(Args...),
                  R(newfunc)(WhoseVtable *, Args...)) -> decltype(newfunc) {
  auto ptr = dynamic_cast<WhoseVtable *>(obj);
  void **vtable = *(void ***)ptr;
  void *offset = *(void **)&fptr;
  vtablehook_unprotect(vtable);
  auto &vtableitem = vtable[((uintptr_t)offset) / sizeof(void *)];
  auto old_func = vtableitem;
  vtableitem = (void *)newfunc;
  vtablehook_protect(vtable);
  return decltype(newfunc)(old_func);
}
} // namespace cpphack