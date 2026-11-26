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
- [ ] **Transform comp:** Model matrix

Right now the order of draw commands is the order they are stored in.