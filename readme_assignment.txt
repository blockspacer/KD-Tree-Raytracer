Assignment: Add a KDTree triangle search to the raytracer to make it faster

The provided code is not public domain unless othewise stated in the specific source file.

Included is a (very slow) raytracer that just brute force searches every triangle in a model and 
chooses the right (closest) one. Visual Studio project should compile and run out of the box. Also, 
a config file (raytracer.ini) and four, varying polycount versions of Stanford's dragon model are 
included. Brute force raytracing is slow even for lowpoly model and low resolution, but should be 
acceptable for modern computer (few minutes on default settings).

General requirements:
	- Use English (variable names, commenting etc.)
	- Try to match the style of the existing code
	- Use features that are known C++ characteristics like inheriting and/or templates
	- Performance isn't the main thing to focus on, more important is that code works (e.g. code 
	compiles, doesn't leak memory, doesn't crash). Adequate performance should be reachable without 
	lowlevel heroic optimizations
	- You can use known libraries for non-essential tasks (for example, if you want to add 
	visualization or something like that). You may not use libraries for actual task
	- List these matters to a separate document
	   - Used libraries, source code or other references you might have used (library names, books, URL etc.)
	   - Did you have any particular trouble doing the assignment
	   - How much time the implementation took and where did the time go

Particular requirements:
	1. Accelerated raytracer should be able to render highpoly version (_res1) in similar time it takes the brute force version to 
	render lowpoly version (_res4) or faster
	2. Code must not leak memory
	3. Code must compile out of the box, like the original code does
		3a. That is, if you use any third party libraries etc., you must include them
	4. Use existing Frozenbyte tools (containers, strings, vectors, etc)
	5. Do not disable warnings as errors or lower warning level
	6. You must clearly document all changes in the code. If you change project settings, include 
	an additional document explaining the changes
	7. You must also include a compiled exe (just in case)

For useful tips, we recommend:
	1. Wikipedia
	2. http://www.flipcode.com/archives/articles.shtml (Raytracing Topics & Techniques)
