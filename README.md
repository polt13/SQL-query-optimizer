**Requirements**

* CMake
* Makefile
* g++ (with support for C++14 and newer preferrably, although it should work on C++11 as well)
* asan, gdb if compiled with debug flags

**How To Run**

- cmake -B build 
- cd build
- make
- ./Project_JJ_Part1

**NOTE**: To comply with best C++ practices, we're using the C++ version of the headers for all the C libraries we use. Namely, we opted for 'cstdio` over `stdio.h`
and `cstring` over `string.h` (the latter is useful because of `memmove`). Every method is also wrapped in the `std` namespace, which helps avoid conflicts.

**Type Description**

`dataForm.h` contains the definitions for all of the important types. Since the majority of these classes contain methods that are very basic, we chose to include
their implementations directly in the header files for the sake of simplicity.

**Creating relations**

To create a relation, which is what `PartitionedHashJoin` operates on, you first need to create a `tuple` array. Each tuple consists of a key value (on which the Join operation
is performed) and a value (the rowID of the tuple). 

<ins>Because the `tuple` array is expected to be created created during runtime, it should be dynamically allocated on the heap using operator `new`. After passing
the pointer to the allocated array as an argument to the constructor of a `relation`, the user should not attempt to delete the array, as this is automatically
handled by the destructor of the `relation`.</ins>

`test.cpp` contains a few examples on how to create a relation.

**Partitioning**

When it comes to partitioning, you can configure the methods the way you see fit. 

* By default, `partition` decides how many passes it needs to do based on the size of the relation. If you want to force `partition` to not perform partitioning, 1 pass,
or 2 passes you can configure it by setting the 2nd argument of `partition` to `0 (no partition), 1 (1 pass), 2 (2 passes)` respectively. -1 is the default value for the 
2nd argument, which indicates that `partition` automatically determines how many partitions it needs to create, based on the size of the relation.

* `partition` uses the last 2 bits of the key for the first pass of the partitioning and the last 4 bits for the 2nd pass by default. If you want to tweak these values, 
pass them as the final two arguments of `partition`. 

For example, if you need to partition a relation using 4 bits for the first pass and 8 bits for the 2nd (if it comes to it), use `partition(r,-1,4,8)`. If you want to
force the 2nd pass regardless of the size of the relation, use `partition(r,2,4,8)`.

To partition a relation, you need to first create a `Partitioner` object specifically for it. That is because each partitioner maintains an internal state for each relation it partitions.
Namely, during the partitioning phase it generates a histogram and a `psum` array which it later uses to generate the sorted `R'` relation.

Additionally, it maintains an integer which represents how many passes it made. This is useful because both relations need to have equal amount of partitions for PHJ.
When the `r` relationship is partitioned during `PartitionedHashJoin`, we use the afforementioned integer to force the 2nd partitioning to do as many passes as the first
one did.

`Partitioner` holds a pointer to a Histogram object, because a new one needs to be allocated on each pass. When a new one is generated, the previous one is no longer 
of any use - therefore, it can be safely deleted.

The size of the L2 Cache is defined in `partitioner.h`.

