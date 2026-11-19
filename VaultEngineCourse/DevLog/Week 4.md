I like that this doesn't limit the user in which mat4 they use, that it doesn't need headers in the .h file, that it doesn't need heap allocation, doesn't use templating, and that it ensures the correct alignment and size. But, the copy for the user, and in runtime, is a bit disgusting.

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