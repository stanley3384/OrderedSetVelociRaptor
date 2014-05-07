

--The VelociRaptor program helps with doing some ordered set statistical analysis in high throughput screening and assay development. Included are percent of control, Dunnett's test, ANOVA, Levene's test, Anderson Darling test, permutation tests, heatmaps and a few more. The statistical result sets are in tabular format making the data easy to move into any relational database. There are a couple of different ways to produce heatmapped platemaps along with a few other types of graphical output. 

Heatmap
![ScreenShot](/HeatmapReadme.jpg)

--The data sets can also be output to HTML in a heatmapped plate format. If there is a need to customize a layout with HTML there is a Python script along with a small C program that makes it easy to customize a layout with HTML.

Heatmap HTML's
![Screenshot2](/Heatmap2.jpg)


![Screenshot3](/Heatmap3.jpg)


--The VelociRaptor program requires a number of dependencies to compile. The following is how to get the required dependencies for the program on a computer running Ubuntu.

--Start with installing the Apophenia library and the libraries Apophenia depends on.

--Adapted from Ben Klemens README below. Take a look at the following for better information about installing Apophenia.

https://github.com/b-k/Apophenia

--Dependencies for Apophenia. 

sudo apt-get install make gcc libgsl0-dev libsqlite3-dev  
or   
sudo yum install make gcc gsl-devel libsqlite3x-devel  

--There is an optional Python interface, which will require all of the above plus the swig and Python-dev packages. *There isn't a requirement for this in the VelociRaptor program. The swig and Python-dev packages are good to have though.

--Once you have it in place,

tar xzf apophenia-*.tar.gz  
or  
Unzip the contents of the zip file into the new empty folder. 

./configure  
cd apophenia-0.99  
make   
sudo make install  

--After Apophenia is installed, install GTK+, gfortran and gnuplot.

sudo apt-get install libgtk-3-dev gnuplot gfortran

--Hint, you might want to build gnuplot with wxWidgets and Gd dev libraries installed if you can't get the wxt terminal to work. 

--That should be all for the dependencies. Next, create a new empty folder and extract the VelociRaptor-*.tar.gz  or *.zip file to the empty folder. 

--Example

cd /home/.../
mkdir OrderedSetVelociRaptor
cd ./OrderedSetVelociRaptor

--Unpack the archive into the folder.

tar xzf VelociRaptor-*.tar.gz 
or
Unzip the contents of the zip file to the new folder.

--Type make at the command line to build the program.

make 

--Start the program with

./VelociRaptor

--Test it out. There is a notes file along with a references file that might be helpful at this point.

C. Eric Cashon

--And, of course, thank you Github.