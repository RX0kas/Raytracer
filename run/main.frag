#version 330 core

out vec4 FragColor;

#include "const.glsl"

uniform float focalLength;
uniform vec2 resolution;
uniform vec3 camDir;
uniform vec3 camUp;
uniform vec3 camPos;
uniform int time;
uniform int maxBounces;
uniform int lastMove;
uniform int rayPerPixel;
uniform sampler2D oldFrame;

struct Material {
    vec4 color;
    vec4 emissionColor;
    float emissionStrength;
};

Material getDefaultMaterial() {
    Material m;
    m.color = vec4(1,0,1,1);
    m.emissionColor = vec4(1,0,1,1);
    m.emissionStrength = 0.0;
    return m;
}

struct Sphere {
    vec3 center;
    float radius;
    Material material;
};

const int sphereCount = 4;
Sphere spheres[sphereCount];

Sphere getDefaultSphere() {
    return Sphere(vec3(0),0,getDefaultMaterial());
}

const vec4 RED = vec4(1,0,0,1);
const vec4 BLACK = vec4(0,0,0,1);

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct HitInfo {
    bool didHit;
    float dst;
    vec3 hitPoint;
    vec3 normal;
    Sphere sphere;
};

HitInfo getDefaultHitInfo() {
    return HitInfo(false,1e9,vec3(0),vec3(0),getDefaultSphere());
}

//////////////////////////////
//          Random          //
//////////////////////////////
uint wang_hash(uint x) {
    x = (x ^ 61u) ^ (x >> 16);
    x *= 9u;
    x = x ^ (x >> 4);
    x *= 0x27d4eb2du;
    x = x ^ (x >> 15);
    return x;
}


uint generateSeed(int i) {
    uint seed = uint(gl_FragCoord.x) * 1973u + uint(gl_FragCoord.y) * 9277u + uint(time + 1) * 26699u + uint(i) * 911247u + uint(lastMove) * 54782u;
    seed = wang_hash(seed);
    return seed;
}
float randf(inout uint state) {
    // xorshift32 variant
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    // normalize to [0,1)
    return float(state) / 4294967295.0;
}

// --- Cosine-weighted hemisphere sampling ---
vec3 cosineSampleHemisphere(float u1, float u2) {
    float r = sqrt(u1);
    float theta = 2.0 * 3.14159265359 * u2;
    float x = r * cos(theta);
    float y = r * sin(theta);
    float z = sqrt(max(0.0, 1.0 - u1));
    return vec3(x, y, z); // local-space (z = up)
}

vec3 makeTangentBasis(vec3 n) {
    // return tangent vectorX; compute Y via cross in caller
    vec3 up = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangentX = normalize(cross(up, n));
    return tangentX;
}

vec3 sampleHemisphereCosine(vec3 normal, inout uint rngState) {
    float u1 = randf(rngState);
    float u2 = randf(rngState);
    vec3 samplecos = cosineSampleHemisphere(u1, u2); // local coords
    vec3 tangentX = makeTangentBasis(normal);
    vec3 tangentY = cross(normal, tangentX);
    // transform to world
    return normalize(samplecos.x * tangentX + samplecos.y * tangentY + samplecos.z * normal);
}

HitInfo RaySphere(Ray ray)
{
    HitInfo closestHit = getDefaultHitInfo();
    const float MIN_DIST = 0.001; // Avoid self-intersection
    const float MAX_DIST = 1000.0; // Prevent infinite rays

    for(int i = 0; i < sphereCount; ++i) {
        Sphere s = spheres[i];
        HitInfo hit = getDefaultHitInfo();

        vec3 offsetRayOrigin = ray.origin - s.center;
        float a = dot(ray.direction, ray.direction);
        float b = 2.0 * dot(offsetRayOrigin, ray.direction);
        float c = dot(offsetRayOrigin, offsetRayOrigin) - s.radius * s.radius;

        float discriminant = b * b - 4.0 * a * c;

        if (discriminant >= 0.0) {
            float sqrtDisc = sqrt(discriminant);
            float dst1 = (-b - sqrtDisc) / (2.0 * a);
            float dst2 = (-b + sqrtDisc) / (2.0 * a);

            // Choose the closest valid intersection
            float dst = min(dst1, dst2);
            if (dst < MIN_DIST) {
                dst = max(dst1, dst2); // Try the other solution
            }

            if (dst >= MIN_DIST && dst <= MAX_DIST && dst < closestHit.dst) {
                hit.didHit = true;
                hit.dst = dst;
                hit.hitPoint = ray.origin + ray.direction * dst;
                hit.normal = normalize(hit.hitPoint - s.center);
                hit.sphere = s;
                closestHit = hit;
            }
        }
    }
    return closestHit;
}

