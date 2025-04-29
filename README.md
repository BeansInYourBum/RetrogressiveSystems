# RetrogressiveSystems
A very lightweight game engine designed to act like a retro console

The library was written in C but you can use C++ if you want.

Set up the game in the RGSConfigure function but don't load any resources here, wait until RGSBegin is called before using the other functions. RGSDraw* functions and Get/Set/Read/Write pixel(s) can only be used in the RGSRender function

An empty project has been included in the examples to show how to set up each project and the build tasks for VSCode. When copying the project remember to add a '.' character to the front vscode folder name. 

So far this is for Windows only.

