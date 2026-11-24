---

kanban-plugin: board

---

## Backlog

- [ ] Understand the offsets/padding for std140: https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL
- [ ] Use 2 back frame buffers to avoid capping tick rate to monitor hz and still allow for vsync
- [ ] Super set of GLSL, custom pre parsing before uploading source.
	Or
	Slang as shader language. For shader lang features and reflection. https://shader-slang.org/
- [ ] Support vat (vertex animation texture)
- [ ] vertex pulling
- [ ] clang tidy
- [ ] 3 person camera with mouse control. Extra
- [ ] Window resizing


## Next

- [ ] Add documentation / comments


## Now



## Done



***

## Archive

- [ ] Add mouse rotation control
	- [x] Hide cursor
- [ ] Camera control
	- [x] yaw
- [ ] The players should be able to see each other
- [ ] Flying camera
	- [x] Second player
- [ ] Input System
	- [x] Support multiple players
- [ ] camera1/2 are not model matrices for cameras, but their inverse, bad names.
- [ ] Understand the camera math
- [ ] scene isn't displayed correctly
	If instance isn't first in scene file it's not rendered?
- [ ] Scenes in runtime
	- [x] Switch scene
	- [x] Teleport the players back to start
- [ ] Use scene
	- [x] render objects
	- [x] spawn camera at start pos
- [ ] Scene
	- [x] Custom file format
	Should include:
	- [x] Scene name
	- [x] A start pos
	- [x] A goal pos
	- [x] Object positions
- [ ] Split screen
	- [x] Toggle on/off during runtime
- [ ] Delta time
- [ ] add compilation flags to get warnings
- [ ] Do basic validation. Don't let I/O crash app
- [ ] Allow toggling between obj loaders
- [ ] The obj loader shouldn't have to load the same file twice
- [ ] render_obj(string name, vec3 pos)
	Should not trigger unneeded data transfers to the GPU
- [ ] normals in at least one shader
- [ ] support textures in at least one shader
- [ ] crude directional light
	- [x] test normals
- [ ] f lines should support quads too
- [ ] Impl my own obj loader.
	Handle at least these lines:
	- [x] \#
	- [x] o
	- [x] v
	- [x] vt
	- [x] vn
	- [x] f
- [ ] Add model matrix per model
- [ ] Scene populated with at least 5 imported objects
- [ ] Integrate assimp
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
- [ ] Update command buffer dynamically.
	Use 2 buffers to ping-pong between. So all the work stays on the GPU.
	Update draw count based on it.

%% kanban:settings
```
{"kanban-plugin":"board","list-collapse":[false,false,false,false]}
```
%%