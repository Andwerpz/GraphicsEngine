#version 460 core
layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 color;
layout (binding = 1) uniform sampler2D textures;

void main() {
    // vec3 base_color = texture(textures, uv).xyz;
    // color = vec4(base_color, 1);

    color = texture(textures, uv);

    // color = vec4(uv, 0, 1);
}