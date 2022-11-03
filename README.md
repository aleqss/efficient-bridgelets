This code provides a dynamic programming approach for efficiently computing
with bridgelets, which were recently introduced by John Krumm [1].
In particular, it performs an experimental evaluation of computing the sizes of
bridgelets.

_If you wish to understand how the code works and learn more about what we can
compute, check the
[companion website](https://aleqss.github.io/efficient-bridgelets/)._

![Generated trajectories for $T = 400$, going from $(0, 0)$ to $(40, 20)$.](/../gh-pages/trajectories.svg)

# Capabilities
With this code, given a time limit $T$ and a starting point $(a, b)$, we can:
* count the paths from $(a, b)$ to **all** cells in $t$ steps, for all
$0 \leq t \leq T$;
* count the paths from $(a, b)$ to a given $(c, d)$ in $T$ steps that pass
through a cell $(x, y)$ at time $t$, for **all** $(x, y)$ and $0 \leq t \leq T$;
* count the paths from $(a, b)$ to **all** cells in $t$ steps, for all
$0 \leq t \leq T$, in the presence of obstacles that block cells starting at a
given time;
* based on the path counts, efficiently generate random trajectories of length
$T$ that follow the given distribution and go between the given start and end
points.

# Precomputed data
In order to make it easier to use the data, we have precomputed the visit
counts for certain values of $T$.
In particular, a file `v_t_x_y` contains, for each cell, the count of paths
from $(0, 0)$ to $(x, y)$ in $t$ steps that visit that cell.
Dividing those counts by the sum of all values gives visit probabilities at the
desired level of precision.
Due to the symmetry, we only compute the tables for $0 < y \leq x \leq t$.
The tables are [available](/releases/latest/download/visits.zip) for the values
of $t$ from $1$ to $100$. 

# Build requirements
To run the code, you need a compiler set that supports C++17; make; cmake; GMP;
and python3 with the libraries listed in [requirements file](requirements.txt)
to make the plots.

# Build instructions
To run the code:
```
git clone https://github.com/aleqss/efficient-bridgelets.git
cd efficient-bridgelets
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j6
cd ..
./build/randomwalks
```
The program will offer you to compute, in order, the dynamic programs for all
paths, for visiting the cells, for paths in presence of obstacles, and to
generate the trajectories.
If you wish to use some of these capabilities in your own code, their usage in
[the main function](main.cpp) is a good starting point.

To make the plots afterwards, install the packages and run the script:
```
python3 -m venv env && source env/bin/activate && pip install -r requirements.txt
./plot_data.py
```
This generates plots in [figs/](figs/) based on the files in [data/](data/).

[1] John Krumm. Maximum Entropy Bridgelets for Trajectory Completion. 2022.
Accepted to SIGSPATIAL 2022.
