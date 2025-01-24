#include "AccessPrivate.hpp"
#include <stdio.h>

// with a class
class B {
public:
  B() { x = 42; }

private:
  int x;
  static int y;
  void f(int v, int k) {
    printf("class private instance method: %d %d\n", x, v);
  }
  static void g(int v, int k) {
    printf("class private static method: %d\n", y);
  }
};

int B::y = 42; // initialize static member

HACK_CPP_DEF_PRIVATE_FIELD(B_x, B, x, int);
HACK_CPP_DEF_STATIC_FIELD(B_y, B, y, int);
HACK_CPP_DEF_PRIVATE_METHOD(B_f, B, f, void, (int, int));
HACK_CPP_DEF_STATIC_METHOD(B_g, B, g, void, (int, int));

using namespace cpphack;
int main() {
  B b;

  auto &bx = accessPrivateField<B_x>(b);
  // Use the stowed private member pointer
  printf("B.x = %d\n", bx);
  bx = 114514;
  (b.*accessPrivateMethod<B_f>())(123, 321);
  // Use the stowed private member pointer
  printf("B.y = %d\n", accessStaticField<B_y>());
  accessStaticField<B_y>() = 1919810;
  accessPrivateMethod<B_g>()(123, 321);
}
