********************** NuSMV 2.7.0 (2024/10/25) **********************

## 2.7.0 - 2024-10-25

This is a major release that provides some internal improvements and
several bug fixes.

The main changes are:

 * Switched building system to [meson](https://mesonbuild.com/).

 * Replacement of readline library with
   [editline](https://www.thrysoee.dk/editline/) (Linux),
   [wineditline](https://sourceforge.net/projects/mingweditline/)
   (Windows), or any BSD-compatible alternative (Mac), due to licensing
   issues.

 * Removed support to zChaff solver.


### New features

 * System commands

    - The command `print_clusterinfo` has been removed.

 * Command line options and system variables

    - New variable `expand_wordarray`.

    - New variable `traces_regexp`.


## 2.6.0 - 2015-10-14

This is a major release that comes after four years passed working
under the surface. The release provides some new features, many bug
fixes and optimizations, and substantial differences in the software
architecture and building system.

The main changes are:

* Splitting source code in two main components: the Core NuSMV library
  and the NuSMV Shell. This split enables for a much easier
  integration with other tools, and for a possible reorganization of
  the shell in the future. This change breaks backward compatibility
  of APIs and source code integration.

* Switch to cmake (http://cmake.org) for building source code. This
  allows for a much faster build wrt previously used GNU Autotools,
  higher portability, and lower maintenance costs.

The bug fixes and new features are described below.

Please consider switching to [nuXmv](https://nuxmv.fbk.eu).  nuXmv
extends NuSMV along two directions. For the verification of
finite-state synchronous designs, nuXmv features strong engines based
on state-of-the-art SAT-based algorithms (see the final results at the
[Hardware Model Checking Competition 2015](http://fmv.jku.at/hwmcc15/)).
For the verification of infinite-state synchronous designs, nuXmv
features state-of-the-art SMT-based verification techniques, implemented
through a tight integration with the [MathSAT5](http://mathsat.fbk.eu)
SMT solver. For details about license terms see nuXmv's web pages.


### New features

  * Language

    - NuSMV language was extended to support the `min`, `max`, and `abs`
      operators.

  * System commands

    - Added four new commands `check_pslspec_bmc`,
      `check_pslspec_bmc_inc`, `check_pslspec_sbmc`,
      `check_pslspec_sbmc_inc`. They handle the options `-b` (use bmc),
      `-s` (use sbmc), `-i` (use incremental algorithms), `-c` and `-N`
      previously handled by the single command `check_pslspec`, that
      now is specific for bdd-based model checking.

    - Added command `convert_property_to_invar` which converts LTL
      (CTL) properties in the form "G (AG) next-expr" to INVARSPEC.

    - Command `print_fair_transitions` now provides exact information
      about the fair-transitions, and not about state-input pairs. The
      command provides support also for dumping information in DOT and
      CSV formats, which is useful to visually "inspect" the FSM.

  * Command line options and system variables

    - New variable `boolean_conversion_uses_predicate_normalization`:
      When set to true, performs predicate normalization of any
      expression before performing the booleanization. There are cases
      where enabling this option pays off dramatically, however it
      might be expensive on other cases since the normalization of the
      predicates may blow up the formula. Set to false by default.

    - New variable `ltl2smv_single_justice`: When set to true, the
      tableau construction builds a degeneralized (single fairness)
      symbolic encoding of the Buchi automaton within `ltl2smv` instead
      of the default generalized (multiple fairness) one.  Added
      command line option `-s` to `ltl2smv`, to enable the emit of a
      single justice tableau.

    - New variables `pp_cpp_path` and `pp_m4_path` to allow users to
      explicitly set the paths on the file system of `cpp` and `m4`
      preprocessors.

  * Improvements (internal)

    - Introduced infinite-width words. In previous versione, maximum
      width was limited to 64.

    - RBC inlining now uses a LRU cache which always improves BMC
      performances (up to 30% according to our experiments).

    - Many functions were rewritten to avoid recursion. This allows
      for a faster executions with more limited use of stack.

    - Command `help` no longer relies on external files for accessing
      the commands' documentation. This simplify the usage.


### Bug fixes

There were about 270 bug fixes, most of them were minor, but some were
major issues. Here is the list of the most critical bug fixed:

* Fixes in type checker.

* Fixed some problems related to array handling appearing in some
  corner case use.

* Fixed several issues with portability, in particular to allow
  compilation of core libraray with msvc-2010.


### Documentation

Programmer's manual is now generated with
[Doxygen](http://www.doxygen.org). For this reason documentation of all
functions has been moved to the place of declaration (i.e. to header
files). Previous documentation system has been dropped.


### Source Code

Top level directory has been renamed from `nusmv` to `NuSMV`.  Source
code is now located in `NuSMV/code/nusmv` with `NuSMV/code` in the
inclusion path to allow for explicit inclusion of header files in the
form `nusmv/...` which makes explicit where each header files is
imported from.

Core library and shell have been clearly split into
`NuSMV/code/nusmv/{core,shell}`, and it is now possible to compile the
library standalone (option `ENABLE_SHELL=OFF` at configuration time).


## 2.5.4 - 2011-10-31

This is a minor release that provides some new features, some internal
improvements, and many bug fixes.


### New features

  * System commands

    - Added two new commands `bmc_pick_state` and `bmc_inc_simulate`
      for interactive simulations using SAT;

    - Added option `-n "name"` to command add_property to specify the name

    - Removed unsupported options `-o` and `-1` from the documentation
      command `check_ltlspec_sbmc_inc`;

  * Command line options and system variables

    - Added a new system boolean variable `daggifier_enabled`
      (Default set to 1), and a new relative command line option
      `-disable_daggifier` to enable/disable the daggifier while
      dumping SMV models.

  * Improvements (internal)

    - Performed several refactorings: added wff package and a few
      basic data structures (e.g., `utils/Pair.c`, `utils/Triple.c`);

    - The simplification of the expressions has been improved using
      additional simplification rules, the local symbol tables and
      pointers sorting;

    - Improved iteration over the hash tables elements;

    - Improved construction of static BDD var order.


### Bug fixes

We fixed about 15 bugs in this version. Many thanks to those users who
reported issues, and helped improving NuSMV.

Here is the list of the most critical bug fixed:

  - Fixed a wrong simplification rule within the rbc package;

  - Fixed a bug in condition simplification, which was not taking
    into account context;

  - Fixed a bug in prompt printing that was creating some syncronization
    problem with external tools (thanks to Sylvain Soliman from INRIA for
    the accurate bug report);

  - Fixed a bug in the trace reader that caused seg-faults;

  - Fixed an overflow in the operator swconst;

  - Fixed a bug in the command add_property that was wrongly adding a
    property P even if P was already used;

  - Added some additional checks to trap uncorrect PSL formulae;

  - Fixed a linking dependence about the library `librbcdag.la` in the
    Makefile that prevented the creation of linkable library.


### Documentation

  - The output of `-h` option for `check_ltlspec_sbmc_inc`,
    `bmc_pick_state`, `bmc_inc_simulate`, `add_property`,
    `daggifier_enabled` commands, and the command line option
    `-disable_daggifier` have been updated according to the recent
    changes. The User Manual has been updated as well.


## 2.5.3 - 2011-06-16

This is a minor release that provides many bug fixes, some
internals improvement, and a few new features.


### New features

  * System commands

    - Command `compute` has been renamed `check_compute`. Old
      command `compute` is deprecated.

    - Command `clean_bdd_cache` has been renamed `clean_sexp2bdd_cache`

    - Added option `-F <format>` to command `show_property`, to print in
      tabular or XML formats.

    - Added options `-t` and `-V` to command `show_vars`, to print selected
      information about the variables in the model.

    - Added option `-k <length>` to command `simulate`, to make
      uniform all simulation commands.
      Old syntax `simulate <length>` is now deprecated.

    - Added options `-r`, `-c <simp_constr>` and `-t <next_constr>` to
      SAT-based simulation commands, both incremental and non
      incremental, in order to be more uniform with regard to the
      BDD-based ones.

  * Command line options and system variables

    - Command line option and system variable `disable_bdd_cache`
      have been renamed to `disable_sexp2bdd_caching`

    - New command line option and system variable
      `disable_syntactic_checks` which can improve performances by
      skipping checks on input files which are assumed to be
      syntactically correct by construction.

    - New command line option and system variable
      `keep_single_value_vars` to disable an optimization which by
      default converts enumerative variables to constants when
      their domain contains only a single element.

    - New system variable `default_simulation_steps` is used by
      simulation commands. Command `simulate` now considers it if
      simulation steps are not specified; command `bmc_simulate`
      considers it instead of variable `bmc_length`.

    - New system variable `parser_is_lax` can be used to force the
      parser to try completing parsing even with syntactic errors,
      reporting all found errors at the end.

    - New system variable `prop_print_method` can be used to select
      how properties are printed out.


  * Improvements (internals)

    - New algorithms to compute COI (Cone of Influence) have been
      implemented.

    - Encoding of scalar variables have been improved to fit with
      the BDD variable ordering. The new encoding proved to improve
      performances in many general cases.

    - Fixed BDD encoding of variables to trigger less re-orderings.

    - Some RBC operations now use much less stack space.

    - New Symbol Table implementation, with huge performances and
      memory improvements.

    - New system-widely-used NodeList implementation, which reduces
      global NuSMV memory usage.


### Bug fixes

About 30 bugs were fixed in this version. Many thanks to those
users who reported issues, and helped improving NuSMV.

Here the most critical bug fixes are listed:

 - Fixed a bug in CTL counter example generation, which was
   surfacing in some cases.

 - Fixed a bug which was triggered when adding incorrect properties
   to the properties database.

 - Fixed bug in command `bmc_pick_state` when used with COI.

 - Fixed compilation errors on recent GNU/Linux distributions

 - Fixed compilation errors under Darwin.

 - Fixed many memory leaks.


### Documentation

  - Added a FAQ containing a set of most frequently asked questions
    about NuSMV, and the SMV language.

  - Added to the User Manual the description of some missing
    operators like `sizeof`


## 2.5.2 - 2010-10-29

This is a minor release that provides a few new features, and several
bug fixes.


### New minor features

  * SMV grammar

    - Array indices in PSL expressions can now be generic expressions,
      like in normal SMV grammar. Previously, only constant numbers
      were supported.

  * Environment variables

    - Some traces related environment variables have been renamed to
      improve usability:

      * counter_examples_hiding_prefix  traces_hiding_prefix
      * counter_examples_show_re -> traces_regexp
      * show_defines_in_traces -> traces_show_defines
      * trace_show_defines_with_next -> traces_show_defines_with_next


### Bug fixes

   - Fixed a problem, causing segfault, in the way the COUNT operator
     was handled. Thanks to Alexandre Arnold <a.arnold.fr AT gmail.com>
     for reporing the issue.

   - Fixed an assertion violation occurring in interactive simulation
     when an incorrect constraint was given to restrict the number of
     possible next states to be shown to the user.

   - Fixed three problems in the trace visualization:
     * Word constants in decimal format were not properly printed
       under windows 32bit machines.
     * In some cases the option specifying the format for printing
       word constants was ignored.
     * In some cases the options specifying the filters over signals
       were ignored.

   - Fixed a problem of command execute_partial_traces that was not
     reporting any message to the user for non re-executable traces.

   - Fixed a bug in command write_coi_model which was generating
     an incorrect model for properties with an empty coi.

   - Fixed a problem of the `echo` command that was no longer
     expanding references to environment variables.

   - Fixed a problem that was causing NuSMV to crash when handling
     command line options related to BMC and no SAT solver was
     linked/detected at compile time.

   - Fixed several other small bugs on internals not affecting the the
     normal user interacting with NuSMV via the shell or in batch mode.


## 2.5.1 - 2010-10-01

This is a minor release that provides most notably a major change in
the language. It features also several bug fixes (more than 30), a few
new operators in the SMV language, some new command line options, a
few optimizations, and some fixes in the documentation.

The main changes are listed below. For details please refer to the
ChangeLog.


### New features

  * SMV grammar

    - The new version 2.5.1 makes a strong distinction between
      integers and boolean types. This breaks backward compatibility,
      since integers values cannot be used anymore where booleans are
      used, and vice-versa.

      To mitigate the problems that may arise from this change, new
      operators have been introduced (e.g., the `count` operator), and
      the signature of existing ones have been extended (e.g., the
      `toint` operator) to allow to cope with no longer supported
      expressions.

      Thus, the following typical snipped of code are no longer
      accepted:

      1. Given N boolean expressions b1, ..., bN, to specify that at most
         M out of N must be TRUE, before it was possible to write
         ```
              (b1 + b2 + ... + bN) <= M.
         ```

         From 2.5.1 on this condition has to be specified as
         ```
              count(b1,b2,...,bN) <= M.
         ```

      2. Constant `1` cannot be used anymore as default condition into
         a `case` statement. Thus the following code snipped
         ```
            case
              b1 : e1;
              ...
              bN : eN;
              1  : eD; -- expression for default condition
            esac
         ```

         has to be rewritten as
         ```
            case
              b1 : e1;
              ...
              bN : eN;
              TRUE : eD; -- expression for default condition
            esac
         ```

         In this snippet b1, ..., bN are boolean expressions
         representing the conditions, and e1, ..., eN, eD are the
         expressions the case construct evaluates to when the
         corresponding condition evaluates to TRUE.

    - Added the `count()` operator, which takes a list of boolean
      expressions and returns the number of expressions that evaluates
      to TRUE.

    - Operator `toint()` now supports word type expressions.

  * System commands

    - Command `show_vars` no longer prints by default all the
      values of an enumerative variable, and truncates it if too
      long to be printed, and reports the number of the remaining
      elements. The new command line option `-v` for this command
      enables the previous behavior.

    - Command `set` now accepts `0` and `1` when setting values of
      boolean system variables. Now `set <var> 0` has the same
      effect as `unset <var>`, and `set <var> 1` has the same
      effect as `set <var>`.

  * Command line options

    - The backward-compatibility option `-old` has been extended to
      make NuSMV accept syntactical construct `1` in `case` conditions
      to specify the default condition. This is intended to mitigate
      the impact of the strong type distinction between integer and
      boolean types, in existing SMV models. This feature will be
      removed in future releases.


### Bug fixes

  - Garbage in dimacs file under Windows platforms. Thanks to Luca
    Zanetti <luzanetti AT fbk.eu> for repoting it.

  - Standard output and error, and the shell's prompt are now in
    sync under Windows. Thanks to Aleksey Nogin <anogin AT hrl.com>
    for the report and for providing a patch.

  - The parser can handle now comments with length greater than
    16381 characters.

  - The type checker can now successfully detect ambiguities in
    identifiers which refer to both enumerative literals and
    variable names.

  - Fixed a crash which was in some cases occurring when extracting
    the counter-example of a false property, with cone of influence
    was enabled.

  - Fixed some memory leaks.

  - Fixed several portability issues.


## 2.5.0 - 2010-05-17

Version 2.5.0 is a major release which provides new features, many
optimizations and bug fixes, and an extensive reorganization of the
source code and API.

Many changes were driven by projects which involved
FBK, like:

- [COCONUT](http://www.coconut-project.eu/), COrrect-by-CONstrUcTion
  Workbench for Design and Verification of Embedded Systems.
  COCONUT was founded by EU FP7-2007-IST-1-217069.

- [COMPASS](http://compass.informatik.rwth-aachen.de), COrrectness,
  Modeling and Performance of Aerospace SystemS

- [EuRailCheck](http://es.fbk.eu/projects/eurailcheck), European
  Railways Verification and Validation Tool.

In all these projects, the core of NuSMV has been improved to deal
with large scalability issues. Several bottlenecks were identified,
and fixed, with particular care about the memory usage.

The main changes are listed below. For details please refer to the
ChangeLog.


### New features

  * Improvements

    - New type "signed word" is introduced

    - New type is added to allow signed arithmetic.
      Also a few operations related to signed words are
      introduced. They are "extend" and two casts - "signed" and
      "unsigned". See user manual for more details.

    - Added traces re-execution engine. Traces can now be executed
      within the model, to check whether or not the trace contains
      only valid assignments that are feasible in the model.

  * SMV grammar

    - Properties can now be named in the SMV model.
      The syntax for property naming is `NAME pname := expr`

  * PSL grammar

    - The PSL grammar now supports word operations. All SMV word
      operators are supported, made exception for the bit-selection
      operator, which has a different syntax in PSL [select(w, h, l)].
      See user manual for more details

  * System commands

    - Added the `bmc_pick_state` command. Similarly to `pick_state`,
      chooses an element from the set of initial states, and makes it
      the current state. This must be done in order to proceed with
      BMC simulation

    - Added the `bmc_inc_simulate` command. Does the same as
      `bmc_simulate`, but using incremental algorithms.

    - Added the `bmc_simulate_check_feasible_constraints` command,
      used to check whether or not a given constraint is feasible and
      can thus be used as BMC simulation constraint.

    - Added the `check_invar_bmc_inc` command, used for checking
      invariant specifications using BMC incremental algorithms.

    - Added the `check_ltlspec_bmc_inc` command, used for checking
      LTL specifications using BMC incremental algorithms.

    - Added the `clean_bdd_cache` command.  Cleans the cached results
      of evaluations of symbolic expressions to ADD and BDD
      representations.

    - Added the `execute_partial_traces` command, used for incomplete
      trace execution

    - Added the `execute_traces` command, used for complete trace
      execution

    - Added the `print_formula` command, which prints a formula in
      canonical format.

    - Added the `show_dependencies` command, which shows the
      dependencies for the given expression

    - Added the `write_coi_model` command, which reduces a model using
      the Cone Of Influence over a given property and dumps it on a
      file.

    - Added the `write_flat_model_udg` command, which writes the input
      given SMV model in the speciﬁed uDraw ﬁle

    - Added the `write_pred_clusters_model` command, which writes flat
      models corresponding to clusters of predicates to file.

    - Added the "-P name" command option to all property-checking
      commands (check_ctlspec, check_ltlspec, ...). When using this
      option, only the property named `name` is checked.

    - Added some new command options to the `check_invar` command:

        * -s strategy  : Force the analysis strategy (Overrides the
                         `check_invar_strategy` environment
                         variable). Possible values are {`forward`,
                         `backward`, `forward-backward`, `bdd-bmc`}

        * -e heuristic : Force the heuristic in case of
                         forward-backward strategy (Overrides the
                         `check_invar_forward_backward_heuristic`
                         environment variable). Possible values are
                         {`zigzag`, `smallest`}

        * -t number    : When using the mixed BDD and BMC approach
                         specify the heuristic threshold. Overrides
                         the `check_invar_bddbmc_threshold`
                         environment variable.

        * -k number   : When using the mixed BDD and BMC approach
                         specify the BMC max k. Overrides the
                         `bmc_length` environment variable

        * -j heuristic : Specify the heuristic used to switch from BDD
                         to BMC (Overrides the
                         `check_invar_bddbmc_heuristic` environment
                         variable). Possible values are {`steps`,
                         `size`}

    - Added "-k number" options and "-t time" option to the
      `compute_reachable` command, which respectively limit the
      forward search in number of steps or in execution time.

    - Added `-2` and `-n` options to the `echo` command. `-2` prints
      the given string to stderr instead of stdout. `-n` does not add
      a trailing newline to the printed string.

    - Added the `-d` option to the `flatten_hierarchy` command. This
      option delays the construction of vars constraints until needed

    - Added the `-p` option to the `print_fsm_stats` command, which
      prints out the normalized predicates the FSM consists of.

    - Added a few command options to the `print_reachable_states`
      command:

        * -d          : Prints the list of reachable states with defines
                        (Requires -v)

        * -f          : Prints the formula representing the reachable
                        states.

        * -o filename : Prints the result on the specified filename
                        instead of on standard output

  * Command line options

    - Added option "-source sc_file": Executes NuSMV commands from
      file sc_file

    - Added option `-disable_bdd_cache` to disable bdd reachable
      states caching.

    - Added option "-bdd_soh heuristics" to set the heuristics to be
      used to compute BDD ordering statically by analyzing the input
      model.

    - Added option "-ojeba alg" to set the algorthim used for
      BDD-based language emptiness of Buchi fair transition systems.

  * System variables

    - System variables are now printed in a lexicographic order when
      calling command `set`

    - Added system variable `bdd_static_order_heuristics`, which sets
      the heuristics for guessing a good ordering by analyzing the
      input model.

    - Added system variable `bmc_inc_invar_alg`, which sets the
      default algorthim for BMC incremental invariants checking.

    - Added system variable `check_invar_bddbmc_heuristic`, which sets
      the bdd-bmc heuristics to be used when using bdd-bmc BDD
      invariants checking strategy

    - Added system variable `check_invar_bddbmc_threshold`, which sets
      the threshold to be used when using bdd-bmc BDD invariants
      checking strategy

    - Added system variable `check_invar_forward_backward_heuristic`,
      which sets the heuristics used for forward-backward strategy
      when checking invariants using BDD.

    - Added system variable `check_invar_strategy`, which sets the
      strategy to be used for BDD based invariants specifications
      checking.

    - Added system variable `counter_examples_hiding_prefix`. Symbols
      names that match this string prefix will be not printed out when
      showing a trace

    - Addes system variable `counter_examples_show_re`. Only symbols
      whose names match this regular expression will be printed out
      when showing a trace.

    - Added system variable `daggifier_counter_threshold`,
      `daggifier_depth_threshold` and `daggifier_statistics`, which
      control the model dumper behavior.

    - Added system variable `enable_bdd_cache`, which determines if
      during evaluation of symbolic expression to ADD and BDD
      representations the obtained results are cached or not.

    - Added system variable `oreg_justice_emptiness_bdd_algorithm`,
      which sets the algorithm used to determine language emptiness of
      a Buchi fair transition system.

    - Added system variable `rbc_rbc2cnf_algorithm`, which defines the
      algorithm used for conversion from RBC to CNF format.

    - Added system variable `show_defines_in_traces`, which controls
      whether defines should be printed as part of a trace or be
      skipped

    - Added system variable `trace_show_defines_with_next`, which
      controls wheter defines containing next operator should be
      printed as part of a trace or be skipped

    - Added system variable `use_coi_size_sorting`, which
      enables/disables the use of the cone of influence variables set
      size for properties sorting, before the verification step.


### Bug fixes

  - Fixed the precedence of `mod` operator. Now it has the same
    precedence as division instead of having precedence
    between +/- and shift.


## 2.4.3 - 2007-05-22

This is a minor release that provides several bug fixes and
commands completion when interactive shell is used.

### New features

  * User interaction

    - Added commands completion functionality to the interactive shell
      (requires readline library to be enabled and to be available at
      compile time). Thanks to Tivadar Szemethy <tivadar.szemethy AT
      vanderbilt.edu> who contributed the code for this feature.


### Bug fixes

  - Fixed a failure that may occur when model checking LTL formulae
    with defines referring directly or indirectly to input variables.

  - Fixed a critical bug that made formula (AF p | AF q) being
    rewritten as AF (p | q).

  - Fixed a critical bug that made NuSMV fail when allocating more
    than 4096 BDD variables.

  - SBMC was not handling correctly past temporal operators.

  - Operator unary minus (-) was not fully handled by type checker
    and other components.

  - Fixed a bug that allowed the properties database to contain
    properties with problems (undefined symbols, type errors, ...).

  - Use of keyword `self` as module's actual parameter has been
    re-enabled.

  - Fixed some memory leaks.


## 2.4.2 - 2007-04-06

This is a minor release that provides compatibility with 64-bit
architectures, speed improvements, a few extensions to the user
interface and many bug fixes. Latest versions of CUDD and Minisat
are also now used. Overall this release results on average
faster than previous ones.


### New features

  * Improvements

    - Specifications using words are bit-blasted using the circuits
      corresponding to the functions occurring in the specification
      when SAT based BMC is selected for verification. This enabled
      the verification of models out of the scope of previous versions
      of NuSMV.

    - SAT based BMC encoding now features RBC inlining [ABE04] to
      reduce problem size. With RBC inlining enabled, our experiments
      showed on average a reduction of total model checking time.

      RBC inlining has also been extended to symbolic SEXP encoding
      to possibly benefit from its use in BDD based searches.

      This inlining is not enabled by default since it does not
      provide significant improvements on average, and in some cases
      it results in degraded performance.

      We remark that inlining might result in a dramatic degrade of
      performances in some cases, depending on the nature and structure
      of the problem.

      [ABE04] P. A. Abdulla, P. Bjesse and N. Eén: Symbolic
         Reachability Analysis Based on SAT-Solvers. In Proceedings of
         Tools and Algorithms for Construction and Analysis of
         Systems, 6th International Conference, TACAS 2000. Pages
         411-425. Lecture Notes in Computer Science 1785. Springer.

    - SAT based BMC uses an extended version of the efficient RBC to
      CNF conversion described in [She04]. The new CNF conversion
      leads to smaller CNF than previous one, and to an average
      improvement in verification.  Thanks to Daniel Sheridan for the
      preliminary version of the CNF conversion, and to Gavin Keighren
      for the preliminary porting to the new version of NuSMV.

      [She04] D. Sheridan. The Optimality of a Fast CNF conversion and
         its use with SAT. In Proceedings of the 7th International
         Conference on Theory and Applications of Satisfiability
         Testing (SAT'2004), 10-13 May 2004, Vancouver, BC, Canada.

    - A patched version of CUDD-2.4.1 is now linked to NuSMV. Previous
      version was a patched version of CUDD-2.3.0.

    - Minisat2-061208 is now linked to NuSMV. Previous version was
      Minisat-1.14. By default NuSMV now uses the MiniSAT extended
      solver with simplification capabilities enabled.

    - NuSMV has been ported to 64-bit architectures. It can run and
      compile smoothly on Intel Xeon(TM), AMD Opteron(TM), and Intel
      IA64(TM) architectures.

      We would like to thank Moshe Vardi, Kim Andrews, Franco M. Bladilo,
      and Jan E. Odegard from Rice University for providing us with
      cray facilities for testing the porting. The Cray computing
      facility at Rice substantially contributed to the completion of
      the 64-bit porting of NuSMV.

  * User interaction

    - Variable `sexp_inlining` is used to enable/disable inlining
      at sexp level. Sexp inlining at the moment is applied only
      when BMC is performed. By default Sexp inlining is disabled.

    - Option -sin on|off to control new system variable sexp_inlining.

    - Variable `rbc_inlining` is used to enable/disable RBC
      inlining before committing a problem to any SAT solver. By
      default RBC inlining is enabled.

    - Option -rin on|off to control new system variable rbc_inlining.


### Bug fixes

  - Fixed a bug that led to cyclic assignments not being correctly
    detected. Thanks to Michael Whalen <mwwhalen AT
    rockwellcollins com> and Li Zhong <gravityzhong AT gmail
    com> for reporting the bug.

  - Fixed a critical bug that made the booleanizer cache not take
    into account the contexts of booleanized formulae.  Thanks
    to Jori Dubrovin <jdubrovi AT circuit tcs hut fi>, Tommi
    Junttila <Tommi.Junttila AT tkk fi> and Vesa Ojala <vojala
    AT circuit tcs hut fi> for reporting the bug.

  - Fixed a bug that made ltl2smv fail when expanding DEFINEs
    containing input variables.

  - Fixed a bug in Incremental SBMC that resulted in false
    counterexamples in some cases if the property involved input
    variables. Thanks to Tommi Junttila <Tommi.Junttila AT tkk fi>
    for reporting the bug and also for providing a fix.

  - Fixed associativity of some PSL operators that were not
    consistent with the PSL LRM and with the core NuSMV
    grammar. Thanks to Andrea Fedeli <andrea.fedeli AT st com> for
    notifying us of the problem.

  - Applied bug fixes and cleanups suggested by Felix Rauch Valenti
    <frauch AT cse unsw edu au>. Valenti and others are working on a
    research project in the area of static analysis, i.e. finding
    problems (bugs) in C/C++ source code automatically. The research
    project is called
    [Goanna](http://www.ertos.nicta.com.au/research/goanna/) and uses
    NuSMV as backend.
    

  - Fixed trace plugins that were not correctly working
    when words variables were used. Thanks to Vincent Gourcuff
    <vincent.gourcuff AT lurpa ens-cachan fr> for reporting the bug.

  - Fixed a typo in configure.ac. Thanks to Mark Tuttle
    <mark.r.tuttle AT intel com> for reporting the bug.

  - Many other bug fixes.


## 2.4.1 - 2006-12-06

This is a minor release that provides to external contributions and
some extensions to the user interface. Moreover, it fixes many bugs.

### New features

  * Improvements

    - Optimized LTL tableau construction (contributed by Stefano
      Tonetta's <tonettas_AT_lu.unisi.ch>) for LTL model checking via
      BDDs. The optimization leads to an improvement of performance of
      about 30% on average. Moreover, minor low-level optimizations
      have been also integrated to the BDD-based LTL model checking,
      e.g., it is now possible to reuse (default) previously
      computated reachable states while combining the model and the
      tableau FSMs.

    - When building the scalar fsm, variable ordering is now taken
      into account. This is an extension of the contribution by Wendy
      Johnston <wendy_AT_itee.uq.edu.au> that initially extended NuSMV
      to allow the user to specify a "transition relation ordering"
      showing positive effects on the BDD conjunctive partioning. The
      ordering is taken from the BDD variable ordering by default, or
      it is taken from a specific "transition relation ordering"
      provided explicitly by the user through an option.


  * System commands

    - Added options `-f` (force) to the commands
      `build_boolean_model`, `bmc_setup`, `go`, `go_bmc` and
      `process_model`. When specified, the new option forces the
      corresponding model construction, even when COI is enabled.

    - Command `write_boolean_model` now dumps the "current variable"
      ordering when the dynamic reordering had triggered.

    - Added command `check_ctlspec` as an alias of `check_spec` (that
      becomes deprecated)

    - Command `check_pslspec` supports SBMC through new options `-s`,
      `-c` and `-N`. See the user manual for further information.


  * System variables

    - Added system variable `ltl_tableau_forward_search` to enable
      calculation of reachable states set for the LTL tableau FSM.

    - Added system variable `trans_order_file` to specify an
      alternative variable ordering to be used for the transition
      relation clusterization.

    - Renamed system variable `program_name` to `program_path` to hold
      the complete path of the executable file that is being
      executed when NuSMV is running.

    - Added new system variable `program_name` to hold the system
      (`NuSMV`) name. The new variable may be used to create new
      interactive programs derived from NuSMV.


  * Command line options

    - Added option `-flt` to enable calculation of reachable states
      set for the LTL tableau FSM (see system variable
      ltl_tableau_forward_search).

    - Added option `-t <tv_file>` to specify an alternative variable
      ordering to be used for the transition relation clusterization.


  * SMV grammar

    - Added new keyword CTLSPEC as an alias of SPEC (that become
      deprecated).


### Bug fixes:

  - Fixed a critical bug in CUDDs that made BDD variable groups
    inconsistent after a reordering.

  - Fixed a critical bug that affected groups creation in CUDDs

  - Fixed a critical bug that made booleanization being performed
    even when not needed, both in batch and interactive mode. Notice
    that the interactive mode of version 2.3.1 was also affected.

  - Assignment of bit selections is explicitly not handled, but
    trapped.

  - Fixed order of bits in words, that was reversed as the MSB was
    stored at the lowest position.

  - Bits of scalar variables were not grouped by default.

  - Operator `mod` may have boolean operands but a warning is emitted.

  - Dynamic reordering was erroneously disabled and got locked during
    encoding of input variables.

  - ITE (if-then-else) are now handles natively into PSL.

  - The command `process_model` no longer creates the boolean model.

  - Several fixes to the interactive shell when COI is enabled.

  - Fixed simulation starting from states into traces generated by
    BDD-based LTL model checking. The trace is stripped of the tableau
    variables. Warning: this fix may lead to more fake loops printed
    out when the trace is printed, as states that were different when
    tableau vars were in the trace now may become the same state after
    tableau vars are abstracted.

  - Disabling re-ordering during some specific actions (commonly
    executed by BMC) to prevent problems when reordering is
    called during the printing of a counterexample.

  - Fixed distributed examples there were no longer compilable due to
    stronger type checking rules.

  - Fixed some memory leaks.

  - Fixed implicit ordering of scalar input variables. Thanks to
    Andrew Miner <asminer_AT_cs.iastate.edu> for reporting this bug.

  - Fixed type checking of repeated Sere like {id [*N]}

  - Fixed Unary Minus operator to work on words and general
    expressions.

  - Many other minor bug fixes.


## 2.4.0 - 2006-07-31

This is a major release of NuSMV. Several improvements have been made
with respect to NuSMV 2.3. The most relevant are:

### Major changes

  - The NuSMV input language has been extended to support the type
    Word to model array of bits, and to allow bitwise logical and
    arithmetic operations over variables of type Word.

  - Introduced strong typing in order to improve the quality of the
    NuSMV file by eliminating subtle implicit cast that were causing
    problems. A backward compatibility switch disable such checks.

  - Integrated the Simple Bounded Model Checking [1] and its
    incremental and complete variant [2]. Thanks to Timo Latvala and
    Tommi Junttila.

  - Deeply restructured the NuSMV internals to provide support for the
    the dynamic creation and removal of new symbols (variables,
    constants, defines) to eliminate critical code within the LTL BDD
    based model checking algorithms, and to facilitate the integration
    of the family of the Simple Bounded Model Checking algorithms
    above.


### User interaction

  * General improvements
    - Improved the error messages reporting. Line numbers now
      corresponds to the real line number within the input file.
    - Improved the writing of the BDD order file: now it is possible to
      print the bit level order file.

  * New commands
    - `check_ltlspec_sbmc`: uses non-incremental algorithm to perform SBMC
    - `check_ltlspec_sbmc_inc`: uses incremental algorithm to perform SBMC
    - `gen_ltlspec_sbmc`: generates DIMACS file for SBMC problems
    - `print_fsm_stats`: substitutes the deprecated command `print_clusterinfo`

  * New system variables
    - `output_word_format`: controls in which base Words constants are
      outputted (during traces, counterexamples, etc, printing).
    - `backward_compatibility`: disables some new features to guarantee
      a better backward compatibility.
    - `type_checking_warning_on`: controls the notification of warning
      messages generated by the type checker.
    - `bmc_sbmc_gf_fg_opt`: controls whether the system exploits an
      optimization when performing SBMC on formulae in the form "F G p" or
      "G F p".

### New command line options
    - `old`: disables some new features to garantee a better backward
      compatibility.
    - `old_div_op`: enables the old semantics of `/` and `mod`
      operations (from version 2.3.0) instead of ANSI C semantics.
    - `df`: disables the computation of reachable states that is
      enabled by default in version 2.4.0.

  * Deprecated and no longer supported features
    - The command `print_clusterinfo` is deprecated (see `print_fsm_stats`)
    - The system variable `enable_reorder` is no longer available


### Other features
  - A windows installer is provided for Microsoft Windows (TM) users.


### Bug fixes
  - Type checking prevents many bugs there were afflicting previous
    versions to occur.
  - Support of dynamic symbols declaration fixes most of bugs that
    afflicted LTL BDD-based model checking.
  - Determinization variables are now dynamically declared as state
    variables instead of input variables.
  - "G Sequence" is no longer a parsable PSL property.
  - Several bug fixes have been applied to the printers of PSL formulae
  - Fixed re-entrancy of command `flatten_hierarchy`
  - Fainess condition are now printed correctly
  - Many memory leaks were fixed and numeric overflows.
  - Many other bug fixes (see details in ChangeLog).


## 2.3.1 - 2005-11-22

This is a minor release to fix a critical bug and a bunch of other
minor bugs. A new command line option `-sat_solver` has been added.

### New features
  - NuSMV is now linked to the latest version of MiniSat
    Sat solver:
     - MiniSat version 1.14
  - When available, the MiniSat solver is used by default.
  - Sat solvers like MiniSat can be now integrated more
    easily when compiling from sources.
  - The command line option `-sat_solver` allows the user to set
    the default Sat solver to be used by BMC. Many thanks to
    Guido Wimmel <wimmel_AT_in.tum.de> for providing the code to
    handle the new command.

### Bug fixes
  - Fixed a problem with COI when only LTL properties are present
    in the SMV file: the COI structure were wrongly initialized.
    Thanks to Kevin Xuke <xk02_AT_mails.tsinghua.edu.cn> for reporting
    this bug.
  - Fixes on the PSL support:
     - Removed a bug that caused wrong handling for some LTL and SERE
       PSL expressions.
     - Fixed expansion of `forall` expression.
  - The command `reset` does not modify input_order and output_order
    variables anymore.
  - The XML trace dumper plugin dumps loops information now.
  - Fixed bug in BddFsm_get_strong_backward_image: the method
    bdd_fsm_get_backward_si_projection computed legal states/input
    without caring for invariants.
  - Other minor bug fixes (see ChangeLog).


## 2.3.0 - 2005-07-15

This is a major release of NuSMV aiming to enable users to specify and
check properties expressed in PSL -- the Accellera (and upcoming IEEE)
standard Property Specification Language.

PSL is becoming one of the most popular property specification
languages across the EDA community, both in simulation and in formal
verification. It is intuitive, and easy to learn, read, and write. In
a nutshell, PSL extends linear temporal logics with the power of
regular expressions, and it is thus able to capture all omega-regular
properties. PSL is defined in four layers:

* the Boolean layer allows to write expressions over states.

* the Temporal layer uses Boolean expressions to specify behavior over
  time.

* the Verification layer consists of directives that specify how tools
  should use temporal layer specifications to verify functionality of
  a design.

* the Modeling layer defines the environment within which verification
  is to occur.

PSL is defined in various flavors, each corresponding to a particular
underlying hardware description language. The flavor determines the
syntax of the Boolean and modeling layers, and allows easy,
interoperable use of PSL across languages and design flows.

This version of NuSMV supports PSL with a new NuSMV flavor, and is
currently restricted to a subset of the Boolean and Temporal
layers. More comprehensive support will be implemented in the
forthcoming releases.

This release was developed as part of the activity within the PROSYD
project (http://www.prosyd.org -- Property-Based System Design), a
STREP project funded under the VI framework of the IST EU program.

### Minor changes

  - Fixed a bug in the interactive shell that inhibited the possibility
    to introduce and verify new properties to be interpreted and thus
    instantiated within a module declared in the input file.

  - Fixed a bug in the system interactive command `check_fsm`.


## 2.2.5 - 2005-05-05

This is a minor release to fix a critial bug and a bunch of other
minor bugs. Also, the usability of command line option `-load` has
been improved.

### Commands and options
  - The command line option `-load` now forces the interactive mode,
    even if option `-int` is not specified.


### Bug fixes
  - Fixed a critical bug that prevented the model from being built
    correctly when the interactive mode was used. This bug did not
    affect the batch mode at any rate.
  - Fixed an overflow error in the interactive simulator.
    Thanks to Alessandro Saiani for reporting the the bug.
  - Fixed the setting of system variables for reachable states
    computation in interactive mode, during system initialization.
  - Fixed several memory leaks and wrong string manipolations.
  - Fixed parsing of end-of-line under Windows within ltl2smv.
    Thanks to H. Peter Gumm for the bug report.
  - Other minor bug fixes (see ChangeLog for details).


## 2.2.4 - 2005-03-17

This is a minor release. One critical bug has been fixed, as well as a
few other minor bugs.  Furthermore, some commands have been enriched
with new options, and a few new system variables have been added.

### Commands and system variables
  - Enriched commands:
    * write_order supports dumping of bits (option -b).
    * echo supports file redirection (options -o and -a).

  - Added new system variables:
    * write_order_dumps_bits: to globally control the way the
      command write_order dumps encoded variables.

    * on_failure_script_quits: to control the behaviour of the command
      interpreter when a non-fatal error occurs.

### Bug fixes
  - Fixed a critical bug in the simulator that was affecting both
    the random and deterministic modalities, not the interactive
    mode.
  - In commands pick_state and simulate the option -a had a wrong
    semantics.
  - The command check_fsm wrongly printed out inputs instead of only
    state variables while printing deadlock states.
  - Other non-user side fixes and improvements. See ChangeLog for
    further details.


## 2.2.3 - 2004-12-29

This is a minor release. A few bug fixes have been made, and some new
features have been implemented.

### Improvements
  - ltl2smv is now invoked internally instead of as an external program.
    As result, it is not necessary to specify path to ltl2smv anymore.
  - Improved performances of boolean and BE conversions by caching results.
  - Improved performances of CNF conversion.
  - Fixed syntactical errors within a few distributed SMV files.

### Bug fixes
  - Fixed bugs in memoizing of formulas during conversion of SEXP to BEXP.
  - Fixed several bugs concerned with memory leaks and incorrent
    memory access.
  - Fixed bugs in DIMACS dumping.
  - Scalar variables with range 0..1 are now considered as boolean
    variables.
  - Relaxed a few too strong assertions that prevented NuSMV to
    compile some correct SMV files.


## 2.2.2 - 2004-08-06

This is a minor release.
Incremental algorithms for Bounded Model Checking have been
integrated, and a new generic interface to Sat solvers (incremental
and non-incremental) has been implemented.
Furthermore, a few bugs that afflicted previous versions have been
fixed.

### Features
  - Added support for MiniSat solver.
  - New generic interface for incremental and non-incremental sat
    solvers. Interfaces for Minisat have been
    implemented.
  - New algorithms for checking of invariants and LTL properties via
    Bounded Model Checking:
    * for checking of LTL properties via incremental sat.
    * "een-sorensson" for invariants via non-incremental sat.
    * "dual" and "zigzag" for invariants via incremental sat. See
      the paper "Temporal induction by incremental SAT solving" by
      Niklas Een and Niklas Sorensson for further information.

    Many thanks to Tommi Junttila <junttila_AT_fbk.eu> for having
    provided the first working prototype.

### Commands and system variables
  - Implemented new commands:
    * check_ltlspec_bmc_inc
    * check_invar_bmc_inc

  - Added option -a to the command check_invar_bmc (to specify
    algorithm to be used)

  - Added new system variables:
    * bmc_invar_alg: used to choose algorithm for non-incremental
      invariant checking.
    * bmc_inc_invar_alg: used to choose algorithm for incremental
      invariant checking.

  - Removed file extension ".dimacs" from default values assigned to
    system variables:
    * bmc_dimacs_filename
    * bmc_invar_dimacs_filename

### Improvements
  - Improved booleanization of scalar expressions in bmc. Thanks to
    Angela Pappagallo <angela_pappagallo_AT_yahoo.it>.
  - Improved usability of commands in bmc interactive mode.

### Bug fixes
  - Fixed several bugs of user commands for bmc.
  - Fixed a bug that occurred when instantiating more than once
    a variable whose range was containing one value only. Thanks to
    Charles Pecheur <pecheur_AT_email.arc.nasa.gov>.
  - Fixed a bug in the model checking code that caused assertion
    violation due to a missing intersection with fair states while
    starting computing counter-examples. Thanks to Charles Pecheur
    <pecheur_AT_email.arc.nasa.gov>.
  - Fixed wrong behavior of option -r in batch mode: only the number
    of reachable states is printed out now.
  - Fixed wrong allocation when sat solvers but SIM were called
    internally.
  - Other minor bug fixes (see ChangeLog for details).


## 2.2.1 - 2004-06-23

This is a minor release. A few bug fixes have been made.

### Bug fixes
  - Preprocessors invocation under Windows
  - Dynamic reordering works now properly.
  - Fixed a missing check of assignment of input variables, when
    arrays were used.
  - Variables are currently declared with the same order as 2.1.x
    series.
  - Fixed behavior of cmd line option '-o filename'.

### Contributions
  - Added examples/m4 directory containing a bunch of m4 examples,
    kindly provided by Roger Villemaire <villemaire.roger_AT_uqam.ca>.
  - Added contrib/nusmv-mode.el an (X)Emacs major-mode for editing and
    running NuSMV sources, kindly provided by Roger Villemaire
    <villemaire.roger_AT_uqam.ca>.

### Known bugs/problems
  - NuSMV launched with -cpp option and no input file falls into
    segfault.
  - Under Windows, the usage of multiple preprocessors will make
    NuSMV hang.


## 2.2.0 - 2004-06-10

This is a major release of NuSMV. Several improvements have been made
with respect to NuSMV 2.1. The most relevant are:

### Major changes

  - Input variables are now interpreted as labels over transitions in
    the Kripke structure. This solves several problems, including
    incorrect behaviors with processes.

  - Improved multiple FSM management, with APIs to enable
    composition. Improved management of encodings from scalar
    variables to the boolean space, with API to enable merging of
    encodings and other functionalities.

  - Packages and sub-packages have been introduced to represent the
    most significant data structures. Each package comes with a
    library, that can be linked separately from the rest of the
    system. New packages include different representations of FSM
    (i.e. Sexp, BDD, RBC), and Encodings.

  - Traces can be exported in different formats: textual (original),
    TAB separated list (to simplify import in spreadsheets), XML (for
    web browsing). Traces in XML format can also be imported and
    validated.

   - Improved BDD variable ordering. The reordering process can now
     work at the level of the boolean variables used to encode scalar
     variables, and the ordering file can directly refer to boolean
     variables. This may result in more compact BDDs (e.g. for
     formulae like x = y). Thanks to Dan Sheridan
     <dan.sheridan_AT_contact.org.uk> for the contribution.


### User Interaction

  - Input files can now be preprocessed with multiple preprocessors
    (e.g. cpp, m4); the invocation order can be defined by the
    user. Thanks to Roger Villemaire <villemaire.roger_AT_uqam.ca> for
    the interface with m4.

  - Trans type "Conjunctive" has been replaced by "Threshold" and is
    no longer supported.

  - Improved interactive simulator. In the choice of next states,
    duplicated entries are no longer present, and a distinction
    between state variables and input variables is enforced. Thanks to
    Anthony Wilder <anthony.wilder_AT_roke.co.uk> for the contribution.

  - Added some command line options: `-dcx`, to disable
    counter-example generation for false properties; `-pre`, to
    specify a list of pre-processors to be applied to the input file.


### Bounded Model Checking

  - Improved the RBC package by introducing some new simplifications.
    Improved the algorithm for the generation of the CNF. Thanks to
    T. Junttila <Tommi.Junttila_AT_hut.fi> for the contribution.


### Porting and Other

  - Porting under Mac OS X (see section 3 - Mac OS X in the file
    README_PLATFORMS for details).

  - Native compilation under windows using MinGW
    (http://www.mingw.org/). This eliminates the requirement for
    cygwin (http://www.cygwin.com) (see section 4 - Microsoft Windows
    in the file README_PLATFORMS for details).

  - Removed dependency on the tokens generated by yacc.


### Documentation

  - The user manual of version 2.1 has been split in two parts: a user
    manual and a tutorial.

  - The user manual has been updated to describe new commands and
    functionalities.


### New restrictions

  - Input variables can occur in LTL formulae and in fairness
    conditions. However, they are no longer allowed within CTL and
    invariant properties.

  - The body of DEFINEd symbols can no longer refer to next value of
    any variable.


### Bug fixes

  - Fixed problems related to wrong results in presence of processes.

  - Fixed problems in the check for recursive definitions.

  - Removed BDD memory leaks in the code for dealing with LTL under
    full fairness.


### Performance

  - The regression test w.r.t. version 2.1.2 pointed out an
    improvement in the performances of the model checking algorithms
    (BDD and SAT) in the majority of the considered cases.


### Known bugs/problems

  - Some temporal formulae containing a "liberal" interpretation of
    truth values (e.g. "(AX p) = (AX q)") are correctly parsed, but
    may cause an internal error. A restriction will be enforced in the
    next releases.

  - In some cases, a significant degradation in performance of the SIM
    sat solver was noticed as a consequence of the new RBC package and
    the CNFization algorithm. The precise cause of the degradation is
    being investigated.

  - Pressing Ctrl-C under Microsoft Windows Operating Systems within
    the interactive shell will cause unpredictable behavior.


## 2.1.2 - 2002-11-22

This is a bug fix release that solves a problem occurring on NuSMV
release 2.1.1 while checking for recursive definitions. Many thanks to
Yunja Choi for the bug report.


## 2.1.1 - 2002-11-08

This is a bug fix release that solves some problems showed by previous
releases.

### Generic
  - fixed a problem in the checking for recursive definitions that
    allowed to parse assignments with dependencies not broken by a
    time delay. (affected file nusmv/src/compile/compileCheck.c)

  - fixed a problem in the encoding that caused NuSMV to exit with an
    error when the BDD dynamic variable ordering was enabled. Many
    thanks to Yunja Choi for the bug report. (affected file
    nusmv/src/compile/compileEncode.c)

### BMC tableau generator
  - fixed a bug in the tableau generator that erroneously ignored
    NuSMV invariants in path of length 0. (affected file
    nusmv/src/bmc/bmcModel.c)

  - improved the performances of the Past LTL tableau generator.
    (affected file nusmv/src/bmc/bmcTableauPLTLformula.c)

### Documentation
  - improved the user manual. Thanks to Thomas Wahl for signaling us
    the ambiguities. (affected file nusmv/doc/user-man/nusmv.texi)

  - fixed a problem in the on-line documentation of the
    write_boolean_model command. Many thanks to Andrea Fedeli for the
    bug report. (affected file nusmv/src/compile/compileCmd.c)

### Parsing
  - fixed a problem in the lexical analyzer in the parsing of very
    long comments. (affected file src/parser/input.l)

  - not recognized preprocessor macros now cause a parsing error.
    (affected file src/parser/input.l)


## 2.1.0 - 2002-07-03

This is a major release of NuSMV. Several improvements have been done
with respect to NuSMV 2.0. The most relevant are:

### New functionalities

  - Past LTL

    Now LTL properties can also include *past* temporal operators.
    Differently from standard temporal operators, that allow to
    express properties over the future evolution of the FSM, past
    temporal operators allow to characterize properties of the path
    that lead to the current situation.

    Past LTL temporal operators are supported both in BDD-based Model
    Checking and in Bounded Model Checking of LTL specifications.  The
    extended LTL to SMV tableau translator for the past fragment of LTL
    has been contributed by Ariel Fuxman <afuxman_AT_cs.toronto.edu>.

  - Full Fairness

    Now NuSMV supports two types of fairness constraints, namely the
    weak fairness, or "justice", constraints and the strong fairness,
    or "compassion", constraints.

    A justice constraint consists of a formula f which is assumed to
    be true infinitely often in all the fair paths. A compassion
    constraint consists of a pair of formulas (p,q); if property p is
    true infinitely often in a fair path, then also formula q has to
    be true infinitely often in the fair path.

    Old versions of NuSMV supports only weak fairness.  In the current
    version of NuSMV, compassion constraints are supported only for
    BDD-based LTL model checking.  The strong fairness model checking
    algorithm for LTL specifications has been contributed by
    Rik Eshuis <eshuis_AT_cs.utwente.nl>.

    We plan to add support for compassion constraints also for CTL
    specifications and in Bounded Model Checking in the next releases
    of NuSMV.

### Architecture

  Several aspects of the NuSMV architecture have been improved.

  - We have improved the management of the Conjunctive Partitioning
    management of transition relations. With respect to old versions
    of NuSMV, the new code is more modular and easier to extend.

  - The Bounded Model Checking code has undergone a general revision.
    In particular:
    - the file organization of the bmc package has been improved
      in order to enhance readability and extensibility;
    - a generic interface to Boolean Expression Managers has been
      added; it is now possible to replace the current manager of
      boolean expressions (RBC) with more advance managers (e.g., BED);
    - a generic interface to SAT solvers gas been defined; this
      makes it easier to add support for new SAT solvers in NuSMV.

### Documentation

  - We have updated the user manual with the new features of NuSMV 2.
    Moreover, we have added to the manual a tutorial that covers
    the basic functionalities on NuSMV.

### Bug fixes

  Several bug fixes and minor enhancements have been done.


## 2.0.3 - 2002-03-27

- Fixed important bug in the management of the BDD-based
  representation of the Finite State Machines. This bug
  lead to an assertion failure in the case properties were
  checked using the cone-of-influence reduction.

  Many thanks to Yunja Choi for the bug report.

  (Affected files: `nusmv/src/compile/compile.h`, `compileCmd.c`,
  `compileStruct.c`, `compileUtil.c`, `nusmv/src/prop/propProc.c`,
  `nusmv/src/sm/smMisc.c`)

- Fixed bug in the "reset: routine of the interactive shell.
  The reachable states were not freed during the reset, thus
  leaving garbage information that lead to failures in
  the following commands.
  (Affected files: `nusmv/src/sm/smCmd.c`)

- Fixed a memory leak in some routines added to the CUDDs to fulfill
  the needs of NuSMV. (Affected files: `cudd-2.3.0.1/cudd/cuddAddOp.c`,
  `cuddBddOp.c`)

- Extended printing routines to new operators XOR and XNOR.
  (Affected files: `nusmv/src/node/nodePrint.c,nodeWffPrint.c`)

- Enhanced CUDD routine that extracts the symbolic representation of BDDs.
  (Affected files: `cudd-2.3.0.1/cudd/cuddBddOp.c`)


## 2.0.2 - 2002-01-30

- Fixed important bug in the optimized tableau construction of
  SAT-based Bounded Model Checking engine. This bug lead to an
  "assertion fail" in the verification of formulas "p U q".
  (Affected files: `nusmv/bmc/bmcModel.c`)

- Fixed minor bug in main routine of the batch verification: now the
  BDD master FSM is not built in the case of SAT-based Bounded Model
  Checking. This fix permits a substantial gain in performance in the
  case of Bounded Model Checking. (Affected files: nusmv/sm/smMisc.c)

- Disabled printing of memory/cpu statistics under cygwin. This
  fix allows for a clean compilation of NuSMV also on windows
  platforms. (Affected files: `cudd-2.3.0.1/util/cpu_stats.c`)

- Minor upgrades in the documentation and in the Makefile.


## 2.0.1 - 2001-11-14

This is a bug fix release that solves a problem occurring on SunOS systems.
Nothing relevant changes in the Linux version.

Many thanks to Rik Eshuis for the bug report.
