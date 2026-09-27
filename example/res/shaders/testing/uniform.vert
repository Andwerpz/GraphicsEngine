#version 460 core
layout (location = 0) out vec2 uv;

// layout (binding = 0) uniform Camera {
//     mat4 pr_matrix;
//     mat4 vw_matrix;
// } camera;

// struct A {
//     float foo;
//     uint bar;
// };

// struct Foo {
//     mat4 mat1;
//     vec3 vec1;
//     mat4 mat;
//     float arr[3][100];
//     A arr2[3];
// };

// layout (binding = 1) uniform Bar {
//     Foo foo;
//     mat4 asdf;
//     Foo foo2;
// } bar;

void main() {
    vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );
    vec2 pos = positions[gl_VertexID];
    uv = pos * 0.5 + 0.5;
    gl_Position = vec4(pos, 0.0, 1.0);
}