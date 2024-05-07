module reset
#module swap oneapi oneapi/eng-compiler/2023.12.15.002
#module load spack-pe-gcc
module load cmake
module load python
#module swap oneapi/eng-compiler/2022.12.30.003 oneapi/eng-compiler/2022.12.30.005
module load hwloc

export HTTP_PROXY=http://proxy.alcf.anl.gov:3128
export HTTPS_PROXY=http://proxy.alcf.anl.gov:3128
export http_proxy=http://proxy.alcf.anl.gov:3128
export https_proxy=http://proxy.alcf.anl.gov:3128
git config --global http.proxy http://proxy.alcf.anl.gov:3128

