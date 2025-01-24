# Toolbox for hacking C++

This Repo collects several handy tools for hacking and debugging C++ code.

## VTable Hooks

Header-only tool at `Hooks/VTable/VTableHook.hpp`. Only works in Itanium C++ ABI (gcc and clang). Tested on x64.

Replacing a virtual method with your own one. Supports single/multiple virtual base classes.

Developed based on [vtable-hook](https://github.com/Thordin/vtable-hook). Under MIT license.

## Accessing private/protected class fields and methods

Header-only tool at `Lang/AccessPrivate.hpp`. It should work on any modern C++ compilers.

Developed based on [@altamic's gist](https://gist.github.com/altamic/d3778f906b63c6983ef074635eb36c85)