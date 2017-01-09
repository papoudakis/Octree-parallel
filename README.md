# Octree-pthreads
This is a PThreads and OpenMP implementation of octrees for hashing particles in 3D space. In this implementation we wrote parallel code for 4 functions:

* compute_hash_codes
* morton_encoding
* radix_sort
* data_rearrangement

## Execution
In order to run one PThread version:
```
cd PThreads
make
./test_octree
```
For OpenMp version run:
```
cd OpenMp
make
./test_octree
```

##Results

Plot for  compute_hash_codes:

![hash_cube](https://cloud.githubusercontent.com/assets/9269275/21784133/01012ab0-d6c3-11e6-9dc3-b74cefa39003.png)

Plot for morton_encoding:

![morton_cube](https://cloud.githubusercontent.com/assets/9269275/21784177/1ef59cd6-d6c3-11e6-8e3d-61e0b612cb07.png)

Plot for radix_sort:

![cube_radix](https://cloud.githubusercontent.com/assets/9269275/21784208/35ff294c-d6c3-11e6-8bc9-0762e0f5a175.png)


Plot for data_rearrangement:

![rearranged_cube](https://cloud.githubusercontent.com/assets/9269275/21784229/48018b4e-d6c3-11e6-9522-987beccc57c9.png)

