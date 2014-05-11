

--The VelociRaptor program helps with doing some ordered set statistical analysis in high throughput screening and assay development. Included are percent of control, Dunnett's test, ANOVA, Levene's test, Anderson Darling test, permutation tests, heatmaps and a few more. The statistical result sets are in tabular format making the data easy to move into any relational database. There are a couple of different ways to produce heatmapped platemaps along with a few other types of graphical output. 

Heatmap
![ScreenShot](/HeatmapReadme.jpg)

--The data sets can also be output to HTML in a heatmapped plate format. If there is a need to customize a layout with HTML there is a Python script along with a small C program that makes it easy to customize a layout with HTML.

Heatmap HTML's
![Screenshot2](/Heatmap2.jpg)


![Screenshot3](/Heatmap3.jpg)


--The VelociRaptor program requires a number of dependencies to compile. The following is how to get the required dependencies for the program on a computer running Ubuntu. First download the OrderedSetVelociRaptor-master.zip file into a new folder.

--Start with installing the Apophenia library and the libraries Apophenia depends on.

--Adapted from Ben Klemens README below. Take a look at the following https://github.com/b-k/Apophenia for better information about Apophenia. The VelociRaptor program doesn't compile correctly with the most recent version of Apophenia(version 0.999). Working at it. For now, there is a previous version of Apophenia(version 0.99) in the Misc folder that will work. Start there.

cd ./Misc

--Dependencies for Apophenia. 

sudo apt-get install make gcc libgsl0-dev libsqlite3-dev   

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

--Test it out. There is a notes file along with a references file that might be helpful at this point.

C. Eric Cashon

--And, of course, thank you Github.