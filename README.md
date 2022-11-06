## Contributors
<table>
  <tbody>
    <tr>
      <td align="center"><a href="https://github.com/polt13"><img src="https://avatars.githubusercontent.com/u/59566389?v=4" width="100px;" alt="Polydoros Tamtamis"/><br /><sub><b>Polydoros Tamtamis</b></sub></a><br /><sub>sdi1900184</sub></td>
      <td align="center"><a href="https://github.com/chrisioan"><img src="https://avatars.githubusercontent.com/u/75933777?v=4" width="100px;" alt="Christos Ioannou"/><br /><sub><b>Christos Ioannou</b></sub></a><br /><sub>sdi1900222</sub></td>
    </tr>
  </tbody>
</table>
<br></br>

## Table of contents
* [Requirements](#requirements)
* [How To Run](#how-to-run)
* [Type Description](#type-description)
* [Creating Relations](#creating-relations)
* [Partitioning](#partitioning)
* [Hash Table](#hash-table)
<br/><br/>

## Requirements

* CMake
  ```sh
  sudo apt install cmake
  ```
* Make
  ```sh
  sudo apt install make
  ```
* Compiler with support for C++11 or newer
  ```sh
  sudo apt install g++
  ```
* ASan, gdb if compiled with debug flags
  ```sh
  sudo apt install libasan4-dbg
  ```
  ```sh
  sudo apt install gdb
  ```
<br/><br/>

## How To Run

  ```sh
  cmake -B build
  ```
<!-- tsk -->
  ```sh
  cd build/
  ```
<!-- tsk -->
  ```sh
  make
  ```
<!-- tsk -->
  ```sh
  ./Project_JJ_Part1
  ```

**NOTE**: To comply with best C++ practices, we're using the C++ version of the headers for all the C libraries we use. Namely, we opted for `cstdio` over `stdio.h`
and `cstring` over `string.h` (the latter is useful because of `memmove`). Every method is also wrapped in the `std` namespace, which helps avoid conflicts.
<br/><br/>

## Type Description

`dataForm.h` contains the definitions for all of the important types. Since the majority of these classes contain methods that are very basic, we chose to include
their implementations directly in the header files for the sake of simplicity.
<br/><br/>

## Creating Relations

To create a relation, which is what `PartitionedHashJoin` operates on, you first need to create a `tuple` array. Each tuple consists of a key value (on which the Join operation
is performed) and a value (the rowID of the tuple). 

<ins>Because the `tuple` array is expected to be created created during runtime, it should be dynamically allocated on the heap using operator `new`. After passing
the pointer to the allocated array as an argument to the constructor of a `relation`, the user should not attempt to delete the array, as this is automatically
handled by the destructor of the `relation`.</ins>

`test.cpp` contains a few examples on how to create a relation.
<br/><br/>

## Partitioning

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
<br/><br/>

## Hash Table

As it has been discussed in class' Piazza Forum, the ``Hash Table`` is implemented as a ``circle``, which means when we get to the "last" bucket, the next one is the very first one. To do that, we have to apply ``modulo (%) num_buckets`` to the index, where *num_buckets* indicates the *HT's size*.

- For example, if **HT's size = 60** and **NBHD_SIZE = 32** then, **bucket[59] neighbourhood** consists of **bucket[59], bucket[0], ... bucket[30]**.

Same goes when we're on the "first" bucket and want to go further back.

- For example: If **HT's size = 60**, we are on **bucket[4]** and want to **go back 8 slots**, then we end up on **bucket[56]**.

In case a ``rehash`` is needed, we simply ``increase the HT's size by doubling it and adding 1``.
*All the tuples have to be re-inserted*.

Furthermore, there is a chance that ``multiple tuples with same keys but different rowIDs`` are inserted into the Hash Table. If the amount of such tuples **exceeds** the ``NBHD_SIZE``, then rehashing does not help at all (Neighbourhood will remain full). For this reason, we have implemented a ``Linked List`` (chaining) where, if the exact same key is found, the whole tuple is appended to the list. This List is part of ``class bucket`` fields.

The size of the Neighbourhood is defined in ``hashtable.h`` as ``NBHD_SIZE``.

**NOTE:** Some of the unit tests (e.g Full HT Insert, Swap HT Insert) will fail if NBHD_SIZE is changed because they are based on that specific size.
*Nevertheless, the whole implementation works just fine*.
