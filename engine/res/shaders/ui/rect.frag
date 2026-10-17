#version 460 core

layout (location = 0) in vec2 frag_pos;
layout (location = 1) in vec2 frag_uv;
layout (location = 2) in vec4 frag_color;   // premultiplied
layout (location = 3) in vec4 frag_info;    // {read from texture, unused, unused, unused}

layout (location = 0) out vec4 color;

layout (binding = 1) uniform sampler2D albedo;

void main() {
    if(frag_info.x != 0.0) {
        // sample texture
        color = texture(albedo, frag_uv) * frag_color;
    }
    else {
        // single color
        color = frag_color;
    }
}