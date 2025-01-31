
## CITING MISTRAL

<pre>
@article{hebrardmistral,
  title={Mistral, a Constraint Satisfaction Library},
  author={Hebrard, Emmanuel},
  journal={Third International CSP Solver Competition},
  url={http://www.cril.univ-artois.fr/CPAI08/Competition-08.pdf},
  pages={31--39}
}
</pre>


## BUILDING instructions for MistralXCSP

$ mkdir build
$ cd build
$ cmake ..
$ make MistralXCSP

## RUNNING MistralXCSP

$ ./MistralXCSP <xml file> -t <time limit in seconds>

## COMPETITION CMDLINE

DIR/MistralXCSP BENCHNAME -t TIMELIMIT -s RANDOMSEED

## COMPILATION AND CMDLINE FOR CP2023 PAPER

$ make preemptive-jsp
bin/preemptive-jsp BENCHNAME -t TIMELIMIT 


## OLD README

Mistral is an open source constraint programming library written in C++. It implements a modelling API, however, it can also read instance files in XCSP3 or FlatZinc format. It is also fully interfaced with Numberjack (https://github.com/eomahony/Numberjack) which provides a Python API for modelling and solving combinatorial optimization problems using several back-end solvers.

A number of examples are available in the examples/src folder. To get started, you can have a look at the /examples/src/simple_linear_constraints.cpp file. It provides an simple problem defined by a set of linear constraints.  To compile it, use the following command: 

$make simple_linear_constraints 

This generated the binary bin/simple_linear_constraints


The minizinc interface is available in the fz/folder. The fz/README file contains information regarding compilation and execution. 

Compilation note:

*/ The solver requires the boost library. Get the latest version here https://www.boost.org and update the Makefile accordingly.
