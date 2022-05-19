# eic_endcap_insert

Running the standalone simulation
---------------------------------
Enter the EIC container and source the default enviroment. Then do the following:

```
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$ATHENA_PREFIX -DCMAKE_CXX_STANDARD=17
make install

cd ..
./run_sim
```

