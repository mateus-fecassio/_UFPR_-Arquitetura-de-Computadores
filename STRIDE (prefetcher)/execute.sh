#!/bin/bash

time ./orcs -t ../traces/astar/astar.CINT.PP200M > astar.txt

time ./orcs -t ../traces/calculix/calculix.CFP.PP200M > calculix.txt

time ./orcs -t ../traces/dealII/dealII.CFP.PP200M > dealII.txt

time ./orcs -t ../traces/gromacs/gromacs.CFP.PP200M > gromacs.txt

time ./orcs -t ../traces/libquantum/libquantum.CINT.PP200M > libquantum.txt

time ./orcs -t ../traces/namd/namd.CFP.PP200M > namd.txt
