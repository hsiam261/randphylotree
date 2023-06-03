# rand-phylo-tree
`rand-phylo-tree` takes as input a `yaml` config file and generates a rooted phylogenetic tree with topology sampled 
uniformly from the set of all topologies given the number of leaves (species) in the phylogenetic tree. The branch lengths 
can be sampled uniformly over an interval or from a normal distribution.

Further more, `rand-phylo-tree` generates a k-word count matrix for that tree, given the parameters of a birth-death-migration 
by sampling the k-word frequencies at the root from the stationary distribution and then simulating mutation in-accordance to a
linear birth death migration model.

## Format of the `yaml` Config Files
The `yaml` can have the following keys:
- `number-of-species`: the number of leaves in the phylogenetic tree to be generated (required)
- `number-of-sites`: the number of sites in k-word frequency matrix (required)
- `number-of-samples`: the number of trees that will be generated (optional). when the number of samples is omitted in the `yaml`,
a single tree is generated. The name of the output tree is read from the second command line argument. If it is present, the second
command line argument is used as prefix of the output file and the integers 0 to `number-of-samples - 1` is appended to the end of it to make the names unique.
- `branch-length-distribution`: the probability distribution for the branch lengths (required). the distribution can be one of the
following two:
  - `uniform`: this is map that requires the two following keys:
    - `lower`
    - `upper`
    when `uniform` is used as the `branch-length-distribution`, the branch lengths are being sampled uniformly from the closed interval `[lower, upper]`
  - `normal`: this is a map that requires the following two keys:
    - `mean`
    - `standard-deviation`
  when `normal` is used as the `branch-length-distribution`, the branch lengths are sampled from the normal distribution with mean `mean` and variace `standard-deviation^2`.
- `lambda`, `m`, `mu`: the rates of the birth, migration and death (required). They can be either a float or a one of the previously
 mentioned distributions. when any of them are initialized with -1, it is assumed that the rates are unknown and the k-word 
matrix generation part is skipped.

## Format of the Generated Tree
```
NUMBER-OF-SPECIES/LEAVES
NUMBER-OF-SITES
LAMBDA M MU
# NEXT 2xNUMBER-OF-SPECIES - 2 LINES
u v w # u and v are two endpoints of the tree and w is the branch length
...
...
# NEXT NUMBER-OF-NODES LINES
a_1 a_2 ... a_{NUMBER-OF-SITES} #each line contains a list of NUMBER-OF-SITES space separated non-negative integers
...
...
```
- The tree is generated in a manner that the nodes of the tree are the integers from 0 to 2xNUMBER-OF-SPECIES-3.
- The root is always labeled with 0.
- The species/leaves are always labeled with 1, 2, ... NUMBER-OF-SPECIES.

## Dependencies
- `cmake`
- `make`
- `git`
- `g++17`
- `bash`
- `yaml-cpp`

## Steps to Build
- install git if not already present
- clone the repo
- install cmake, make and g++17
- run the `build.sh` script.

It will create a single executable named `generate-random-tree`.

## Run
```
./generate-random-tree` yaml-config output-file|output-file-prefix
```
