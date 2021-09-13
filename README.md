# MinecraftCloneCPPOpenGL
A Minecraft clone which I made from scratch over the course of a few days of working on it. In this clone you can explore the plains, desert, snow biomes, and caves, whilst altering the world in any way that you like. A showcase for this project can be found here: https://youtu.be/4RPGVNyXpUc 
  
# Controls:  
  
Num-Keys - select block including:  
1 - grass  
2 - dirt  
3 - stone  
4 - cobblestone  
5 - wood  
6 - log  
7 - brick  
8 - gravel  
9 - leaves  
0 - sand  
  
W - Move forwards  
S - Move backwards  
A - Move left  
D - Move right  
Space - Move up  
Shift - Move down  
  
Move Mouse - Look around  
Left Click - Break block  
Right Click - Place block  
  
Escape - Close Program  
  
# Altering Textures  
Textures can be altered in this project by opening the "texAtlas2" image in an image editor, and simply changing the part of the texture atlas which you would like to alter to change how certain blocks look in game.  
  
  
# Altering World Generation  
  
This project includes my own Perlin Noise class, where the parameters of said perlin noise can be altered to change how the world is generated including the number of iterations, amplitude, damping (over each iteration), scale (how the perlin noise grid should be scaled (larger values will cause "hills" and "valleys" to cover a larger surface area)), and ratioFactor (the scale of each iteration follows a geometric series of the form scale * ratioFactor^(n-1) where n is the current iteration). To change this however, recompilation would be required.  
  
Changing the world size is possible through changing the chunkCount and chunkSize variables (where width of the world = chunkCount * chunkSize), however this would require recompilation of the program. A smaller world would be quicker to generate and to run (higher framerates) than a larger world. World generation may take some time (a matter of minutes) as it is currently CPU bound.  
