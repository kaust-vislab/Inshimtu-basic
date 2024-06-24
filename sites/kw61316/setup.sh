~/packages/cmake-3.28.1-linux-x86_64/bin/cmake \
    -Dcatalyst_DIR=/home/kressjm/packages/inshimtu-catalystV2/paraview/build/install/lib/cmake/catalyst-2.0 \
    -DVTK_DIR=/home/kressjm/packages/inshimtu-catalystV2/vtk/install/lib/cmake/vtk-9.3  \
    -DBOOST_ROOT=/home/kressjm/packages/inshimtu-catalystV2/boost_1_85_0 \
    -DBoost_NO_BOOST_CMAKE=TRUE \
    -DBoost_NO_SYSTEM_PATHS=TRUE \
    -DCMAKE_BUILD_TYPE=DEBUG \
    ../