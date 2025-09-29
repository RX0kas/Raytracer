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
uniform sampler2D accumTex;

uint rngState;

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
uint hash(uint x) {
    x = (x ^ (x >> 16)) * 0x7feb352dU;
    x = (x ^ (x >> 15)) * 0x846ca68bU;
    return x ^ (x >> 16);
}

float random(inout uint state) {
    state = hash(state);
    return float(state) / 4294967295.0;
}

vec3 randomDirection(inout uint state) {
    vec3 v;
    float lenSq;
    do {
        v = vec3(random(state) * 2.0 - 1.0,
        random(state) * 2.0 - 1.0,
        random(state) * 2.0 - 1.0);
        lenSq = dot(v, v);
    } while(lenSq > 1.0 || lenSq == 0.0);

    return v * inversesqrt(lenSq);
}

// hemisphere sampling with bias to avoid self-intersection
vec3 randomHemisphereDirection(vec3 normal, inout uint state) {
    vec3 dir = randomDirection(state);

    // Make sure we're in the hemisphere
    if (dot(dir, normal) < 0.0) {
        dir = -dir;
    }

    return dir;
}

// cosine-weighted sampling for more realistic results
vec3 randomCosineHemisphereDirection(vec3 normal, inout uint state) {
    float r = sqrt(random(state));
    float theta = TWO_PI * random(state);

    vec3 tangent = normalize(cross(normal,
    abs(normal.x) > 0.1 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0)));
    vec3 bitangent = cross(normal, tangent);

    float x = r * cos(theta);
    float y = r * sin(theta);
    float z = sqrt(max(0.0, 1.0 - x*x - y*y)); // Avoid negative sqrt

    return tangent * x + bitangent * y + normal * z;
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

vec3 Trace(Ray ray) {
    vec3 radiance = vec3(0.0);
    vec3 throughput = vec3(1.0);

    for (int bounce = 0; bounce < maxBounces; ++bounce) {
        HitInfo hit = RaySphere(ray); // should return closest hit with sphere in HitInfo
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
        vec3 newDir = sampleHemisphereCosine(hit.normal, rngState);

        // update throughput: for lambertian BRDF = albedo/pi and pdf = cos(theta)/pi
        // BRDF/pdf => albedo (cancels pi), so we can simply multiply by albedo
        throughput *= hit.sphere.material.color.rgb;

        // move ray direction
        ray.direction = newDir;

        // Russian roulette after few bounces
        if (bounce > maxBounces/4) {
            float p = max(max(throughput.r, throughput.g), throughput.b);
            float r = randf(rngState);
            if (r > p) break;
            throughput /= max(p, 1e-6);
        }
    }

    return radiance;
}

void initRNG(vec2 pixelCoord) {
    uint seed = uint(gl_FragCoord.x) * 1973u + uint(gl_FragCoord.y) * 9277u + uint(time + 1) * 26699u;
    rngState = wang_hash(seed);
}

void main()
{
    initRNG(gl_FragCoord.xy);

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

    spheres[0] = Sphere(vec3(-3.5,0,10), 1.0, m0);
    spheres[1] = Sphere(vec3(0,0,10), 2.0, m1);
    spheres[2] = Sphere(vec3(3.5,0,10), 1.0, m2);
    spheres[3] = Sphere(vec3(0,-102,10), 100.0, floorM);

    const int rayPerPixel = 50;

    vec3 allColor[rayPerPixel];
    for (int i=0;i<rayPerPixel;i++) {
        allColor[i] = Trace(r);
    }

    float xAvg = allColor[0].x;
    float yAvg = allColor[0].y;
    float zAvg = allColor[0].z;

    if (rayPerPixel>1) {
        for (int i=1;i<rayPerPixel;i++) {
            xAvg += allColor[i].x;
            yAvg += allColor[i].y;
            zAvg += allColor[i].z;
        }
    }

    FragColor = vec4(xAvg/rayPerPixel,yAvg/rayPerPixel,zAvg/rayPerPixel, 1.0);
}