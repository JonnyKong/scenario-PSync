Prerequisites
=============
    mkdir ndnSIM
    cd ndnSIM

    git clone https://github.com/named-data-ndnSIM/ns-3-dev.git ns-3
    git clone -b ndnSIM-2.7 --recursive https://github.com/named-data-ndnSIM/ndnSIM ns-3/src/ndnSIM

    # Build and install NS-3 and ndnSIM
    cd ns-3
    ./waf configure -d optimized
    ./waf
    sudo ./waf install

    cd ..
    git clone https://github.com/JonnyKong/scenario-PSync.git scenarios
    cd scenarios

    ./waf configure
    ./waf

After which you can proceed to compile and run the code

For more information how to install NS-3 and ndnSIM, please refer to http://ndnsim.net website.

Patches
=======
After checking out the correct versions, you have to apply these patches to ndnSIM and NFD:

* `ndnSim_patches/ndnSIM.patch` (to `ns3/src/ndnSIM`)
* `ndnSim_patches/NFD.patch` (to `ns3/src/ndnSIM/NFD`)
* `ndnSim_patches/ndn-cxx.patch` (to `ns3/src/ndnSIM/ndn-cxx`)

These patches are used to implement some additional features that ns-3 doesn't currently support well (e.g. loss rate in Wi-Fi networks).