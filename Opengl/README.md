# Simple OpenGL projects in C++
A collection of a a few OpenGL projects I had to write for my Computer Graphics course at Uni, all of them are mostly written in C++ using STL and all that. Most of them rely on a framework defined in a different file. I can't upload those due to possbile copyright claims or whatever.

<br/>

**Arch Decresing program**

The goal was to draw a Catmull-Rom spline interolated on randomly chosen control points. Color it with the triangluation method and after pressing the button "d" start approaching a circle by decreasing the arch of the random polygon, by calculating the arch decreasing vector in control points using Lagrange interpolation between them effectively making a circle out of it.

<br/>

**Double slit**

The goal was to recreate the double slit experiment, by placing a laser inside of an infinite cilinder, with an U shaped hole on it (the hole was made using Bezier approximation) and having the light paint the interference lines of another transformed hyperboloid cone on the camera space. Transformation needed to be done by multiplying the transformation matrices and the model matrix, effectively chaning the explicit equiation of the points of the shape. The normal vector calculation is off but oh well, the lights don't exactly work either but they're there

<br/>

**Terrain**

The main goal was to create a terrain with a few hills, in which I succeeded, and color it using shaders, to make the "mountaintops" brown and he valley green, in which I also succeeded. Then U had to create a Catmull-Rom shaped train track with the optical values of metal, and have it follow the terrain ... this didn't exactly work out. Also we had to run a train on this track by calculating every normal vector along the track and changing the camera setting. This task didn't exactly work out as planned as I didn't have time to finish it but oh well
