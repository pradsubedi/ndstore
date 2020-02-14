# ndstore

## Dependencies

* mercury (git clone --recurse-submodules https://github.com/mercury-hpc/mercury.git)
* argobots (git clone https://github.com/pmodels/argobots.git)
* margo (git clone https://xgitlab.cels.anl.gov/sds/margo.git)

## Building
Create a build directory
```
$ mkdir build
$ cd build
```
Generate make files from source files (doesnot build included tests)
```
$ cmake ..
```
OR

If you want to build included tests, you need MPI and enable tests option
```
$ cmake -DENABLE_TESTS=ON -DCMAKE_C_COMPILER=mpicc ..
```
Now install the ndstore provider
```
$ make
$ make install
```

