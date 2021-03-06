<h4 align="center">VelociRaptor Notes</h4> 

The VelociRaptor application was designed to help with the analyses of ordered data sets with a bias towards high throughput screening. Data sets that are generated from experiments designed and organized in an orderly fashion to facilitate automation and analysis. Data that is in order but comes in as a long continuous stream of numbers with no information about what each number represents. A fairly common stituation in statistical analysis.

If data is in order, then you can simplify some calculations. SQL makes many calculations easy but there are a few that turn out to be difficult in SQL(1). Calculations that may be tough in SQL can be accomplished in C. As a demonstration, the percent of control problem becomes easier and more flexible than it's SQL counterpart. That is the idea of the program. Make a few things easy to help with the analysis of large data sets.

In an ordered dataset it is important that data is not deleted. In SQL this is especially important with experimental results because if you delete a value anywhere in a set it makes it difficult to do ordered calculations on what remains especially if you didn't have an incremental counter field to give you an idea where the deletion occurred. Relational databases don't explicitly maintain the order of data but if data is inserted in order, initially it will be in order. Conceptually a little different than how a transaction processing system operates. It is a good idea not to delete experimental data and to flag the records you would like to ignore. If you get a NaN from a platereader don't delete it. If you are just using the interior 60 wells on a 96 well plate due to edge effects, use a SELECT statement to get at your 60 well data values and one or more incremental number fields to keep them in order. Try not to delete records and try to keep the order of the experiment intact. Keeping data intact and in order allows you to move data into and out of applications easily. The VelociRaptor application expects a filled number field. Data is read in as type double. A bit of a constraint but the functions expect numbers. Just don't throw away data to get there. 

Although a few calculations become easier to handle as ordered data sets it is still advantageous to store data in a relational database. In order to facilitate this, the data sets produced by the application can be output to a SQLite database by right clicking on a treeview dataset or using the Data menu drop down. The tables can be easily selected from using Sqliteman. This makes it easy to run calculations that SQL is good at such as if you want two times a sample standard deviation and want to output a row count, mean and standard deviation so that you can send it to gnuplot for an error plot graph. Easy to create a scatterplot of the percent of control values also. A couple of graphs are already connected in the application in this fashion.

Here are some hints on how to make use of the VelociRaptor application. The application can be started in the terminal window or by clicking on the program in the file manager. If the terminal window is open the application will print status reports to the terminal window. Nothing fancy but sometimes helpful. Once the application starts it is easy to generate some random data sets with the test data button. A fast way to experiment with how things work. Start with the default values in the fields and go from there. Calculate percent, make an auxiliary table and then right click on each treeview and send the data to the database.  That is all that is needed to do the graphing and analysis.

The random number generation and calculations should be very fast for datasets less than 100,000 records. For data sets larger than 100,000 records you are going to get some lag time. To import a text file with 100,000 numbers is very rapid also. Test it out.
 
In order to import a text file the file needs to be in column format. This means that the text file has a single column of numbers ending with the cursor below the last number in the list. This is how data is set up if you select a single field of numbers in Sqliteman, ctrl+c to copy it into the clipboard and ctrl+v to copy the data into a text editor. Save the data there and it will be ready to import. If the import is unsuccessful make sure that the column only has numbers and decimals in it. If the terminal window is open when importing a text file, the line numbers with the problem characters will be indicated. Having data in a column in a text file is a very simple and fast way to move data into the application. Once the data is imported make sure to change the parameter arguments to match your data. Nothing fancy here either and nothing automatic. Important! When importing data you need to set up your parameter values to match the data set before running any calculations. 

You can also import a text file in plate or matrix format with with common delimiters(space, ',', '\n' or '|'). The import will linearize your data reading across rows. 
 
Another way to import data is to use the Sqlite Connect menu to get a single column of data from a SQLite database. Be careful of implicit type conversions though. If importing text type data you might get a lot of zeroes from an incorrect type conversion. Something to be aware of. Once again, you need to set up your parameter values manually for the newly imported data before doing calculations.

For calculations, the optional positive and negative control values overide the Set Size for Stats value in the percent of control function and for generating random values for controls. If the optional controls are not checked the calculations will be performed in Set Size for Stats blocks down the column of numbers with the positive control the first block and negative control in the second block. The random numbers will generate your numbers in this pattern also unless you set the control values before you get the random numbers. If you set your control values the Set Size for Stats value is still needed. When the auxiliary table is built it needs a block size for default sets. You can make adjustments for your analysis if they aren't in block form by setting the Picks values in the auxiliary dialog. 

Once the data is loaded into the database and the auxiliary table is built, everything is setup for doing analysis and graphing. There are several methods for adjusting for multiplicity in a set. With a test dataset you can look at several different methods for comparison. There is also some simple formatting, heatmapping and graphing for taking a look at data. 

The program is open source so you are welcome to look at the code and make changes if you so wish.

Try to make sense of what you are doing. It is a test version and has a some work left to be done. It is an attempt to combine a bunch of scripts into a useful program with a graphical user interface. Thank you for taking a look.

C. Eric Cashon    
cecashon@aol.com

(1). SQLite percent of control. This is the SQL example. It isn't a very dynamic solution. There are other ways to solve this. You could go the procedural language route of some dialects of SQL or you might write the code in C or a combination of SQL and C. The following SQL script is setup to work with the default values in the VelociRaptor program after they have been loaded into the SQLite database.

```sql
SELECT T2.KeyID, T2.Data, (((T2.Data - T2.Neg)/(T2.Pos - T2.Neg)) *100) AS POC FROM (SELECT T1.KeyID, T1.Data, 
(SELECT CASE WHEN T1.KeyID % 96 <> 0 THEN (SELECT (SUM(T3.Data)/ 4) FROM Data T3 WHERE T3.KeyID >= (T1.KeyID / 
96) * 96 + 1 AND T3.KeyID <= (T1.KeyID / 96) * 96 + 4) ELSE (SELECT (SUM(T4.Data)/ 4) FROM Data T4 WHERE 
T4.KeyID >= ((T1.KeyID - 1) / 96) * 96 + 1 AND T4.KeyID <= ((T1.KeyID - 1) / 96) * 96 + 4) END) AS Pos, (SELECT 
CASE WHEN T1.KeyID % 96 <> 0 THEN (SELECT (SUM(T5.Data) / 4) FROM Data T5 WHERE T5.KeyID >= (T1.KeyID / 96) * 96 
+ 5 AND T5.KeyID <= (T1.KeyID / 96) * 96 + 8) ELSE (SELECT (SUM(T6.Data) / 4) FROM Data T6 WHERE T6.KeyID >= 
((T1.KeyID - 1) / 96) * 96 + 5 AND T6.KeyID <= ((T1.KeyID - 1) / 96) * 96 + 8) END) AS Neg FROM Data T1) AS 
T2;
```



