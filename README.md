This code provides a dynamic programming approach for efficiently computing with bridgelets, which were recently
introduced by John Krumm [1]. It in particular performs an experimental evaluation of computing the sizes of bridgelets.

To run it, you need a compiler set that supports C++17; make; cmake; and
python3 with libraries listed in [requirements file](requirements.txt) to make
the plots.

# Build instructions

```
git clone https://github.com/aleqss/efficient-bridgelets.git
cd efficient-bridgelets
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j6
cd ..
./build/randomwalks
```

To make the plots afterwards, install the packages and run the script
```
python3 -m venv env && source env/bin/activate && pip install -r requirements.txt
./plot_data.py
```
This generates plots in [figs/](figs/) based on the files in [data/](data/).

[1] John Krumm. Maximum Entropy Bridgelets for Trajectory Completion. 2022.
Accepted to SIGSPATIAL 2022.