vec3 getRayDir(vec3 camDir, vec3 camUp, vec2 texCoord) {
    vec3 camSide = normalize(cross(camDir, camUp));
    vec2 p = 2.0 * texCoord - 1.0;
    p.x *= resolution.x / resolution.y;
    return normalize(p.x * camSide + p.y * camUp + focalLength * camDir);
}

vec3 Trace(Ray ray,int i) {
    vec3 radiance = vec3(0.0);
    vec3 throughput = vec3(1.0);

    uint seed = generateSeed(i);

    for (int bounce = 0; bounce < maxBounces; ++bounce) {
        HitInfo hit = RaySphere(ray);// should return closest hit with sphere in HitInfo
        if (!hit.didHit) {
            // environment: return black or env color
            break;
        }

        // offset to avoid self-intersection
        ray.origin = hit.hitPoint + hit.normal * 1e-4;

        // if emitter -> accumulate emission * throughput
        float emit = hit.sphere.material.emissionStrength;
        if (emit > 0.0) {
            radiance += throughput * hit.sphere.material.emissionColor.rgb * emit;
            // If you want direct-only from emitters, you could 'break' here,
            // but for path tracing, we usually continue (or break depending)
        }

        // sample new direction cosine-weighted around normal
        vec3 newDir = sampleHemisphereCosine(hit.normal, seed);

        // update throughput: for lambertian BRDF = albedo/pi and pdf = cos(theta)/pi
        // BRDF/pdf => albedo (cancels pi), so we can simply multiply by albedo
        throughput *= hit.sphere.material.color.rgb;

        // move ray direction
        ray.direction = newDir;

        // Russian roulette after few bounces
        if (bounce > maxBounces/4) {
            float p = max(max(throughput.r, throughput.g), throughput.b);
            float r = randf(seed);
            if (r > p) break;
            throughput /= max(p, 1e-6);
        }
    }

    return radiance;
}

vec4 combine_with_old_frame(vec4 newPixel, vec2 texCoord) {
    vec4 oldPixel = texture(oldFrame, texCoord);

    float weight = 1.0 / float(lastMove + 1);
    return mix(oldPixel, newPixel, weight);
}

void main()
{
    vec2 texCoord = gl_FragCoord.xy / resolution;
    Ray r = Ray(camPos, getRayDir(camDir, camUp, texCoord));

    Material m0;
    m0.color = vec4(1,0,0,1);
    m0.emissionColor = vec4(1,0,0,1);
    m0.emissionStrength = 0.0;
    Material m1;
    m1.color = vec4(0,0,0,1);
    m1.emissionColor = vec4(1,1,1,1);
    m1.emissionStrength = 1.0;
    Material m2;
    m2.color = vec4(0,0,1,1);
    m2.emissionColor = vec4(0,0,1,1);
    m2.emissionStrength = 0.0;

    Material floorM;
    floorM.color = vec4(1,1,1,1);
    floorM.emissionColor = vec4(1,1,1,1);
    floorM.emissionStrength = 0.1;

    spheres[0] = Sphere(vec3(-3.5,-1.5,10), 1.0, m0);
    spheres[1] = Sphere(vec3(0,0,10), 2.0, m1);
    spheres[2] = Sphere(vec3(3.5,-1.5,10), 1.0, m2);
    spheres[3] = Sphere(vec3(0,-102,10), 100.0, floorM);

    vec3 allColor[100];
    float xAvg = 0;
    float yAvg = 0;
    float zAvg = 0;
    for (int i=0;i<rayPerPixel;i++) {
        vec3 t = Trace(r,i);
        xAvg += t.x;
        yAvg += t.y;
        zAvg += t.z;
    }
    vec4 newColor = vec4(xAvg/rayPerPixel,yAvg/rayPerPixel,zAvg/rayPerPixel, 1.0);

    if (lastMove<=1)
        FragColor = newColor;
    else
        FragColor = combine_with_old_frame(newColor,texCoord);
}