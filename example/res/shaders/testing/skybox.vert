#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

layout (location = 0) out vec3 frag_pos;
layout (location = 1) out vec3 frag_normal;
layout (location = 2) out vec2 frag_uv;

layout (binding = 0) uniform Camera {
    mat4 projection;
    mat4 view;
} camera;

void main() {
    gl_Position = camera.projection * vec4(mat3(camera.view) * pos, 1.0);

    frag_pos = pos;
    frag_normal = normal;
    frag_uv = uv;
}