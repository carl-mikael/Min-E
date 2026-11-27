What do I want it to actually do?

- [x] Add
- [x] Iterate over components in a storage
- [ ] Remove
- [x] Keep components packed

- [ ] Free all arenas

Struggling a bit to see how to fit an ECS into the engine, in a way that makes sense. What would it look like if every "thing" that's being rendered right now became an entity? Aka what information does every "thing" need/use.

Store camera in player component? Not sure what to have in the player component so I'm trying to find something that could make sense, for the assignment.

The base instance has:
1. **Draw command**: describing which mesh data to use and how many instances
2. **Model matrix**: describing its transform
3. **Instance offset**: how to offset the position of the transform, in local space

- [x] **Mesh comp:** Draw command
	- [x] Add, and use for upload
	- [x] Update
	- [x] Reupload
- [x] **Transform comp:** Model matrix
- [x] **Texture comp:** handle to zoffset/index in 3D texture storage

Right now the order of draw commands is the order they are stored in.

Having a player component is a requirement. What does each player have/use?
1. A camera
2. Input bindings & callback events
3. A viewport
4. A background/clear color
5. The other player's camera transform to render a model there

The data is:
1. Camera: transform
2. Other player's camera: transform
3. Input
	1. Movement input: vec3
	2. This - Yaw input: float
	3. OR - Mouse data
		1. last_cursor_pos: vec2[double]
		2. current_cursor_pos/mouse_delta: vec2[double]

Camera transform is fair. And then you can get other player's cameras.
Maybe if the player is part of split screen stuff, like if it's active. Maybe move that player component outside the storage's component capacity. To remove the need for a flag, and still keep it close in memory.
Storing input is fair.

- [ ] Camera is already stoooooooored, I'm stooooring transforms. Wait, but I store camera in main as well, need to refactor this.