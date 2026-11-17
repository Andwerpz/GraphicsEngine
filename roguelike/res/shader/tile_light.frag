#version 460 core

layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 color;

layout (binding = 0) uniform sampler2D albedo_tex;
layout (binding = 1) uniform sampler2D normal_tex;
layout (binding = 2) uniform sampler2D position_tex;

const float INF = 1e15;

struct Ray {
    vec3 origin;
    vec3 dir;
};

layout (binding = 3) uniform AmbientLight {
    vec3 ambient_color;
    float ambient_intensity;
} ambient_light;

struct PointLight {
    vec3 position;
    float range;
    vec3 color;
    float intensity;
};
layout (binding = 6) uniform PointLightBuffer {
    int count;
    PointLight lights[16];
} point_lights;

layout (binding = 4) uniform DirectionalLight {
    vec3 direct_dir;
    vec3 direct_color;
    float direct_intensity;
} directional_light;

// represents the volume [bmin, bmax)
struct Box {
    vec3 bmin;
    vec3 bmax;
};
layout (binding = 5) uniform BoxBuffer {
    int count;
    Box boxes[256];
} box_buffer;

// translates a normal from normal_tex into world space normal
vec3 world_normal(vec3 encoded) {
    vec3 n = encoded * 2.0 - 1.0;
    return normalize(vec3(n.x, n.z, n.y));
}

float safe_inv(float d) {
    if(abs(d) > 1e-8) return 1.0 / d;
    return d >= 0.0 ? INF : -INF;
}

// a point is inside a box if all of its components are >= bmin and < bmax
// for each component, we can figure out at what distances along the ray it will be inside
// as long as there is a point along the ray such that it is inside all components, then the ray intersects the box
float ray_box_dist(Ray ray, Box box) {
    // t0 = distance along ray for each component to get to bmin
    // t1 = distance along ray for each component to get to bmax
    vec3 inv = vec3(safe_inv(ray.dir.x), safe_inv(ray.dir.y), safe_inv(ray.dir.z));
    vec3 t0 = (box.bmin - ray.origin) * inv;
    vec3 t1 = (box.bmax - ray.origin) * inv;

    // tsm = distance along ray for each component to get to near plane
    // tbg = distance along ray for each component to get to far plane
    vec3 tsm = min(t0, t1);
    vec3 tbg = max(t0, t1);

    float tnear = max(max(tsm.x, tsm.y), tsm.z);
    float tfar = min(min(tbg.x, tbg.y), tbg.z);
    if(tnear > 1e-3 && tnear <= tfar) return tnear;
    return INF;
}

// traces the ray against the map geometry and returns the distance to the closest hit
// returns infinity if there is no hit
float nearest_hit(Ray ray) {
    float nearest = INF;
    int count = box_buffer.count;
    for(int i = 0; i < count; i++) {
        Box box = box_buffer.boxes[i];
        float t = ray_box_dist(ray, box);
        if(t < nearest) nearest = t;
    }
    return nearest;
}

// returns true if the nearest hit in the direction is closer than dist
bool in_shadow(Ray ray, float dist) {
    return nearest_hit(ray) < dist;
}

// in_shadow but for directional lights
bool in_shadow(Ray ray) {
    return in_shadow(ray, INF / 2.0);
}

// generate directions using cosine-weighted golden angle spiral
vec3 hemisphere_dir(vec3 n, int i, int count) {
    float phi = 2.39996323 * float(i);
    float t = (float(i) + 0.5) / float(count);
    float cos_theta = sqrt(1.0 - t);
    float sin_theta = sqrt(t);
    vec3 up = abs(n.y) < 0.9 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(n, up));
    vec3 bitangent = cross(n, tangent);
    return tangent * (cos(phi) * sin_theta) + bitangent * (sin(phi) * sin_theta) + n * cos_theta;
}

// inverse-square falloff
// should return 1 when dist2 == 0 and 0 when dist2 == range^2
float quadratic_falloff(float dist2, float range) {
    float r2 = range * range;
    if(r2 <= 1e-6) return 0.0;
    float window = clamp(1.0 - dist2 / r2, 0.0, 1.0);
    window *= window;
    return window / (dist2 + 1.0);
}

// Narkowicz fit of the ACES filmic curve
// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 tonemap_aces(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// 1.0 indicates no ambient occlusion, 0.0 indicates full ambient occlusion
float ambient_occlusion(vec3 P, vec3 N) {
    const int RAYS = 32;
    const float RADIUS = 4.0;
    vec3 origin = P + N * 0.04;
    float occl = 0.0;
    for(int i = 0; i < RAYS; i++) {
        vec3 dir = hemisphere_dir(N, i, RAYS);
        float t = min(nearest_hit(Ray(origin, dir)), RADIUS);
        float w = clamp(1.0 - t / RADIUS, 0.0, 1.0);
        occl += w * w;
    }
    return 1.0 - occl / float(RAYS);
}

void main() {
    vec4 alb = texture(albedo_tex, uv);
    vec4 pos = texture(position_tex, uv);
    if(alb.a <= 0.0 || pos.a != 1.0) {
        color = vec4(0.0);
        return;
    }
    
    vec3 albedo = alb.rgb;
    vec3 lit = vec3(0.0);

    {
        vec3 N = world_normal(texture(normal_tex, uv).rgb);
        vec3 P = pos.xyz + N * 0.02;    // offset position a little outside of the wall
        vec3 V = normalize(vec3(0.0, 1.0, 1.0));

        // ambient lighting
        {
            vec3 ambient = ambient_light.ambient_color * ambient_light.ambient_intensity;
            float ao = ambient_occlusion(P, N);
            lit += albedo * ambient * ao;
        }

        // direct lighting
        {
            vec3 Ld = normalize(-directional_light.direct_dir);
            float direct_ndotl = max(dot(N, Ld), 0.0);
            float direct_shadow = 1.0;
            if(direct_ndotl > 0.0 && in_shadow(Ray(P, Ld))) direct_shadow = 0.0;
            vec3 direct = directional_light.direct_color * directional_light.direct_intensity * direct_shadow;
            lit += albedo * direct * direct_ndotl;
        }

        // point lighting
        for(int i = 0; i < point_lights.count; i++) {
            PointLight light = point_lights.lights[i];
            vec3 to_light = light.position - P;
            float dist2 = dot(to_light, to_light);
            float falloff = quadratic_falloff(dist2, light.range);
            if(falloff <= 0.0) continue;
            float dist = sqrt(dist2);
            vec3 Ld = to_light / max(dist, 1e-4);
            float ndotl = max(dot(N, Ld), 0.0);
            float shadow = 1.0;
            if(ndotl > 0.0 && in_shadow(Ray(P, Ld), dist)) shadow = 0.0;
            vec3 point = light.color * light.intensity * ndotl * shadow * falloff;
            lit += albedo * point;
        }

        // bias pixels with greater Y to be brighter
        lit *= 1.0 + max(P.y, 0.0) * 0.5;
    }

    vec3 mapped = tonemap_aces(max(lit, vec3(0.0)));
    color = vec4(mapped * alb.a, alb.a);
}
