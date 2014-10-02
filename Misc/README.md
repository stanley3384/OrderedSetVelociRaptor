

--Some miscellaneous items to experiment with. For the OpenGL programs, on Ubuntu, you can get the needed libraries to compile the examples with by getting the freeGLUT and Mesa libraries. If you have a good graphics card you might not need the mesa libraries.

sudo apt-get install libglu1-mesa-dev freeglut3-dev mesa-common-dev

--The plate_stack_viewGL2 program provides a 3d view of heatmapped stacked microtiter plates. You can pull in data from the VelociRaptor database or run uniform random data sets to experiment with. There are three different heatmap settings that you can choose from to get a 3d visual on the data. It uses the fixed OpenGL pipeline.

3d Heatmap

![ScreenShot](/Misc/platemapGL1.jpg)


--If you want to test out vertex and fragment shaders the gtk3shadersGL.c program is a simple program using the dynamic pipeline in OpenGL.

Shaders

![ScreenShot](/Misc/vertex.jpg)


--There are a variety of other programs that are small test programs. Good for figuring out how a certain technology works and easy to change around. For testing threads, linear gradients and radial gradients the css1.c program is a start. It uses Cairo along with CSS for some 2d color.

Threads

![ScreenShot](/Misc/threads.jpg)


C. Eric Cashon

