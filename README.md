# Polyhedron Generator
This repository represents a C++ implementation of the Conway Polyhedron Notation, allowing such polyhedra to be generated in a 3D space.

## General Description
Conway's Polyhedron Notation is a method for generating complex polyhedra by performing a set of operations on a seed polyhedron. This allows vastly complicated polyhedra, or polyhedral graphs, to be automatically produced either geometrically or topologically.

By implementing these operators, a 3D geometric representation of many polyhedra can be created and stored. Each different implementation is capable of producing a different geometric embedding for the corresponding set of operations' polyhedral graph. This geometry often requires further processing to maximise visibility and uniformity, by placing it into canonical form.

## How To Use
- Ensure the corresponding DLLs are present in the program's directory
- Execute using generated polyhedra.exe *operatorStream*:
	- *--operators* followed by *operatorStream* allows stream to be specified after first command-line-argument
	- *--shader* followed by one of *point tri line solid* selects starting shader
	- *--projection* followed by one of *ortho persp* selects starting camera projection
	- For example, *polyhedra adaT --shader solid --projection ortho* generates polyhedron with notation *adaT*, using shader *solid-wireframe*, with camera projection set to *ortho-graphic*
- Operate program using the following keybindings:
	- *p* toggles camera projection mode *(perspective, orthogonal)*
	- *g* cycles graphical shader mode *(points, triangles, lines, solid wireframe)*
	- *wasd left-control space* navigate free camera mode
	- *right-mouse-click* spin pivot camera mode
	- *e* switch to pivot camera mode
	- *tab* switch to free camera mode
	- *alt* exit window focus

## Compilation & Running Requirements
- Ensure MinGW compiler or compatible alternative is installed and accessible within the folder's environment
- Ensure GLEW and GLM header files are present
- Setup program using command *make prepare* then *make* on Windows operating system within the folder's directory to produce the *polyhedra.exe* executable
- Ensure the given DLLs are supplied to the executable's directory when compiling and executing

## Features
- Conway Polyhedron Notation operators including the following:
	- *T* tetrahedron seed; operators given to the left of seeds are ignored or processed into the preceding polyhedron
	- *d* duality, *a* rectification, *k* akisation, *g* gyro are fully implemented
	- *j* join, *n* needle, *z* zip, *t* truncate, *o* ortho, *e* expand, *s* snub, *m* meta, *b* bevel are processed as combinations of the implemented operators

## Implementation Contents
- Window generation for displaying results, built with SDL2
- Shader library for producing the graphical representation, built with OpenGL 3.3 Core Profile & GLSL
- Polyhedra mesh generation & storage for processing the notation operator stream
- Model storage for processing object shader data
- Camera library for navigating the 3D scene, built with GLM
- Utility libraries for inputting main function arguments, generating console statements for debugging, and file accessing of shader source files

## What To Add Next
- *Operators*: indexed and non-original operators allow more polyhedra to be generated
- *Canonical Form*: by using numerical methods, resulting polyhedra can be post-processed to minimise ambiguities
- *Model Importing/Exporting*: store resulting polyhedra as wavefront object models using .OBJ and .MTL files for the meshes and shaders
- *Multiple Polyhedra*: allow multiple polyhedra to be generated and combined in the 3D scene
- *Uniform Implementations*: implement more - and improve existing implementations of the current - operators, to ensure a more efficient and naturally uniform output.
- *Shaders*: add different shaders helpful in highlighting different properties of the displayed polyhedra, like face total-edges or area

## Citations
- Introduction to Conway Polyhedron Notation: https://en.wikipedia.org/wiki/Conway_polyhedron_notation