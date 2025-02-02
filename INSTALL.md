# Compiling and Installing NuSMV

NuSMV is available both as binary executables and source code.
This file contains instructions about the installation process of
binary-distributed files and the compilation process from the source
code.

Table of contents:

- [Binary Distribution](#binary-distribution)
- [Compiling from Source](#compiling-from-source)
  * [Setting Up the Development Environment](#setting-up-the-development-environment)
    + [Generic Requirements](#generic-requirements)
    + [MSYS2 under Windows](#msys2-under-windows)
  * [Building Steps](#building-steps)


## Binary Distribution

Binary distributions are available at [NuSMV][] web site.
NuSMV is distributed as binary executable for the most common
operating systems and architectures:

* Linux x86-64
* MacOS x86-64
* Windows x86-64

To install the binary distribution, download the desired file,
decompress it and run the NuSMV binary from a suitable shell.

Optionally, set the `NUSMV_LIBRARY_PATH` environment variable, e.g., for bash shell:

```sh
export NUSMV_LIBRARY_PATH=/opt/NuSMV-X.Y.Z/share/nusmv
```

> 📝 **Note:** the path `/usr/share/nusmv`, where NuSMV expects to find its
> configuration file `master.nusmvrc`, is hard-coded into NuSMV.

Now you are ready to run and enjoy NuSMV:

```sh
NuSMV -int # to run the interactive mode
```


## Compiling from Source

Unpack the NuSMV's source code pack, which creates the
`NuSMV-<version>/` directory (`NUSMVDIR` from here on).


### Setting Up the Development Environment

#### Generic Requirements

To build NuSMV, you will need:

* An ANSI C compiler (GNU [gcc][] will do, as will several versions of cc)
* An ANSI C++ compiler for building Minisat (g++ will do)
* [Flex][]
* GNU [Bison][]
* GNU [patch][] utility
* One of [libedit][] (Linux), [wineditline][] (Windows), or any BSD-compatible
  alternative (Mac), if you want a shell (and you do).
* [Python][] 3.x
* Building system [meson][] with ninja, version 1.5 or greater
  - Install with `pip install -r requirements.txt` possibly from
    within a Python virtual environment, or
  - See [Getting Meson][meson-get]
* [pkgconf][] or [pkg-config][]
* The most recent version of NuSMV's source code available at [NuSMV][nusmv].


The following dependencies are optional:

* GNU [GMP][]
* Library [libxml2][]
* Library [editline][libedit]

Still optionally if you intend to run the NuSMV test suite:

* Library [GoogleTest][gtest]


If you intend to build the documentation:

* [Doxygen][]
* A fully working LaTeX environment with [latexmk][], such as
  [TeX Live][texlive] or [MiKTeX][]


#### MSYS2 under Windows

Follow the install instructions at [MSYS2][] and then run the
provided script from a 'MSYS2 MSYS' shell, in `NUSMVDIR` directory:

```sh
./script/msys2-setup.sh -yu
```

All required packages will be installed. Use option `-h` for extra packages.

> 📝 **Note:** if you plan to run `NuSMV.exe` out of the MSYS2 environment, add
> option `--prefer-static` to the meson setup step.


### Building Steps

1. Enter a shell within your development environment

2. Enter `NUSMVDIR` directory

3. Configure the building environment (only if not already done):
   ```sh
   meson setup build   # `build` is the name of a directory to be the build root
   ```
   > 📝 **Note:** if you want to change the default path `/usr/share/nusmv`
   > where NuSMV expects to find its configuration file `master.nusmvrc`, add
   > option `-Ddata-dir=<your/path>` to the setup command above.

   > 📝 **Note:** if you want to further customize the setup, use `meson configure`

4. Compile NuSMV with
   ```sh
   meson compile -C build
   ```

   The NuSMV libraries and binaries will be located in `./build` folder.
   The executables can be run directly with no need to install them.

5. Optionally build the documentation:
   ```sh
   meson compile -C build doc
   ```

6. Optionally run the test suite:
   ```sh
   meson test -C build
   ```

7. Optionally, install all libraries and binaries with:
   ```sh
   meson install -C build
   ```


[REFERENCES]: #

[bison]: https://www.gnu.org/software/bison/ "GNU Bison home page"
[doxygen]: https://doxygen.nl "Doxygen home page"
[flex]: https://github.com/westes/flex "Flex repository"
[gcc]: https://gcc.gnu.org "GNU Compiler Collection home page"
[gmp]: https://gmplib.org "GNU Multiple Precision Arithmetic Library home page"
[gtest]: https://google.github.io/googletest/ "GoogleTest home page"
[latexmk]: https://ctan.org/pkg/latexmk/ "CTAN - latexmk"
[libedit]: https://www.thrysoee.dk/editline/ "Editline Library (libedit) home page"
[libxml2]: https://gitlab.gnome.org/GNOME/libxml2 "LibXML2 home page"
[meson-get]: https://mesonbuild.com/Getting-meson.html "Getting meson"
[meson]: https://mesonbuild.com/ "Meson home page"
[miktex]: https://miktex.org "MiKTeX home page"
[msys2]: https://www.msys2.org/ "MSYS2 home page"
[nusmv]: https://nusmv.fbk.eu "NuSMV home page"
[patch]: https://www.gnu.org/software/patch/ "GNU patch"
[pkg-config]: https://www.freedesktop.org/wiki/Software/pkg-config/ "Freedesktop.org's pkg-config home page"
[pkgconf]: https://gitea.treehouse.systems/ariadne/pkgconf "pkgconf home page"
[python]: https://www.python.org "Python home page"
[texlive]: https://tug.org/texlive/ "Tex Live home page"
[wineditline]: https://sourceforge.net/projects/mingweditline/ "WinEditLine - SourceForge"
