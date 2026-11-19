- Problem: my current renderer API doesn't enforce or not even showing how values should be formatted. Users would be completely blind to how to use it. Enough to just comment/document?
- This is pretty hype - from text file (the first cube is hidden behind the tree):![[Pasted image 20261119224528.png]]
![[Pasted image 20261119224608.png]]
- I really want to create a memory arena
	- I like the idea of using a master arena for the whole engine, which can be divided into sub arenas.
	- And replacing all std::vectors and std::strings.

- I used ai  to get feedback on the api (on the code further down) and made changes based on it. Ai said the data doesn't have to be aligned on the cpu side, which makes sense, but haven't double checked. Cpu -> gpu data transfer is still a black box concept for me. And it also made a point that the struct doesn't enforce data size from the user, it only enforces it on the upload. I'd argue it still holds some value to do it like this but not worth it when the user has to copy data like that.
- I like that this doesn't limit the user in which mat4 they use, that it doesn't need headers in the .h file, that it doesn't need heap allocation, doesn't use templating, and that it ensures the correct alignment and size. But, the copy for the user, and in runtime, is a bit disgusting.

// renderer.h
#define M44Size 64
struct matrix_ubo
{
uint32_t handle;
alignas(16) char opaque[M44Size * 2];
};
struct matrix44
{
alignas(16) char opaque[M44Size];
};
void set_perspective(matrix_ubo &ubo, matrix44 perspective);
void set_view(matrix_ubo &ubo, matrix44 view);
matrix_ubo test();

// user .cpp
renderer::matrix_ubo mubo = renderer::test();
renderer::matrix44 m44{};
std::memcpy(m44.opaque, glm::value_ptr(perspective), sizeof(perspective));
renderer::set_perspective(mubo, m44);
std::memcpy(m44.opaque, glm::value_ptr(camera1), sizeof(camera1));
renderer::set_view(mubo, m44);