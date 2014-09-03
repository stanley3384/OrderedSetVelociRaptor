
//vertex shader for plate_stack_viewGL3.c and gtk3shadersGL.c

void main()
{	
   //Check if shader is working.
   vec4 current_color = gl_Color;
   //Change red side of plane to yellow.
   if(current_color[3]<0.8) gl_FrontColor=vec4(1.0,1.0,0.0,1.0);
   else gl_FrontColor = gl_Color;
   gl_Position = ftransform();
}
