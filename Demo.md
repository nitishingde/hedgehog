# Demo

## How to build?

```bash
mkdir cmake-build-demo && cd cmake-build-demo
cmake -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" -DRUN_GTEST=OFF -DBUILD_MAIN=ON ../
make
```

There are 2 demo examples.

## Ring communication (example 1)

Let's say there are N processes, 1 on each node
- Process0 sends data to Process1
- Process1 sends data to Process2
- ...
- ProcessN sends data to Process0

How to run it?
```bash
# Make sure to build using the above build script
# go to cmake-build-demo dir
mpirun -np 4 ./main
```

## Tim's example (example 2)

What is the example trying to accomplish?
- Process0 sends data to ProcessN
- Process1 sends data to ProcessN_1
- ...
- ProcessN/2 sends data to ProcessN/2

Each process owns 5 row blocks, of dim (blockWidth = 2, blockHeight = 2).\
Each of these blocks have been filled with their process's node id.\
So process0 will have 5 blocks of 2x2, all having the values 0.\
So process1 will have 5 blocks of 2x2, all having the values 1.\
...\
So processN will have 5 blocks of 2x2, all having the values N.

After receiving these blocks of data, addTask will add 42 to all elements in the block.

How to run it?
```bash
# Make sure to build using the above build script
# go to cmake-build-demo dir
mpirun -np 4 ./demo
```
