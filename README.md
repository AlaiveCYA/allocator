[![CI](https://github.com/AlaiveCYA/allocator/actions/workflows/ci.yml/badge.svg)](https://github.com/AlaiveCYA/allocator/actions/workflows/ci.yml)

# Custom Allocator
A custom allocator project
By Szymon Rzewuski

## Table of contents
  * [General info](#general-info)
  * [Setup](#setup)
  * [Examples](#examples)

## General info
A simple custom implementation of memory allocator similar to malloc.
Allocation is based on first fit algorithm and is using sbrk system call to change heap pointer. All allocations are round up to multiples of 8.
If you are not using clang or gcc you have to call initializeAllocator() before use. Allocator collects statistics from runtime and allows user to print them at the end of runtime or fetch them in struct.
In this library, following functions are shared to user:

mylloc(size)
myfree(ptr)
initializeAllocator()
enableOutput()
disableOutput()
dumpMemory()

## Setup
To use this library you have to do the following:

1. Clone this repository using:
```
$ git clone https://github.com/AlaiveCYA/allocator.git
```
2. Then go into cloned directory:
```
$ cd allocator
```
and  run:
```
$ make install DIR=/path/to/your/installation/folder
```
or:
```
$ chmod +x install_env.sh
$ ./install_env.sh
$ chmod +x install_lib.sh
$ ./install_lib.sh /path/to/your/installation/folder
```
then run
```
$ make
```
to see if everything has been installed properly

## Examples
Here are example programs with utilities:

1. Basic allocation
```
$ #include <mylloc.h>
$ #include <stdio.h>
$ 
$ int main(void){
$ 
$   char* string = (char*)mylloc(sizeof(char)*5);   
$ }
```

2. Basic deallocation
```
$ #include <mylloc.h>
$ #include <stdio.h>
$ 
$ int main(void){
$ 
$   char* string = (char*)mylloc(sizeof(char)*5);
$   myfree(string);   
$ }
```

3. Memory dump
```
$ #include <mylloc.h>
$ #include <stdio.h>
$ 
$ int main(void){
$ 
$   char* string = (char*)mylloc(sizeof(char)*5);
$   dumpMemory();   
$ }
```
