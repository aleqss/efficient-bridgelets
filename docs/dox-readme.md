This code provides a dynamic programming approach for efficiently computing
with bridgelets, which were recently introduced by John Krumm [1].
In particular, it performs an experimental evaluation of computing the sizes of
bridgelets.
It is also possible to adapt the code to your dataset in order to interpolate
new trajectories from the same area based on historical data.

_If you wish to understand how the code works and learn more about what we can
compute, check the
[companion website](https://aleqss.github.io/efficient-bridgelets/)._

![Five random trajectories.](/../gh-pages/trajectories.svg "Illustration of a
bridgelet: generated trajectories from \f$(0, 0)\f$ to \f$(40, 20)\f$ in \f$400\f$ steps.")

# Capabilities
With this code, given a time limit \f$T\f$ and a starting point \f$(a, b)\f$, we can:
* count the paths from \f$(a, b)\f$ to **all** cells in \f$t\f$ steps, for all
\f$0 \leq t \leq T\f$;
* count the paths from \f$(a, b)\f$ to a given \f$(c, d)\f$ in \f$T\f$ steps that pass
through a cell \f$(x, y)\f$ at time \f$t\f$, for **all** \f$(x, y)\f$ and \f$0 \leq t \leq T\f$;
* count the paths from \f$(a, b)\f$ to **all** cells in \f$t\f$ steps, for all
\f$0 \leq t \leq T\f$, in the presence of obstacles that block cells starting at a
given time;
* based on the path counts, efficiently generate random trajectories of length
\f$T\f$ that follow the given distribution and go between the given start and end
points.

# Precomputed data
In order to make it easier to use the data, we have precomputed the visit
counts for certain values of \f$T\f$.
In particular, a file `v_t_x_y` contains, for each cell, the count of paths
from \f$(0, 0)\f$ to \f$(x, y)\f$ in \f$t\f$ steps that visit that cell.
Dividing those counts by the sum of all values gives visit probabilities at the
desired level of precision.
Due to the symmetry, we only compute the tables for \f$0 < y \leq x \leq t\f$.
The tables are available as text files split into several archives on the
[releases page](/aleqss/efficient-bridgelets/releases/latest/) for a range of
values of \f$t\f$.

# Build requirements
To run the code, you need a compiler set that supports C++17; make; cmake; GMP;
and python3 with the libraries listed in [requirements file](../../requirements.txt)
to make the plots.

# Build instructions
<!--To run the code:
```
git clone https://github.com/aleqss/efficient-bridgelets.git
cd efficient-bridgelets
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j6
cd ..
./build/randomwalks
```
-->
```
git clone https://github.com/aleqss/efficient-bridgelets.git
cd efficient-bridgelets
mkdir build
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
cmake --build build -j6
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

[1] John Krumm. 2022. Maximum Entropy Bridgelets for Trajectory Completion.
To appear in _Proceedings of the 30th International Conference on Advances in
Geographic Information Systems (ACM SIGSPATIAL 2022)._
