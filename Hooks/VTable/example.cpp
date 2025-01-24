#include "VTableHook.hpp"
#include <cstdlib>
#include <stdio.h>

struct Base1 {
  virtual int run(int v) = 0;
  virtual int srun(int v) = 0;
};

struct Base2 {
  virtual int run2(int v) = 0;
};

struct SingleParent : Base1 {
  int run(int v) {
    printf("run=%d\n", v);
    return v;
  }
  int srun(int v) {
    printf("srun=%d\n", v);
    return v;
  }
};

struct DoubleParent : SingleParent, Base2 {
  int val = 114;
  int run2(int v) override {
    printf("run2=%d %d\n", v, val);
    return v + val;
  }
};

#define ASSERT(EXPR)                                                           \
  if (!(EXPR)) {                                                               \
    fprintf(stderr, "Assertion failed: " #EXPR);                               \
    std::abort();                                                              \
  }

int (*oldRun2)(Base2 *, int);

int func3(Base2 *ths, int v) {
  auto subths = static_cast<DoubleParent *>(ths);
  printf("hooked run2=%d %d\n", v, subths->val);
  printf("base2=%p subclass=%p\n", ths, subths);
  auto oldret = oldRun2(subths, v);
  return oldret + subths->val;
};

int main() {
  auto sp = new SingleParent();
  auto func = +[](Base1 *, int v) -> int {
    printf("hooked run=%d\n", v);
    return 114514;
  };
  cpphack::hookVTableOf(sp, &Base1::run, func);
  auto func2 = +[](Base1 *, int v) -> int {
    printf("hooked srun=%d\n", v);
    return 1919810;
  };
  cpphack::hookVTableOf(sp, &Base1::srun, func2);
  ASSERT(sp->run(1) == 114514);
  ASSERT(sp->srun(2) == 1919810);

  auto dp = new DoubleParent();
  cpphack::hookVTableOf(dp, &Base1::srun, func2);
  // run2 exists in two places: vtable of DoubleParent::Base2 and vtable of
  // DoubleParent
  oldRun2 = cpphack::hookVTableOf(dp, &Base2::run2, func3);
  // MethodThisWrapper wraps int func3(Base2* ths, int v) to int
  // (*)(DoubleParent* ths, int v)
  cpphack::hookVTableOf(
      dp, &DoubleParent::run2,
      cpphack::MethodThisWrapper<DoubleParent, decltype(&func3), func3>);
  ASSERT(dp->srun(2) == 1919810);
  ASSERT(static_cast<Base2 *>(dp)->run2(2) == 114 * 2 + 2);
  ASSERT(dp->run2(2) == 114 * 2 + 2);
  return 0;
}