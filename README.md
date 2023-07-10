# Vehicular Networks and Cooperative Driving


Anno Accademico 2022/23, Stefano Fontana, Elena Tonini.

[FINAL REPORT](VNCD.pdf)
----

## Build Instructions
### Clone
First of all you should clone the repository with all its submodules
```bash
$ git clone git@github.com:tetofonta/vehicular-networks-and-cooperative-driving.git --recursive
```

### Install Depenencies and configure them
Then you should install all the required dependencies:

 - **cmake** minimum version 3.22: `apt install cmake`
 - **omnetpp** minimum version 6.0, and it should be inserted into the path variable: `PATH="$PATH:/path/to/omnetpp/bin"`
 - **veins** minimum version 5.2, and it should be inserted into the path variable: `PATH="$PATH:/path/to/veins/bin"`
 - **plexe** (michele-segata/plexe) minimum version 3.1, and it should be inserted into the path variable: `PATH="$PATH:/path/to/plexe/bin"`

### Build

```bash
$ cd vehicular-networks-and-cooperative-driving
$ cmake -B "$PWD/build" -S "$PWD" -DCMAKE_BUILD_TYPE=Debug
$ cmake --build $PWD/build --target libvncd -j $(nproc)
```

### Run

```bash
$ cmake --build $PWD/build --target run_vncd
```

### Batch Build and Run

```bash
cd vehicular-networks-and-cooperative-driving
cmake -B "${PWD}/build" -S "$PWD" -DCMAKE_BUILD_TYPE=Release
cmake --build "${PWD}/build" --target libvncd -j $(nproc)
cmake --build "${PWD}/build" --target runmaker
runmaker4.py "${PWD}/build/runs.txt" -j $(nproc)
```
