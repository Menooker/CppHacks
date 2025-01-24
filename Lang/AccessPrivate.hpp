#pragma once
// The implementation is copied from
// https://gist.github.com/altamic/d3778f906b63c6983ef074635eb36c85

namespace cpphack {
// This is a rewrite of the technique showed here:
// http://bloglitb.blogspot.com/2010/07/access-to-private-members-thats-easy.html
// https://gist.github.com/dabrahams/1528856

// Generate a static data member of type Stub::type in which to store
// the address of a private member.  It is crucial that Stub does not
// depend on the /value/ of the the stored address in any way so that
// we can access it from ordinary code without directly touching
// private data.
template <class Stub>
struct stowed {
  static typename Stub::type value;
};
template <class Stub>
typename Stub::type stowed<Stub>::value;

// Generate a static data member whose constructor initializes
// stowed<Stub>::value.  This type will only be named in an explicit
// instantiation, where it is legal to pass the address of a private
// member.
template <class Stub, typename Stub::type x>
struct stow_private {
  stow_private() { stowed<Stub>::value = x; }
  static stow_private instance;
};
template <class Stub, typename Stub::type x>
stow_private<Stub, x> stow_private<Stub, x>::instance;

// private instance methods
template <typename Stub>
struct result {
  /* export it ... */
  typedef typename Stub::type type;
  static type ptr;
};

template <typename Stub>
typename result<Stub>::type result<Stub>::ptr;

template <typename Stub, typename Stub::type p>
struct stowed_method : result<Stub> {
  /* fill it ... */
  struct _stowed_method {
    _stowed_method() { result<Stub>::ptr = p; }
  };
  static _stowed_method stowed_method_obj;
};

template <typename Stub, typename Stub::type p>
typename stowed_method<Stub, p>::_stowed_method
    stowed_method<Stub, p>::stowed_method_obj;

template <typename Stub, typename T>
auto accessPrivateField(T &v) -> decltype(v.*stowed<Stub>::value) {
  return v.*stowed<Stub>::value;
}

template <typename Stub>
auto accessStaticField() -> decltype(*stowed<Stub>::value) {
  return *stowed<Stub>::value;
}

template <typename Stub>
auto accessPrivateMethod() {
  return result<Stub>::ptr;
}
} // namespace cpphack

#define HACK_CPP_DEF_PRIVATE_FIELD(ACC_NAME, CLASS, FIELD, TYPE)               \
  struct ACC_NAME {                                                            \
    typedef TYPE(CLASS::*type);                                                \
  };                                                                           \
  template class ::cpphack::stow_private<ACC_NAME, &CLASS::FIELD>;

#define HACK_CPP_DEF_STATIC_FIELD(ACC_NAME, CLASS, FIELD, TYPE)                \
  struct ACC_NAME {                                                            \
    typedef TYPE *type;                                                        \
  };                                                                           \
  template class ::cpphack::stow_private<ACC_NAME, &CLASS::FIELD>;

#define HACK_CPP_DEF_PRIVATE_METHOD(ACC_NAME, CLASS, FIELD, RTYPE, ARGS)       \
  struct ACC_NAME {                                                            \
    typedef RTYPE(CLASS::*type) ARGS;                                          \
  };                                                                           \
  template class ::cpphack::stowed_method<ACC_NAME, &CLASS::FIELD>;

#define HACK_CPP_DEF_STATIC_METHOD(ACC_NAME, CLASS, FIELD, RTYPE, ARGS)        \
  struct ACC_NAME {                                                            \
    typedef RTYPE(*type) ARGS;                                                 \
  };                                                                           \
  template class ::cpphack::stowed_method<ACC_NAME, &CLASS::FIELD>;
