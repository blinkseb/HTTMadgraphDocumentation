This page summarizes what it's needed to generate the Higgs signal with interference, but no background.

# Madgraph installation

- Setup the environment using the following script (traditionally called `setup_env.sh`):
```bash
#! /bin/bash

echo "Sourcing root"
source /cvmfs/cms.cern.ch/slc6_amd64_gcc481/lcg/root/5.34.17-cms/bin/thisroot.sh

source /cvmfs/cms.cern.ch/slc6_amd64_gcc481/external/libjpg/8b/etc/profile.d/init.sh
source /cvmfs/cms.cern.ch/slc6_amd64_gcc481/external/libpng/1.6.0-cms/etc/profile.d/init.sh

echo "Sourcing gcc / gdb"
source /cvmfs/cms.cern.ch/slc6_amd64_gcc481/external/gcc/4.8.1/etc/profile.d/init.sh;
source /cvmfs/cms.cern.ch/slc6_amd64_gcc481/external/gdb/7.7/etc/profile.d/init.sh;

echo "Sourcing python"
source /cvmfs/cms.cern.ch/slc6_amd64_gcc481/external/python/2.7.3-cms/etc/profile.d/init.sh

echo "Sourcing boost"
source /cvmfs/cms.cern.ch/slc6_amd64_gcc481/external/boost/1.51.0-cms/etc/profile.d/init.sh

export BOOST_ROOT=$BOOST_ROOT

# Find DECAY and export env. variable
local_path=`find . -type d -name 'DECAY'`
abs_path=`readlink -f ${local_path}`
export DECAY=$abs_path
```
You may need to adapt the path if you don't have a CVMFS installation.

- Source the script you just created to setup the environment
- Download and install LHAPDF:
```bash
curl -O -L http://www.hepforge.org/archive/lhapdf/lhapdf-5.9.1.tar.gz
tar xf lhapdf-5.9.1.tar.gz
cd lhapdf-5.9.1
./configure --prefix=$PWD/..
make -j20
make install
```

- Get CT10 PDF:
```bash
mkdir <main_folder>/share/lhapdf/PDFsets
curl -o <main_folder>/share/lhapdf/PDFsets/CT10.LHgrid -L http://www.hepforge.org/archive/lhapdf/pdfsets/5.9.1/CT10.LHgrid
```
- Download Madgraph. You need v2.2.1 at least. For the purpose of this document, I'll use version 2.2.1. Adapt all the paths if your are using a more recent version.
```bash
curl -O -L https://launchpad.net/mg5amcnlo/2.0/2.2.0/+download/MG5_aMC_v2.2.1.tar.gz
tar xf MG5_aMC_v2.2.1.tar.gz
```

- Configure Madgraph to use our LHAPDF:
```
cd MG5_aMC_v2_2_1/input
```
   * Edit `mg5_configuration.txt` and edit around line 136
```
lhapdf = <path_to_lhapdf-config> (something like /gridgroup/cms/brochet/HTT/SL6/Madgraph5/bin/lhapdf-config)
```
- Download and install DECAY
```
cd MG5_aMC_v2_2_1/
curl -O -L https://github.com/blinkseb/HTTMadgraphDocumentation/releases/download/v0.1/DECAY.tar.bz2
tar xf DECAY.tar.bz2
cd DECAY
make clean
make -j5

cd ../..
curl -O -L https://github.com/blinkseb/HTTMadgraphDocumentation/releases/download/v0.1/runDecay.sh
chmod +x runDecay.sh
```

- There is a patch to apply not yet included in the official distribution. Edit the file `madgraph/various/combine_runs.py` and apply the following patch (you need to add the lines starting with a `+` to the file, you have some context around to see where to put the modifications ; below line 143). It's possible that the patch is already applied to the file if you use a version more recent than 2.2.1.
```
=== modified file 'madgraph/various/combine_runs.py'
--- madgraph/various/combine_runs.py	2013-11-29 07:28:53 +0000
+++ madgraph/various/combine_runs.py	2014-06-18 13:34:20 +0000
@@ -143,6 +143,8 @@
                     sign = ''
                 else:
                     sign = '-'
+                if new_wgt.startswith('-'):
+                    sign = ''
                 line= ' %s %s%s %s\n' % (' '.join(data[:2]), sign,
                                            new_wgt, ' '.join(data[3:]))
             fsock.write(line)
```

