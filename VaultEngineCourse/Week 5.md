- I didn't remember that an array of scalar had weird alignment rules using std140 for an ubo. If I understand correctly, each scalar typ in glsl is represented using 4 bytes. And for some reason that  I don't understand, each element in array[scalar] has the same alignment as a vec4 (16). This is crazy, I'm gonna see if maybe I can pack them. Maybe use modulo to unpack?
- Tried this. Only using if in shader when testing. I think the data I get from texture_map is still just 4 bytes, so it won't work. Changed to ivec2 and it's working! Started by packing only 2, I'll make it 4 now.![[Pasted image 20261127144544.png]]

- I'm lost when it comes to all texture params. Like format, I create 3D texture storage using GL_RGB8 but upload textures to it using GL_RGB?? I think layer is which mipmap to use.

- Map textures using UBO, SSBO, or VBO? I'll start off just using gl_DrawID.
	- VBO with divisor like with instance offsets
	- UBO is like smaller but faster SSBO so I'll always start with UBO over SSBO.
	- I think UBO is more flexible then VBO so I'll probably use that. But, tbh, I need to research more about how the approaches would differ.

- I can't store sampler2DArray in ubo

- Switching "mesh" component, first with last. Swapping the data the components store and reuploading draw commands. Would be better to only replace the new data needed. I think it's because it's so much new stuff that I learn/realize something, and then forget about it. While implementing this I was thinking I can't replace data in the middle  using glNamedBufferSubStorage, even thought I use that to update/set view & projection, and same for matrices ubo. Fixed it.
![[Switching'Mesh'Comp.mp4]]
- decltype is my new best friend. So far I've used "multiple sources of truth", having the type of a field defined in the struct and hardcoding the type in sizeof:s.

- I'll either have to use templating and perfect forwarding (I think it's called) for in place construction in the backend or allow the user to be "stupid" and use more mem then they allocated.
![[Pasted image 20261124221324.png]]
Image shows the user (me) allocating 4 bytes, then using sizeof("This is fucked tho?"). Then doing the same thing again. And finally outputting the 2 strings, showing how they overlap.

- Used VirtualAlloc2 for allocation, windows function. I kind of just made it work, I don't understand all the different flags. Just dirtied my toes a bit, will look more into it later when it matters more, or when I have time. Right now I need to make *stuff*, to meet the weekly deadline.

Custom allocator time (VirtualAlloc2): https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc2fromapp
Arena example: https://medium.com/@sgn00/high-performance-memory-management-arena-allocators-c685c81ee338

Just skimmed through, saving this here for quick access if I feel like reading more: https://sploitfun.wordpress.com/2015/02/10/understanding-glibc-malloc/

Learning about DOD, data oriented design.
https://www.youtube.com/watch?v=IroPQ150F6c

- Structure of arrays increases cache locality when performing operations on a specific attribute/data. It also removes padding.
- If possible use a 4byte int, instead of pointers as they take 8bytes.
- Avoid storing the type bool. It has the cost of at least 1 byte, often more thanks to padding, and only stores the information worth of 1 bit.
- Instead of storing a bool, you could remove the field and store the struct in two different arrays instead.
- If the struct contains information that's rarely used, i.e. an NPCs health, it could be stored separately and be looked up only when needed. If we want to update all NPCs positions every frame, it would be unnecessary to waste cache space on their health.
- Math instruction are extremely cheap on the CPU, memory instructions are not. Consider lazily calculating values over saving/looking them up. (Lazily calculating something is calculating it on the spot when you need it).
- You could interpret data in different ways depending on context. The guy in the video talk called it encoding. Can sometimes be done over polymorphism.

This week we have the choice of creating an ECS or do text rendering. I'm not very interested in text rendering so I'll do an "ESC". Though, the more I think about ECS, the less I like they way they want us to do it in the assignment.
If my understanding of ESC is correct, the the goal with it is to add cache locality, usually at the price of complexity. But, if you have entity can have an empty component that takes up space, that works against that. And, what's the point of locality of data if you don't use procedures on the local data? I.e. why would I have a component type which holds data that I don't run procedures on? Like, what's the point of that. Wouldn't it be better to just not stored that thing as a component, i.e. on the entity directly or somewhere else indirectly if it isn't used often with the entity.
Actually, saving data you don't usually use, as components, kind of makes sense as it becomes the indirect lookup mentioned above.