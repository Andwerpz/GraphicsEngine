#version 460 core

layout (location = 0) in vec3 frag_normal;

layout (location = 0) out vec4 color;

void main() {
    vec3 light_dir = normalize(vec3(1.0, 2.0, 3.0));

    float diffuse = dot(normalize(frag_normal), light_dir) * 0.5 + 0.5;
    vec3 base_color = vec3(1.0, 0.7, 0.2);
    color = vec4(base_color * diffuse, 1.0);

    // color = vec4(frag_normal * 0.5 + 0.5, 1.0);
    // color = vec4(base_color, 1.0);
}