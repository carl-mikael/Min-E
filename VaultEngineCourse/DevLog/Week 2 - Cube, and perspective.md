- The vao is created the first time it's bound, not after the handle is generated.
Learned this after trying to bind the ebo to the vao before binding it the first time.

- Used RenderDoc early to see if the cube was correct and noticed I had forgotten top and bottom faces.
![[Pasted image 20261103104919.png]]
Boom!
![[Pasted image 20261103110545.png]]

- Had to add GL_DYNAMIC_STORAGE_BIT to allow the data to be changed with SubData. I wonder if it's better to use one "build" buffer which is then copied over to the target buffer, so the target buffer wouldn't need this bit. Or, if there's another better way.![[Pasted image 20261103141140.png]]

- Accidentally used -z for camera pos and was confused to why the cube was off center to the right when I only used positive vertices for it and didn't translate it to the left. I had forgotten -z is "into" the screen and +z "out of" the screen.

- AAAAGH, I was stuck for so long using sizeof() on the vector. It doesn't return the size it holds, only how big the vector type is, I think. Replaced it with sizeof(whatever the vector holds) * vector.size()