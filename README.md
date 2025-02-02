# NuSMV

This is version 2 of NuSMV, the New Symbolic Model Verifier.

- [Copyright](#copyright)
  * [CUDD License](#cudd-license)
  * [MiniSat release 2.2 License](#minisat-release-22-license)
- [Useful Links](#useful-links)
- [Building and Installing NuSMV](#building-and-installing-nusmv)
- [Files in NuSMV Distribution](#files-in-nusmv-distribution)


NuSMV is a re-implementation and extension of SMV, the first model
checker based on BDDs. It has been designed to be an open architecture
for model checking, which can be reliably used for the verification of
industrial designs, as a core for custom verification tools, and as a
test-bed for formal verification techniques.

NuSMV version 2 extends NuSMV with new model checking algorithms and
techniques. It combines classical BDD-based symbolic techniques with
SAT-based techniques. It also presents other new features: for
instance, it allows for a more powerful manipulation of multiple
models; it can generate flat models for the whole language; it allows
for cone of influence reduction.

The BDD-based model checking component exploits the [CUDD][] library
developed by Fabio Somenzi at Colorado University. The SAT-based
model checking component includes an RBC-based Bounded Model
Checker, connected to a SAT solver to be compiled separately
(instructions and building support are batteries included in
NuSMV, details are underneath).

The currently available SAT solver is:

- The [MiniSat][][^1] SAT library developed by Niklas Een or Niklas Sorensson.


NuSMV version 2 is distributed with an OpenSource license, namely the
GNU Lesser General Public License version 2 (LGPL-2). The aim is to
provide a publicly available state-of-the-art symbolic model
checker. With the [OpenSource][] development model, a whole community
participates in the development of a software systems, with a
distributed team and independent peer review. This may result in a
rapid system evolution, and in increased software quality and
reliability: for instance, the OpenSource model has boosted the
take-up of notable software systems, such as Linux and Apache. With
the NuSMV OpenSource project, we would like to reach the same goals
within the model checking community, opening the development of NuSMV.

You can find further details on NuSMV 2 and on the NuSMV project in
paper:

* A. Cimatti, E. Clarke, E. Giunchiglia, F. Giunchiglia,
  M. Pistore, M. Roveri, R. Sebastiani, and A. Tacchella.
  "NuSMV 2: An OpenSource Tool for Symbolic Model Checking".
  In Proc. CAV'02, LNCS. Springer Verlag, 2002.


Please contact <nusmv-users@fbk.eu> for further information on
NuSMV. Please contact <nusmv@fbk.eu> for getting in touch with
the NuSMV development staff.


## Copyright

NuSMV version 2 (NuSMV 2 in short) is licensed under the GNU Lesser
General Public License (LGPL in short). File LGPL-2.1 contains a copy
of the License.

The aim of the NuSMV OpenSource project is to allow the whole model
checking community to participate to the development of NuSMV. To this
purpose, we have chosen a license that:

1. is "copyleft", that is, it requires that anyone who improves the
   system has to make the improvements freely available;
2. permits to use the system in research and commercial applications,
   without restrictions.

In brief, the LGPL license allows anyone to freely download, copy,
use, modify, and redistribute NuSMV 2, proviso that any modification
and/or extension to the library is made publicly available under the
terms of LGPL.

The license also allows the usage of the NuSMV 2 as part of a larger
software system *without* being obliged to distributing the whole
software under LGPL. Also in this case, the modification to NuSMV 2
(*not* to the larger software) should be made available under LGPL.

The precise terms and conditions for copying, distribution and
modification can be found in file LGPL-2.1. You can contact
<nusmv@fbk.eu> if you have any doubt or comment on the
license.

Different partners have participated the initial release of
NuSMV 2. Every source file in the NuSMV 2 distribution contains a
header that acknowledges the developers and the copyright holders for
the file. In particular:

* CMU and ITC-IRST contributed the source code on NuSMV version 1.
* ITC-IRST has also developed several extensions for NuSMV 2.
* ITC-IRST and the University of Trento have developed
  the SAT-based Bounded Model Checking package on NuSMV 2.
* the University of Genova has contributed SIM, a state-of-the-art
  SAT solver used until version 2.5.0, and the RBC package use in
  the Bounded Model Checking algorithms.
* Fondazione Bruno Kessler (FBK) is currenlty the main
  developer and maintainer of NuSMV 2.

The NuSMV team has also received several contributions for different
part of the system. In particular:

 * [Ariel Fuxman](afuxman@cs.toronto.edu) has extended the LTL to SMV
   tableau translator to the past fragment of LTL
 * [Rik Eshuis](eshuis@cs.utwente.nl) has contributed a strong fairness
   model checking algorithm for LTL specifications
 * [Dan Sheridan](dan.sheridan@contact.org.uk) has contributed several
   extensions and enhancements to the Bounded Model Checking algorithms.


### CUDD License

```
*******************************************************************************
Copyright (c) 1995-2004, Regents of the University of Colorado

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

Neither the name of the University of Colorado nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************
```


### MiniSat release 2.2 License

```
MiniSat -- Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
           Copyright (c) 2007-2010  Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```


## Useful Links

* [NuSMV home page][nusmv]
* NuSMV [mailing list][nusmv-ml] page
* [Download page][nusmv-download] of NuSMV:
* Home page of [nuXmv][], the extension over NuSMV to allow for
  synchronous infinite-state systems
* [CUDD][] home page
* [MiniSat][] home page
* [OpenSource][] site
* [GNU General Public License][gpl] home page


## Building and Installing NuSMV

Follow the instruction in [INSTALL.md](./INSTALL.md).


## Files in NuSMV Distribution

[`NEWS.md`](./NEWS.md)
: Contains the change log for the different releases.

`meson.build`
: The top-level input file of the meson building system.

`LGPL-2.1`
: The GNU Lesser General Public License.

`code/`
: Source code packages of the NuSMV system.

`contrib/`
: A collection of useful programs and scripts.
  See [contrib/README](./contrib/README.md) for further information.

`doc/FAQ`
: A collection of Frequently Asked Questions.

`doc/*/*`
: Various NuSMV manuals and tutorial.

`examples/`
: Various collected examples.

`share/nusmv/master.nusmvrc`
: A NuSMV script designed to be ran automatically at start-up: contains aliases
  for commonly used commands and some default parameter settings.

`subprojects/`
: Dependency project managed by meson:

  `packagefiles/cudd-2.4.1*`
  : The CUDD library source tree and patches for NuSMV.

  `packagefiles/minisat-2.2.0.g37dc6c6*`
  : The MiniSat library source tree and patches for NuSMV.

  `prifiler/`
  : Library for profiling BMC executions.

  `watchdog/`
  : Watchdog library.


[REFERENCES]: #

[cudd]: https://github.com/ivmai/cudd "CU Decision Diagram package - Unofficial git mirror"
[minisat]: http://minisat.se "A minimalistic and high-performance SAT solver - Home Page"
[minisat-37dc6c6]: https://github.com/niklasso/minisat/tree/37dc6c67e2af26379d88ce349eb9c4c6160e8543 "Latest available commit of MiniSat"
[nusmv]: http://nusmv.fbk.eu/ "NuSMV Home Page"
[nusmv-ml]: http://nusmv.fbk.eu/mail.html "NuSMV Mailing-List Page"
[nusmv-download]: http://nusmv.fbk.eu/download.html "NuSMV Download Page"
[nuxmv]: http://nuxmv.fbk.eu "nuXmv Home Page"
[opensource]: http://www.opensource.org/ "Opensource site"
[gpl]: http://www.gnu.org/copyleft/ "GNU General Public License Home page"


[FOOTNOTES]: #

[^1]: Since version 2.2.2, NuSMV is able to use the propositional solver
[MiniSat] to deal with the SAT instances generated during a
Bounded-Model-Checking session. Further information about MiniSat can be found
in the paper entitled "An Extensible SAT-solver", by Niklas Een and Niklas
Sorensson. The currently linked version of MiniSat is 2.2, and more precisely a
patched version of SHA-1 commit [37dc6c6][minisat-37dc6c6].
