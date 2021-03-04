# Hedgehog (hh) : A library to generate heterogeneous dataflow graphs

The API is designed to aid in creating task graphs for algorithms to obtain performance across CPUs and multiple co-processors. This library represents the successor of the Hybrid Task Graph Scheduler ([HTGS](https://github.com/usnistgov/HTGS)).

## Dependencies

1) C++20 compiler, with std::filesystem available (gcc 10.2+, llvm 11+) 

2) pthread

3) CUDA (https://developer.nvidia.com/cuda-zone) [optional]

4) GLOG (https://github.com/google/glog) [optional / log]

5) GTEST (https://github.com/google/googletest) [optional / test]

6) doxygen (www.doxygen.org/) [optional / Documentation]

## Building Hedgehog
**CMake Options:**

CMAKE_INSTALL_PREFIX - Where to install Hedgehog (and documentation)

LOG - Enable log with GLOG

BUILD_DOXYGEN - Creates doxygen documentation ('make doc' to generate)

RUN_GTEST - Compiles and runs google unit tests for Hedgehog ('make run-test' to re-run)

BUILD_MAIN - Build main file

```
 :$ cd <Hedgehog_Directory>
 :<Hedgehog_Directory>$ mkdir build && cd build
 :<Hedgehog_Directory>/build$ ccmake ../ (or cmake-gui)

 'Configure' and setup cmake parameters
 'Configure' and 'Build'

 :<Hedgehog_Directory>/build$ make
 :<Hedgehog_Directory>/build$ [sudo] make install
```

## Tutorials

[Hedgehog Tutorials](https://github.com/usnistgov/hedgehog-Tutorials)

# Credits

Alexandre Bardakoff

Timothy Blattner

Walid Keyrouz

Bruno Bachelet

Loïc Yon

Mary Brady

# Contact Us

<a target="_blank" href="mailto:alexandre.bardakoff@nist.gov">Alexandre Bardakoff (alexandre.bardakoff ( at ) nist.gov</a>

<a target="_blank" href="mailto:timothy.blattner@nist.gov">Timothy Blattner (timothy.blattner ( at ) nist.gov</a>
