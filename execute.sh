#!/bin/bash

./orcs -t traces/astar/astar.CINT.PP200M > astar.txt

./orcs -t traces/calculix/calculix.CFP.PP200M > calculix.txt

./orcs -t traces/dealII/dealII.CFP.PP200M > dealII.txt

./orcs -t traces/gromacs/gromacs.CFP.PP200M > gromacs.txt

./orcs -t traces/libquantum/libquantum.CINT.PP200M > libquantum.txt

./orcs -t traces/namd/namd.CFP.PP200M > namd.txt
