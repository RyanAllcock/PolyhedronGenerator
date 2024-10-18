# Polyhedron Generator
This repository represents a C++ implementation of the Conway Polyhedron Notation, allowing such polyhedra to be generated in a 3D space.

![dckcscO](https://github.com/RyanAllcock/PolyhedronGenerator/assets/133653065/4121f4ff-f90f-4f1c-98e7-311ddae99682)

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
	- To convert shapes into canonical form, decorate operator stream with *c* operators (e.g. *ctdaT*)
	- Note: complicated shapes may require multiple *c* operators spread throughout (e.g. cdckcdccgcD), or even splitting of compound operators (e.g. replace *s* with *dgd*), otherwise use *c* sparingly to avoid diverging the result
- Operate program using the following keybindings:
	- *p* toggles camera projection mode *(perspective, orthogonal)*
	- *g* cycles graphical shader mode *(points, triangles, lines, solid wireframe)*
	- *wasd left-control space* navigate free camera mode
	- *right-mouse-click* spin pivot camera mode
	- *e* switch to pivot camera mode
	- *tab* switch to free camera mode
	- *alt* exit window focus
	- *c v b n m* add *c d a k g* operators to stream, respectively
	- *x* remove last dynamically-added operator from stream
	- *t* export current operator stream to file

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
	- *c* canonical form

## Implementation Contents
- *Window generation* for displaying results, built with SDL2
- *Shader library* for producing the graphical representation, built with OpenGL 3.3 Core Profile & GLSL
- *Polyhedra mesh generation & storage* for processing the notation operator stream
- *Model storage* for processing object shader data
- *Camera library* for navigating the 3D scene, built with GLM
- *Utility libraries* for inputting main function arguments, generating console statements for debugging, and file accessing of shader source files

## Algorithms

### Explicit Operators
#### Duality
- new vertices: old face centres
- new faces: winded, ordered face indices around each old vertex
- new edges: unique edges from new faces
#### Rectification
- new vertices: old edge midpoints
- new faces: winded, ordered edge indices around each old face and each old vertex
- new edges: unique edges from new faces
#### Akisation
- new vertices: old vertices and old face centres
- new edges: old edges and old face vertices to their old face centres
- new faces: each old face edge to centre
#### Gyration
- new vertices: old vertices, 2 midway along each old edge, and at old face centres
- new edges: along new vertices along each old edge and from each first winded, ordered new vertex along an old edge to it's old face centre
- new faces: winded, ordered edge loops between each pair of new face centre edges

### Canonical Form Relaxation
- (1) for each edge, add the difference between it's closest midpoint to the origin and that midpoint's unit vector to the edge's vertex pair
- (2) subtract the vertices' centre of gravity from each vertex
- (3) for each face, calculate an approximate flat plane face, then for each vertex, and add a proportion of the difference between it and the plane in the direction of the plane's normal
- repeat steps (1), (2), then (3), until the maximum change in position of any vertex reaches a minimum tolerance, or until a maximum iteration count is reached
- (1) ensures edges are tangent to the origin, (2) ensures the shape is centred on the origin, and (3) ensures each face is flat

## What To Add Next
- *Active Generation*: allow polyhedra to be developed and cached, to guide the process of polyhedron creation
- *Operators*: indexed and non-original operators allow more polyhedra to be generated
- *Model Importing/Exporting*: store resulting polyhedra as wavefront object models using .OBJ and .MTL files for the meshes and shaders
- *Multiple Polyhedra*: allow multiple polyhedra to be generated and combined in the 3D scene
- *Uniform Implementations*: implement more - and improve existing implementations of the current - operators, to ensure a more efficient and naturally uniform output.
- *Shaders*: add different shaders helpful in highlighting different properties of the displayed polyhedra, like face total-edges or area

## Citations
- Conway Polyhedron Notation Introduction: https://en.wikipedia.org/wiki/Conway_polyhedron_notation
- Canonical Form Algorithm Academic Paper: https://library.wolfram.com/infocenter/Articles/2012/