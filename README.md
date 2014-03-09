

--The VelociRaptor program helps with doing some ordered set statistical analysis in high throughput screening and assay development. Included are percent of control, Dunnett's test, ANOVA, Levene's test, Anderson Darling test, permutation tests, heatmaps and a few more. The statistical result sets are in tabular format making the data easy to move into any relational database.

Heatmap
![ScreenShot](/HeatmapReadme.jpg)

--The VelociRaptor program requires a number of dependencies to compile. The following is how to get the required dependencies for the program on a computer running Ubuntu.

--Start with installing the Apophenia library and the libraries Apophenia depends on.

--Adapted from Ben Klemens README below. Take a look at the following for better information about installing Apophenia.

https://github.com/b-k/Apophenia

--Dependencies for Apophenia. 

sudo apt-get install gsl-bin libgsl0-dev libblas-dev libblas3gf liblapack3gf sqlite3 libsqlite3-0 libsqlite3-dev

--There is an optional Python interface, which will require all of the above plus the
swig and Python-dev packages. *There isn't a requirement for this in the VelociRaptor program.

--Once you have it in place,

tar xzf apophenia-*.tar.gz
or
Unzip the contents of the zip file into the new empty folder. 

cd apophenia-X.XX
./configure; make; sudo make install

--After Apophenia is installed, install GTK+, gfortran and gnuplot.

sudo apt-get install libgtk-3-dev gnuplot gfortran

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