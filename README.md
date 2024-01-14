# Custom Allocator
A custom allocator project
By Szymon Rzewuski

## Table of contents
  [General info](#general-info)
  [Setup](#setup)
  [Examples](#examples)

## General info
A simple custom implementation of memory allocator similar to malloc.
Allocation is based on first fit algorithm and is using sbrk system call to change heap pointer.
All allocations are round up to multiples of 8
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
$ make install
```
or:
```
$ chmod +x install_env.sh
$ ./install_env.sh
$ chmod +x install_lib.sh
$ ./install_lib.sh
```
then run
```
$ make
```
to see if everything has been installed properly

## Examples
Here is basic script
   

