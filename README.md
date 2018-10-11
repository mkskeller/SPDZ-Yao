*Note:* [MP-SPDZ](https://github.com/n1analytics/MP-SPDZ) includes additional functionality such as private inputs.

This repository contains code to run computation with Yao's garbled circuits optimized for AES-NI by [Bellare et al.](https://eprint.iacr.org/2013/426).

#### Preface:

The main purpose of this software is to provide a quick way to benchmark the computation of some programs written in a subset of the SPDZ high-level language (using purely `sint` and `sfix`) with Yao's garbled circuits. Private inputs are not supported.

#### Requirements:
 - GCC (tested with 7.2) or LLVM (tested with 3.8)
 - MPIR library, compiled with C++ support (use flag --enable-cxx when running configure)
 - libsodium library, tested against 1.0.11
 - CPU supporting AES-NI, PCLMUL and AVX2
 - Python 2.x
 - If using macOS, Sierra or later

#### Compile the VM:

Run `make yao` (use the flag -j for faster compilation multiple threads).

#### Compile the circuit:

Run `./compile.py -D <program>` to compile the `Programs/Source/<program>.mpc`. See `gc_tutorial.mpc` and `gc_fixed_point_tutorial.mpc` for examples.

#### Run the protocol:

- Run everything locally: `./yao-simulate.x <program>`
- Run on different hosts:
  - Garbler: ```./yao-player.x -p 0 <program>```
  - Evaluator: ```./yao-player.x -p 1 -h <garbler host> <program>```
