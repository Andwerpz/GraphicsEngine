#version 460 core

layout (location = 0) in vec3 frag_normal;
layout (location = 1) in vec2 frag_uv;

layout (location = 0) out vec4 color;

layout (binding = 1) uniform sampler2D albedo;

void main() {
    vec3 light_dir = normalize(vec3(1.0, 2.0, 3.0));

    float diffuse = dot(normalize(frag_normal), light_dir) * 0.5 + 0.5;
    vec3 base_color = texture(albedo, frag_uv).xyz;
    color = vec4(base_color * diffuse, 1.0);

    float edge_dist = min(min(abs(frag_uv.x), abs(frag_uv.x - 1.0)), min(abs(frag_uv.y), abs(frag_uv.y - 1.0)));
    if(edge_dist < 0.02) color = vec4(1.0);

    // color = vec4(frag_normal * 0.5 + 0.5, 1.0);
    // color = vec4(base_color, 1.0);
}