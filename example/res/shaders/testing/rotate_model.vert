#version 460 core
layout (location = 0) in vec3 pos;

layout (binding = 0) uniform Camera {
    mat4 pr_matrix;
    mat4 vw_matrix;
} camera;

layout (binding = 1) uniform Model {
    mat4 md_matrix;
} model;

layout (location = 0) out vec3 frag_pos;

void main() {
    gl_Position = camera.pr_matrix * camera.vw_matrix * model.md_matrix * vec4(pos, 1.0);
    frag_pos = vec3(model.md_matrix * vec4(pos, 1.0));
}