# ROSE Castor

Castor is a tool that bridges the ROSE compiler framework with the Why3 verification framework.
Currently, the only supported frontend is C++, but we are looking at more frontends in the future.
Castor in its current state is an in-development tool enabling formal verification of idiomatic C++.
C++ can be written with verification conditions embedded in `#pragma` statements, and Castor will convert the program into a WhyML program which can be verified by the Why3 framework, including automated verification with SMT solvers.
There is currently limited but growing support for C++ features, and a custom verification language for writing verification conditions.

## Building Castor

This project _**requires**_ the installation of:

- The ROSE compiler framework
  - The `rose-config` command must be visible from the command line
- ROSE CodeThorn
  - Installation instructions can be found below
- The Why3 verification framework
  - Installation instructions can be found below
- The Why3find frontend for Why3
  - Installation instructions can be found below
- A C++ compiler supporting C++17
- C++ Boost (>1.66)
- CMake (>3.22)
- At least one SMT solver, but ideally more; many (all?) of these are available through `opam`
  - `Alt-Ergo` is **highly recommended** and can be found [here](https://alt-ergo.ocamlpro.com/#releases)
  - `CVC5` is **highly recommended** and can be found [here](https://github.com/cvc5/cvc5)
  - `Z3` is recommended and can be found [here](https://github.com/Z3Prover/z3)
  - `CVC4` is optional and can be found [here](https://github.com/CVC4/CVC4-archived)
  - `Vampire` is optional and can be found [here](https://github.com/vprover/vampire)

To install CodeThorn, first, navigate to your ROSE build tree (e.g., `rose/buildTree`, or whatever directory you ran `configure` from.
Then:

```sh
cd tools/CodeThorn
make install
```

To install Why3 and Why3find properly, first ensure that you have `opam` installed.
You can find how to install opam [here](https://opam.ocaml.org/doc/Install.html).
After `opam` is visible:

```sh
opam init
opam switch create castor-switch 4.14.1
eval $(opam env --switch=castor-switch) # We recommend adding this to your ~/.bashrc
opam install why3
git clone https://git.frama-c.com/pub/why3find.git
cd why3find
opam install .
```

Please make sure all of these are installed and visible in your `$PATH` before proceeding.

After cloning and navigating to the root of the repository:

```sh
git submodule init
git submodule update
mkdir build 
cd build
cmake ..
make
make install
```

Then, make sure to initialize Why3:

```sh
why3 config detect
```

There are some additional flags you can optionally pass to CMake

|          Flag                          |                    Description                                        |
|----------------------------------------|-----------------------------------------------------------------------|
| `CTEST_TIMEOUT`                        | Specifies the timeout for running automated tests                     |
| `CTEST_NEGATIVE_TIMEOUT`               | Specifies how long to attempt to prove negative tests                 |
| `SMOKE_TEST`                           | `true`/`false`, enable or disable smoke tests                         |
| `CODETHORN_INCLUDE_DIR`                | The path containing CodeThorn header files                            |
| `CODETHORN_LIB_DIR`                    | The path containing CodeThorn shared libraries                        |
| `SUPER_DANGEROUS_DYNAMIC_MEMORY_ALLOC` | Enable **unsound** dynamic memory allocation support via `new`/`free` |

## Testing Castor

You can run the automated test suite by running
```sh
make test
```

The `-DCTEST_TIMEOUT` flag for CMake sets the timeout for the Castor when running tests.
**Some tests might run for much longer than the timeout given.** 
Using a 120 second timeout, the longest test took several minutes on our test machine.
See the section on "Using Castor" for why this is.

The tests are expected to take a long time to complete.
On our test machine, the tests take **several hours** to pass.
A timeout of 120 was sufficient to get all tests to pass on our test machine, but you may find that a longer timeout is necessary,
depending on the SMT solvers installed, their versions, and your hardware configuration.

## Using Castor

After building Castor, you can do
```sh
castor help
```
to view the main help page.

There are three main subcommands in Castor, viewable from this top-level help menu.
```sh
castor prove ...   # prove a file with Castor
castor compile ... # compile a file with Castor only after successfully proving it
castor parse ...   # parse a file with Castor without proving it
```
From each subcommand you can use the `--help` flag to view individual command-line switches supported for that subcommand.

Example usage:
```sh
castor help
castor parse --help
castor parse my_program.cpp --debug --generate-whyml
castor prove my_program.cpp --transformer=sp --standard=c++11 -t 30
```

## Documentation

To generate Doxygen documentation, you can run

```sh
doxygen Doxyfile
# folder now exists at ./doxygen
```

## Contact

For questions or comments that do not fit under a GitHub issue, the authors may be contacted at `castor@llnl.gov`.

## Release

This code is released under the BSD 3-Clause License.

LLNL-CODE-2017292

SPDX-License-Identifier: BSD-3-Clause
