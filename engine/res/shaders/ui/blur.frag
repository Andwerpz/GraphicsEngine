#version 460 core
layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 color;

layout (binding = 0) uniform Settings {
    int is_vertical;
    vec2 pos;
    vec2 dim;
    int sigma;
} settings;

layout (binding = 1) uniform sampler2D albedo;

// int SIGMA = 16;
// int RADIUS = SIGMA * 3;
void main() {
    bool outside_rect =
        uv.x < settings.pos.x ||
        uv.y < settings.pos.y ||
        uv.x > settings.pos.x + settings.dim.x ||
        uv.y > settings.pos.y + settings.dim.y;
    if(outside_rect) {
        discard;
    }

    int SIGMA = settings.sigma;
    int RADIUS = SIGMA * 3;

    vec2 texel = 1.0 / vec2(textureSize(albedo, 0));
    vec2 step = (settings.is_vertical != 0) ? vec2(0.0, texel.y) : vec2(texel.x, 0.0);

    vec4 sum = vec4(0.0);
    float weight_sum = 0.0;
    for(int i = -RADIUS; i <= RADIUS; ++i) {
        float w = exp(-float(i * i) / (2.0 * float(SIGMA * SIGMA)));
        sum += texture(albedo, uv + step * float(i)) * w;
        weight_sum += w;
    }
    color = sum / weight_sum;

    if(settings.is_vertical != 0) color.a = 1.0;
}