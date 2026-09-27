#version 460 core
layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 color;

layout (binding = 2) uniform Color {
    vec3 col;
} ucolor;

float test_func(float x1, int x2) {
    return 1.0;
}

void main() {
    // color = vec4(uv, 0.0, test_func(1.0, 0));
    color = vec4(ucolor.col, 1.0);
}