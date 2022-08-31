Vectorpack
==========

This is a C++ library to implement algorithms for solving the Vector Packing problem.
The library optionnaly includes a set of classical algorithms to solve the problem, such as First First, Best Fit, Worst Fit, with many variants.

A simple executable is also provided to call an algorithm on an instance of the Vector Packing problem.
The instance input file must be in the VBP format introduced for the [VPSolver](https://github.com/fdabrandao/vpsolver) by F. Brandão (see [manual here](https://github.com/fdabrandao/vpsolver/blob/master/docs/vpsolver_manual.pdf)).


The code of this repository is under the LGPL v3.0 license.
Contributions in this project are welcome!


Installation
============

The only dependency is `cmake` (version > 3.2) and the build and install phase are rather classic.

By default, only the `Vectorpack` library will be built and installed, including the packing algorithms.
If you want only the library but no algorithms, add `-Dinclude_algorithms=OFF` in the `cmake` configure call.

If you want to build the executable instead of the library, add `-Dbuild_executable=ON` instead.

