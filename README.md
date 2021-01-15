
# Simple OpenGL projects in C++
A collection of a a few OpenGL projects I had to write for my Computer Graphics course at Uni. All of them are mostly written in C++ using STL. Most of them rely on a framework defined in a different file. I can't upload those due to possbile copyright claims.

<br/>

**Arch Decresing program**

The goal was to draw a Catmull-Rom spline interpolated on randomly chosen control points. Color it with the triangluation method and after pressing the button "d", start approaching a circle by decreasing the arch of the random polygon, by calculating the arch decreasing vector in control points using Lagrange interpolation between them, effectively making a circle out of it.

<br/>

**Double slit**

The goal was to recreate the double slit experiment, by placing a laser inside of an infinite cilinder, with a "U" shaped hole on it (the hole was made using Bezier approximation) and having the light paint the interference lines of another transformed hyperboloid cone on the camera space. Transformation needed to be done by multiplying the transformation matrices and the model matrix, effectively chaning the explicit equiation of the points of the shape. The normal vector calculation is off as I couldn't figure out how to do it on time. The lights don't exactly work either.

<br/>

**Terrain**

The main goal was to create a terrain with a few hills (in which I succeeded) and color it using shaders, to make the "mountaintops" brown and the valley green (in which I also succeeded). We also had to create a train track with a Catmull-Rom Spline with the optical values of metal, and have it follow the terrain (which I couldn't do). We also had to run a train on this track by calculating every normal vector along the track and changing the camera setting (which I also couldn't do on time).


# Cpp-chat
Simple **WORK IN PROGRESS** C++ Chat program. Uses C++ sockets and can recieve a message through the network and save it in a conversation log file, enables user creation etc. 
