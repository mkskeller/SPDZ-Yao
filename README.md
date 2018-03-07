(C) 2018 University of Bristol, Bar-Ilan University. See License.txt

This repository contains the code to benchmark ORAM in SPDZ-BMR as used for the [Eurocrypt 2018 paper](https://eprint.iacr.org/2017/981) by Marcel Keller and Avishay Yanay.

#### Preface:

This implementation only allows to benchmark the data-dependent phase. The data-independent and function-independent phases are emulated insecurely. The software should be considered an academic prototype, and we will only give advice on rurunning the examples below.

#### Requirements:
 - GCC (tested with 7.2) or LLVM (tested with 3.8)
 - MPIR library, compiled with C++ support (use flag --enable-cxx when running configure)
 - libsodium library, tested against 1.0.11
 - CPU supporting AES-NI and PCLMUL
 - Python 2.x
 - If using macOS, Sierra or later

#### To compile:

1) Edit `CONFIG` or `CONFIG.mine`:

 - Add the following line at the top: `MY_CFLAGS = -DINSECURE`
 - For processors without AVX (e.g., Intel Atom) or for optimization, set `ARCH = -march=<architecture>`.

2) Run `make bmr` (use the flag -j for faster compilation multiple threads). Remember to run `make clean` first after changing `CONFIG` or `CONFIG.mine`.

#### Configure the parameters:

1) Edit `Program/Source/gc_oram.mpc` to change size and to choose Circuit ORAM or linear scan without ORAM.
2) Run `./compile.py -D gc_oram`.

#### Run the protocol:

- Run everything locally: `Scripts/bmr-program-run.sh gc_oram`.
- Run on different hosts: `Scripts/bmr-program-run-remote.sh gc_oram <host1> <host2> [...]`

To run with more than two parties, change `CFLAGS = -DN_PARTIES=<n>` in `CONFIG`, and compile again after `make clean`.
