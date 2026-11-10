---

kanban-plugin: board

---

## Backlog

- [ ] Understand the offsets/padding for std140: https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL
- [ ] Use 2 back frame buffers to avoid capping tick rate to monitor hz and still allow for vsync
- [ ] Slang as shader language. For shader lang features and reflection. https://shader-slang.org/
- [ ] Support vat (vertex animation texture)


## Next

- [ ] Impl my own obj loader.
	Handle at least these lines:
	- [ ] \#
	- [ ] o
	- [ ] v
	- [ ] vt
	- [ ] vn
	- [ ] f
- [ ] f lines should support quads too
- [ ] Do basic validation. Don't let I/O crash app
- [ ] Allow toggling between obj loaders
- [ ] The obj loader shouldn't have to load the same file twice
- [ ] render_obj(string name, vec3 pos)
	Should not trigger unneeded data transfers to the GPU
- [ ] normals in at least one shader
- [ ] crude directional light
	- [ ] test normals
- [ ] support textures in at least one shader
- [ ] Scene populated with at least 5 imported objects


## Now

- [ ] Integrate assimp


## Done



***

## Archive

- [ ] Learn RenderDoc
- [ ] Go over the code line by line
- [ ] Ensure you can explain the webgl cubes: https://webglfundamentals.org/webgl/lessons/resources/webgl-state-diagram.html?exampleId=triangle#no-help
- [ ] Ensure you understand all the matrices
- [ ] Try indirect multidraw
- [ ] Instance a struct with alignas and upload directly, the first time, instead of using sub-data. For the ubo
- [ ] Try instancing
- [ ] Don't use geom shader for duplication
- [ ] Change between perspective and orthographic view on space
- [ ] Render 2 spheres, 2 pyramids, and 2 cubes
- [ ] Possible to duplicate the shapes with an offset using a geometry shader?
- [ ] Size pyramid over time/frames
- [ ] Render pyramid
- [ ] Move sphere around over time/frames
- [ ] Impl matrices and use uniform buffer for perspective & view: https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL
- [ ] Watch linear transforms and matrices: https://www.youtube.com/watch?v=kYB8IZa5AuE&list=PLZHQObOWTQDPD3MizzM2xVFitgF8hE_ab&index=4
- [ ] Watch matrix multiplication as composition: https://youtu.be/XkY2DOUCWMU?si=MHkV9EOWBNw5a-mF
- [ ] Spin cube over time/frames
- [ ] Render sphere
- [ ] Is there a better way of creating the uniform buffer? Right now it's using GL_DYNAMIC_STORAGE_BIT
- [ ] Ensure you can explain the n0-help example
- [ ] glGetError: https://learnopengl.com/In-Practice/Debugging
	Use Debug Output instead
- [ ] Render House
- [ ] draw_triangle
- [ ] draw_rectangle
- [ ] Add teachers to git repo
- [ ] debug function that only executes in debug mode, and prints the function it's in

%% kanban:settings
```
{"kanban-plugin":"board","list-collapse":[false,false,false,false]}
```
%%