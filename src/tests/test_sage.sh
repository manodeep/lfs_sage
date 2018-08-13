#!/bin/bash
cwd=`pwd`
datadir=../auxdata/trees/mini-millennium
# the bash way of figuring out the absolute path to this file
# (irrespective of cwd). parent_path should be $SAGEROOT/src/tests
parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd "$parent_path"/$datadir
if [ ! -f trees_063.7 ]; then
    curl -O https://data-portal.hpc.swin.edu.au/dataset/7bab038b-1d1f-4e79-8cfc-ea171dd1492f/resource/7ff28a50-c401-4a07-9041-13524cbac5c9/download/mini-millennium.tar 
    if [[ $? != 0 ]]; then
        echo "Could not download tree files from the Swinburne data portal...aborting tests"
        echo "Failed"
        exit 1
    fi
    
    tar xvf mini-millennium.tar
    if [[ $? != 0 ]]; then
        echo "Could not untar the mini-millennium tree files...aborting tests"
        echo "Failed"
        exit 1
    fi
fi


rm -f model_z*

# cd back into the sage root directory and then run sage
cd ../../../../
./sage "$parent_path"/$datadir/mini-millennium.par
if [[ $? != 0 ]]; then
    echo "sage exited abnormally...aborting tests"
    echo "Failed"
    exit 1
fi

# now cd into the output directory for this sage-run
cd "$parent_path"/$datadir
files=`ls model_z*`
testdata_dir=output
if [[ $? == 0 ]]; then
    npassed=0
    nbitwise=0
    nfiles=0
    nfailed=0
    for f in $files; do
        ((nfiles++))
        diff -q   $f  $testdata_dir/$f
        if [[ $? == 0 ]]; then
            ((npassed++))
            ((nbitwise++))
        else
            python "$parent_path"/sagediff.py $f $testdata_dir/$f         
            if [[ $? == 0 ]]; then 
                ((npassed++))
            else
                ((nfailed++))
            fi
        fi
    done
else
    # even the simple ls model_z* failed
    # which means the code didnt produce the output files
    # everything failed
    npassed=0
    # use the knowledge that there should have been 64
    # files for mini-millennium test case
    # This will need to be changed once the files get combined -- MS: 10/08/2018
    # Changed to 8 -- MS: 13/08/2018
    nfiles=8
    nfailed=$nfiles
fi
echo "Passed: $npassed. Bitwise identical: $nbitwise"
echo "Failed: $nfailed"
# restore the original working dir
cd "$cwd"
exit $nfailed
