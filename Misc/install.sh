#!/bin/bash

#    An install script for the VelociRaptor program and Apophenia-0.999.
#This is still being tested!!! It is just a collection of the steps in README.
#Make sure to take a look at README before trying this script. Also take a 
#look at the script so you can see what it is doing.
#    This script uses sed to make version changes so that everything compiles
#correctly. Still testing things out.
#    To use, make a new directory and copy the install2.sh file to the new directory.
#A "test" directory would work fine. Then run the script.
#
# bash install2.sh
# or 
# ./install2.sh

read -p "Do you wish to install the VelociRaptor program? (y/n) " answer
if [ "$answer" = "y" ]; then
    #Check sudo permissions.
    sudo -v
    sudo_return=$?
    if [ $sudo_return -ne 0 ]; then
        echo "Need sudo permission to install."
    else
        #See if github is available.
        ping -c 1 www.github.com
        ping_return=$?
        if [ $ping_return -ne 0 ]; then
            echo "Can't find github. Make sure you have an active internet connection and retry."
        else
            echo "Get dependencies"
            sudo apt-get update 
            sudo apt-get install make gcc libgsl0-dev libsqlite3-dev
            sudo apt-get install libgtk-3-dev gnuplot gfortran
            echo "Make RaptorDirectoryTest"
            rm -rf RaptorDirectoryTest
            mkdir RaptorDirectoryTest
            cd ./RaptorDirectoryTest
            echo "Make ApopTest directory"
            mkdir ApopTest
            cd ./ApopTest
            echo "Get Apophenia zip file"
            wget https://github.com/b-k/Apophenia/archive/pkg.zip
            echo "Unzip Apophenia"
            unzip pkg.zip
            echo "Move to apophenia package directory"
            cd ./Apophenia-pkg
            echo "Configure"
            ./configure
            echo "Make"
            make
            echo "Make Install"
            sudo make install
            #move back to RaptorDirectoryTest
            cd ../../
            echo "Get VelociRaptor zip file"
            wget https://github.com/cecashon/OrderedSetVelociRaptor/archive/master.zip
            echo "Unzip"
            unzip master.zip
            echo "Move to VelociRaptor master"
            cd ./OrderedSetVelociRaptor-master
            echo "Build the program"
            make
            echo "Move program to install directory"
            mv VelociRaptor ../../
            cd ../../
            echo "Run with ./VelociRaptor"
            ./VelociRaptor
        fi    
    fi
else
  echo "Exit Install"
fi


