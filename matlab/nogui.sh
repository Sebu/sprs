
   #!/bin/bash
   # if you can not use the mex files, uncomment this line.
   export LD_LIBRARY_PATH=~/uni/diplom/SPAMS/libs_ext/mkl32/
   export DYLD_LIBRARY_PATH=~/uni/diplom/SPAMS/libs_ext/mkl32/
   export KMP_DUPLICATE_LIB_OK=true
   cd git/diplom/matlab
   matlab $* -nosplash -nodesktop -nojvm -r "addpath('~/uni/diplom/SPAMS/release/mkl32/'); testDict('$1', 12); exit;"