- Madgraph is installed and functionnal.

# Setup TopBSM model

The next step is to install the topBSM model into Madgraph. Go to the Madgraph folder:
```
cd models/
curl -O -L https://github.com/blinkseb/HTTMadgraphDocumentation/releases/download/v0.1/topBSM_v4.tar.bz2
tar xf topBSM_v4.tar.bz2
```

# Generate template

The first thing to do is to generate a template process. We'll use this template as a base to generate signal with different masses and couplings.

- Go to the Madgraph folder and launch it
```
./bin/mg5_aMC
```

- Load the topBSM model, and generate `g g > t t~`
```
import model_v4 topBSM
generate g g > t t~ / o0 QED=2 QCD=2 QS0=2 Qs2=0 QS1=0
output gg_tt_template
exit
```

- Edit the run card to set default values. **This step is very important, do not forget it**.
    - Go to the folder `gg_tt_template`, and edit the file `Cards/run_card.dat`
    - First, change `ebeam1` and `ebeam2` to the desired center of mass energy (for 8 TeV, set both to 4000)
    - Change `pdlabel` (usually line 51) from `cteq6l1` to `lhapdf` to use external LHAPDF
    - Change `lhaid` (next line) from `10042` to `10800` to use the CT10 PDF
    - **Very important** After the line containing `scalefact`, add the following line

            F        = fixed_couplings

    - Just below, change the version of the LHE file produced by inserting the following line
    
            1.0      = lhe_version       ! Change the way clustering information pass to shower.

  - Change `maxjetflavor` from `4` to `5`
  - Finally, change 'xqcut' from `0` to `20`
  - **Optionnal** You can also change the number of generated events with the options `nevents`. Don't go over 500000.

- The next part is the tricky one. We'll modify Madgraph code to compute only signal + interference, without the background.
    - Go the the `SubProcesses/P0_gg_ttx` folder. This folder contains the source code for the matrix element computation in the file `matrix1.f`. Open this file.
    - Interesting code starts around line 263, with the line `CALL VXXXXX(P(0,1),ZERO,NHEL(1),-1*IC(1),W(1,1))`. This is a call to HELAS in order to compute a vertex.
    - After the series of `CALL` (line 279), the amplitude of each diagrams are computed are stored inside the `AMP` array. `AMP(1)` contains the amplitude of the first diagram, `AMP(2)` the one of the second diagram, etc. Open the file matrix.jpg to see all the diagrams and which index corresponds to which diagram.
    - Computation of the matrix element is done on lines 282 to 289. The first thing to do is to comment these lines. To do that, simply insert `c` at the beginning of each line
    - After the final `ENDDO` (line 296), insert the following code.


                  MATRIX1 = 0.D0
            c     Compute matrix element directly from formula.
            
            c     Squared matrix element for diagram 1
            c     MATRIX1 = MATRIX1 + 12*AMP(1)*DCONJG(AMP(1))
            
            c     Squared matrix element for diagram 2
                  MATRIX1 = MATRIX1 + 24*AMP(2)*DCONJG(AMP(2))
            
            c     Squared matrix element for diagram 3
            c      MATRIX1 = MATRIX1 + 16D0/3D0*AMP(3)*DCONJG(AMP(3))
            
            c     Squared matrix element for diagram 4
            c      MATRIX1 = MATRIX1 + 16D0/3D0*AMP(4)*DCONJG(AMP(4))
            
            c     Interference between backgrounds
            c      MATRIX1 = MATRIX1 + 6*AMP(3)*DCONJG(AMP(1))
            c      MATRIX1 = MATRIX1 + 6*AMP(1)*DCONJG(AMP(3))
            c      MATRIX1 = MATRIX1 - 6*AMP(4)*DCONJG(AMP(1))
            c      MATRIX1 = MATRIX1 - 6*AMP(1)*DCONJG(AMP(4))
            c      MATRIX1 = MATRIX1 - 2D0/3D0*AMP(4)*DCONJG(AMP(3))
            c      MATRIX1 = MATRIX1 - 2D0/3D0*AMP(3)*DCONJG(AMP(4))
            
            c     Interference between signal and background
                  MATRIX1 = MATRIX1 + 4*AMP(3)*DCONJG(AMP(2))
                  MATRIX1 = MATRIX1 + 4*AMP(2)*DCONJG(AMP(3))
            
                  MATRIX1 = MATRIX1 + 4*AMP(4)*DCONJG(AMP(2))
                  MATRIX1 = MATRIX1 + 4*AMP(2)*DCONJG(AMP(4))

    - Interference between backgrounds are already commented in order to generate only signal + interference
    - Details of the calculation can be found in the attached Mathematica notebook.

