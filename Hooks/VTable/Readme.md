# VTable Hooking

The API:

```c++
template <typename WhoseVtable, typename TArg, typename R, typename... Args>
auto hookVTableOf(TArg *obj, R (WhoseVtable::*fptr)(Args...),
                  R(newfunc)(WhoseVtable *, Args...)) -> decltype(newfunc)
```

`obj` should be a pointer to an object to patch the vtable. Note that the vtable is shared among all instances of the object's class. You also need to pass a `fptr` which is a pointer to a virtual function of a class. The `newfunc` parameter should be the function to be used to replace the virtual function.  `cpphack::hookVTableOf` returns the original virtual function defined in the class.

Notes: if your class inherits multiple parent base class with virtual functions, there will be multiple vtables to patch for a single virtual function. For example:

```c++
struct Base1 {
  virtual int run(int v) = 0;
};

struct Base2 {
  virtual int run2(int v) = 0;
};

struct SingleParent : Base1 {
  int run(int v) override {
    printf("run=%d\n", v);
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

int (*oldRun2)(Base2 *, int);

// this is the function to replace DoubleParent::run2
int func3(Base2 *ths, int v) {
  auto subths = static_cast<DoubleParent *>(ths);
  printf("hooked run2=%d %d\n", v, subths->val);
  printf("base2=%p subclass=%p\n", ths, subths);
  auto oldret = oldRun2(subths, v);
  return oldret + subths->val;
};
```

To correctly patch `run2` in `DoubleParent`, you need to run

```c++
  auto dp = new DoubleParent();
  // run2 exists in two places: vtable of DoubleParent::Base2 and vtable of
  // DoubleParent
  oldRun2 = cpphack::hookVTableOf(dp, &Base2::run2, func3);
  // MethodThisWrapper wraps int func3(Base2* ths, int v) to int
  // (*)(DoubleParent* ths, int v)
  cpphack::hookVTableOf(
      dp, &DoubleParent::run2,
      cpphack::MethodThisWrapper<DoubleParent, decltype(&func3), func3>);
```