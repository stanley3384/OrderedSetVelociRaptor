

--Some miscellaneous items to experiment with. For the OpenGL programs you can get the needed libraries to compile the examples by getting the freeGLUT and the Mesa libraries on Ubuntu. Some of the OpenGL programs use GTK+ for the UI and some use freeGLUT for the UI.

sudo apt-get install libglu1-mesa-dev freeglut3-dev mesa-common-dev

--There are a couple of Python(version 2.7.3) test scripts in the Python folder. Some of them are from answers given on the GTK+ Forum. They might be helpful in figuring out a few things with Python and GTK+. If you are interested in automating LibreOffice apps with Python and GTK+ there is a short Writer and Calc example in there also.

--In the encryption folder there are a couple of examples encrypting values into a SQLite database using a simple character shift, Twofish and Rijndael encryption. There is some test code using both the Mcrypt and the OpenSSL libraries.  

--The plate_stack_viewGL2 program provides a 3d view of heatmapped stacked microtiter plates. You can pull in data from the VelociRaptor database or run uniform random data sets to experiment with. There are three different heatmap settings that you can choose from to get a 3d visual on the data. It uses the fixed OpenGL pipeline.

3d Heatmap

![ScreenShot](/Misc/platemapGL1.jpg)


--If you want to test out vertex and fragment shaders the gtk3shadersGL.c program is a simple program using the dynamic pipeline in OpenGL.

Shaders

![ScreenShot](/Misc/vertex.jpg)


--There are a variety of other programs that are small test programs. Good for figuring out how a certain technology works and easy to change around.

Thread Pool

![ScreenShot](/Misc/threads.png)

Bit Calc

![ScreenShot](/Misc/bit_calc.png)


--There are a lot of 2d and 3d drawing capabilities on computers. Very useful for data visualization and a little bit of fun drawing with color. If you are looking for some simple programs with sound, threads, sockets, time, etc. Take a look around in some of the above folders. There might be something useful.

C. Eric Cashon

