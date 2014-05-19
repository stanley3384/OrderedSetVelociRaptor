#!/bin/bash

#An install script for the VelociRaptor program.
#This is still being tested!!! It is just a collection of the steps in README.
#Make sure to take a look at README before trying this script. Also take a 
#look at the script so you can see what it is doing and modify as needed.

#To use, make a new directory and copy the install.sh file to the new directory.
#A "test" directory would work fine. Then run the script.
#
# bash install.sh
# or 
# ./install.sh
#
#You should end up with the program and the install script in the new directory. 

read -p "Do you wish to install the VelociRaptor program? (y/n) " ANSWER
if [ "$ANSWER" = "y" ]; then
    #Check sudo permission.
    sudo -v
    RETURN=$?
    if [ $RETURN -ne 0 ]; then
        echo "Need sudo permission to install."
    else
        #See if github is available.
        ping -c 1 www.github.com
        STATUS=$?
        if [ $STATUS -ne 0 ]; then
            echo "Can't find github. Make sure you have an active internet connection and retry."
        else
            echo "Make RaptorDirectoryTest"
            rm -rf RaptorDirectoryTest
            mkdir RaptorDirectoryTest
            cd ./RaptorDirectoryTest
            echo "Get zip file"
            wget https://github.com/cecashon/OrderedSetVelociRaptor/archive/master.zip
            echo "Unzip"
            unzip master.zip
            echo "Move to Misc"
            cd ./OrderedSetVelociRaptor-master/Misc
            echo "Get dependencies"
            sudo apt-get update
            sudo apt-get install autoconf automake libtool 
            sudo apt-get install make gcc libgsl0-dev libsqlite3-dev
            sudo apt-get install libgtk-3-dev gnuplot gfortran
            echo "Unpack Apophenia"
            tar xzf apophenia-*.tgz
            echo "Apophenia directory"
            cd ./apophenia-0.99
            echo "Configure"
            ./configure
            echo "Make"
            make
            echo "Make Install"
            sudo make install
            echo "VelociRaptor directory"
            cd ../../
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


