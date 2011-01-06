
   #!/bin/sh
   # if you can not use the mex files, uncomment this line.
   export LD_LIBRARY_PATH=~/uni/diplom/SPAMS/libs_ext/mkl64/
   export DYLD_LIBRARY_PATH=~/uni/diplom/SPAMS/libs_ext/mkl64/
   export KMP_DUPLICATE_LIB_OK=true
   matlab $* -r "addpath('~/uni/diplom/SPAMS/release/mkl64/'); addpath('~/uni/diplom/SPAMS/test_release'); "
