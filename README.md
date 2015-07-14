

--The VelociRaptor program helps with some ordered set statistical analysis in high throughput screening and assay development. Included are percent of control, Dunnett's test, ANOVA, Levene's test, Anderson Darling test, permutation tests, heatmaps and a few more. The statistical result sets are in tabular format making the data easy to move into any relational database. There are a couple of different ways to produce heatmapped microtiter platemaps along with a few other useful types of graphical output. 

Descriptive Statistics
![ScreenShot](/statistics.png)

--Data sets can also be output to HTML from the application in a heatmapped microtiter plate format or in tablular format. If there is a need to customize a layout with HTML there is a Python script along with a couple C programs that make it easy to customize a layout with HTML. With a little HTML and CSS you can get a print layout exactly how you want it. 

--An additional utility program for formatting data for printing is report_generator.py which can generate a basic report layout for grids, tables and crosstabs from the sqlite database. The program is also written in C and called OSVreport_generator.c. The C version can do the redraws and scrolling of the drawings faster than the python version. The OSVreport_generator program is built along with the VelociRaptor program using make or it can be built by itself. For testing how it works there are a couple of different sequences that you can choose from that can be easily heatmapped to see a couple of general patterns. 


Some Heatmap Layouts
![Screenshot2](/Heatmap1.png)

![Screenshot3](/table1.jpg)

![Screenshot4](/Heatmap3.jpg)

--There are various graphical items in the Misc folder in both 2d and 3d. Take a look at the OpenGL graphing programs with eigen vectors and values. Or the plate_stack_viewGL2 program for a 3d stacked heatmapped view of microtiter plate data.

Magic Square
![Screenshot4](/wave1.jpg)


--The VelociRaptor program requires a number of dependencies to compile. The following is how to get the required dependencies for the program on a computer running Ubuntu 14.04 with GTK+ 3.10.

--Start with making a new folder or directory. Then get the needed files for building the program. 

--Adapted from Ben Klemens README below. Take a look at the following https://github.com/b-k/Apophenia for better information about installing and using the Apophenia library.

--Dependencies for Apophenia. 

sudo apt-get install make gcc libgsl0-dev libsqlite3-dev 

--Get the Apophenia package.

wget https://github.com/b-k/Apophenia/archive/pkg.zip

--unzip and build the library.

unzip pkg.zip

cd apophenia-pkg

./configure

make

sudo make install

--After Apophenia is installed, install GTK+, gfortran and gnuplot.

sudo apt-get install libgtk-3-dev gnuplot gfortran

--Hint, you might want to build gnuplot with wxWidgets and Gd dev libraries installed if you can't get the wxt terminal to work. 

--Move back to the main folder and get the VelociRaptor files.

cd ../

wget https://github.com/cecashon/OrderedSetVelociRaptor/archive/master.zip

--Unzip and build the program.

unzip master.zip

cd ./OrderedSetVelociRaptor-master

make 

--Start the program with

./VelociRaptor

--Or, if you want to run the above lines in an install script, there is an install.sh file in the Misc folder that will accomplish this. You would just need to copy the install script to a new folder and run it. It is recommended that you take a look at the script before running it to see how it works. That way you can modify it and see how the pieces fit together. 

./install.sh

--Test it out. There is a notes file along with a references file that might be helpful at this point. There are a few extra items in the Misc folder that might also be of interest.
 
C. Eric Cashon

--Thank you Github and the work of all that made it possible to put together.

![Screenshot5](/dino.png)