- The template if now ready for the generation.

# Generate the signal

The first thing you need to do is to copy the template folder to a folder where the generation will take place
```
cp -r gg_tt_template gg_tt_M400_scalar_S_i
```

This example will show how to generate events for a 400 GeV scalar particle. Go to the `gg_tt_M400_scalar_S_i`, and edit the file `Cards/param_card.dat`. In this file, we'll set the mass of the particle, as well as it's coupling.

 * The mass is set by the `6000045` parameter in the `MASS` block (`Block MASS`) (around line 77). Set it to `400` (notice it's already the default value)
 * The coupling is set by 2 parameters inside the block `MGUSER` (`BLOCK MGUSER`), `1` and `2` (lines 121 and 122). The first parameter control the scalar coupling, and the second the pseudoscalar coupling. So, to generate a scalar particle with coupling 1, set `1` to 1 and `2` to 0.

All the editing is done. You're ready to generate the events. Launch Madgraph :
```
./bin/madevent
```

You now have two choices. Either generate one run (`launch`) or multiple runs (`multi_run`). Multiples runs are needed if you want more than 500 000 events. We'll generate only one run, so simply execute the command
```
launch
```

Type enter to select the defaults options, and enter a second time to validate the cards (we already edited them). Now, just wait for the generation to finish. LHE files are located in the `Events` folder. Have fun!

# Post-processing

Note: it seems that's this is now done automatically done by recent Madgraph version. It's still wise to check on one LHE file that the header is correct.

**This step is very important. If you forget it, Pythia will consider all events with a negative weight as a positive weight event, resulting in garbage after hadronization.**

The LHE file produced by Madgraph expect by default that only positive weight events are produced. Without our modification, positive and negative weights events exist. Inside the LHE file, in the `<init></init>` section, you'll find a bunch of numbers, corresponding to the `HEPRUP` structure (see http://arxiv.org/pdf/hep-ph/0109068v1.pdf for more details). The field we're interested in is `IDWTUP`, ie, how the event weight should be interpreted by the program reading the LHE field. By default, Madgraph set it to `3`, which mean *Events are unweighted on input such that all events come with unit weight XWGTUP=+1.*. It's a bit problematic since we also have events with negative weight. The solution is to replace the field `IDWTUP` with the value `-3`, which means *Same as IDWTUP=+3, but the event weights may be either +1 or –1 on input.*. Perfect!

Open the LHE file with a text editor. Find the `<init>` block (around line 566). On the first line, you'll find a `3` alone, followed by a `1` and the end of the line. Replace `3` by `-3`, and you're done.

# Decay LHE file

The LHE produced by Madgraph contains top quark non decayed. Instead of letting Pythia doing the decay (with very bad spin correlation effects), we use a small fortran package called DECAY. All you have to do is to use the wrapper script included named `runDecay.sh` to launch decay on your LHE file.
```
./runDecay.sh <full_path_to_my_lhe_file.lhe>
```

It'll produce an output file name like your input file, but postfixed with `_decayed`.

**DECAY can take a very long time to complete. Be patient!**
