

--The VelociRaptor program helps with doing some ordered set statistical analysis in high throughput screening and assay development. Included are percent of control, Dunnett's test, ANOVA, Levene's test, Anderson Darling test, permutation tests, heatmaps and a few more. The statistical result sets are in tabular format making the data easy to move into any relational database. There are a couple of different ways to produce heatmapped microtiter platemaps along with a few other useful types of graphical output. 

Heatmap
![ScreenShot](/HeatmapReadme.jpg)

--The data sets can also be output to HTML in a heatmapped microtiter plate format or in tablular format. If there is a need to customize a layout with HTML there is a Python script along with a couple C programs that make it easy to customize a layout with HTML. With a little HTML and CSS you can get a print layout exactly how you want it.

Heatmap HTML's
![Screenshot2](/Heatmap2.jpg)


![Screenshot3](/Heatmap3.jpg)


--The VelociRaptor program requires a number of dependencies to compile. The following is how to get the required dependencies for the program on a computer running Ubuntu. First download the OrderedSetVelociRaptor-master.zip file into a new folder.

--Start with installing the Apophenia library and the libraries Apophenia depends on.

--Adapted from Ben Klemens README below. Take a look at the following https://github.com/b-k/Apophenia for better information about Apophenia. The VelociRaptor program doesn't compile correctly with the most recent version of Apophenia(version 0.999). Working at it(See Misc/install2.sh). For now, there is a previous version of Apophenia(version 0.99) in the Misc folder that will work. Start there.

cd ./Misc

--Dependencies for Apophenia. 

sudo apt-get install make gcc libgsl0-dev libsqlite3-dev
 
sudo apt-get install autoconf automake libtool  

--Unpack the file.

tar xzf apophenia-*.tgz   

--Move to the directory.

cd ./apophenia-0.99 

--Set it up.

./configure   
make   
sudo make install  

--After Apophenia is installed, install GTK+, gfortran and gnuplot.

sudo apt-get install libgtk-3-dev gnuplot gfortran

--Hint, you might want to build gnuplot with wxWidgets and Gd dev libraries installed if you can't get the wxt terminal to work. 

--cd back to OrderedSetVelociRaptor-master$

cd ../../

--Type make at the command line to build the program.

make 

--Start the program with

./VelociRaptor

--Or, if you want to run the above lines in an install script, there is an install.sh file in the Misc folder that will accomplish this. You would just need to copy the install script to a new folder and run it. It is recommended that you take a look at the script before running it to see how it works.

./install.sh

--Test it out. There is a notes file along with a references file that might be helpful at this point.

C. Eric Cashon

--Thank you Github and the work of all that made it possible to put together.