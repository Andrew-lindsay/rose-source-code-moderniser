# Source code modernisations tools
Masters of informatics project utilising the ["Rose"](http://rosecompiler.org/) source to source compiler framework.

## Source code transformations developed
The tools developed take a c++ source file as input and produce a new file with the tools rejuvenation applied. 

### Auto-rejuvenation
Finds variable declaration and when applicable replaces the type with c++11 keyword auto that allows the compiler to infer the type. The transformation makes ensure that type information that cannot be infered by [auto](https://en.cppreference.com/w/cpp/language/auto) is preserved.

### For-loop-rejuvenation
This rejuvenation locates for-loops that operate over containers such as arrays, and if the for-loop body does not alter the tranversal over the array, replace the standard for-loop with a [ranged-based for-loop](https://en.cppreference.com/w/cpp/language/range-for) added in c++11.


## Requirements to compile Tools
An installed version of rose (0.9.10.64 was used during development), as the tools are compiled to dynamically link the rose libraries.

To compile rose from source follow the instruction on the [rose homepage](http://rosecompiler.org/ROSE_HTML_Reference/group__installation.html) or use this more direct [guide](https://github.com/rose-compiler/rose-develop/blob/master/scripts/2017-03-ROSE-Unbuntu-16.04-VM-setup.sh) which is used to install rose onto a clean Ubuntu VM.

Alternatively you can download the precompiled ROSE binary.

IN ALL CASES REMEMBER TO ADD THE ROSE LIBRARIES TO YOUR PATH SO IT IS VISABLE.

## Disclaimer

All the tools presented are experimental and could have undesired effects, i.e *Rose* does not always preserve orginal source code syntax used when transforming C++ code ([Rose Maual section 5.9](http://www.rosecompiler.org/ROSE_UserManual/ROSE-UserManual.pdf#section.5.9)).

## Useful resources
1. [Rose tutorial](rosecompiler.org/ROSE_Tutorial/ROSE-Tutorial.pdf), has many examples of the different tools that can be build from rose.
2. [Rose user manual](www.rosecompiler.org/ROSE_UserManual/ROSE-UserManual.pdf) 
