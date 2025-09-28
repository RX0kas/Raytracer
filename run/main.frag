#version 330 core

out vec4 FragColor;

uniform float focal_length;
uniform vec2 resolution;
uniform vec3 camdir;
uniform vec3 camup;
uniform vec3 campos;

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
};

HitInfo RaySphere(Ray ray, vec3 sphereCentre, float sphereRadius)
{
    HitInfo hitInfo = HitInfo(false,-1,vec3(0),vec3(0));

    vec3 offsetRayOrigin = ray.origin - sphereCentre;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(offsetRayOrigin, ray.direction);
    float c = dot(offsetRayOrigin, offsetRayOrigin) - sphereRadius * sphereRadius;

    float discriminant = b * b - 4.0 * a * c;

    if (discriminant >= 0.0) {
        float dst = (-b - sqrt(discriminant)) / (2.0 * a);
        if (dst >= 0.0) {
            hitInfo.didHit = true;
            hitInfo.dst = dst;
            hitInfo.hitPoint = ray.origin + ray.direction * dst;
            hitInfo.normal = normalize(hitInfo.hitPoint - sphereCentre);
        }
    }
    return hitInfo;
}

vec3 getRayDir(vec3 camDir, vec3 camUp, vec2 texCoord) {
    vec3 camSide = normalize(cross(camDir, camUp));
    vec2 p = 2.0 * texCoord - 1.0;
    p.x *= resolution.x / resolution.y;
    return normalize(p.x * camSide + p.y * camUp + focal_length * camDir);
}

void main()
{
    vec2 texCoord = gl_FragCoord.xy / resolution;
    Ray r = Ray(campos, getRayDir(camdir, camup, texCoord));

    HitInfo hit = RaySphere(r, vec3(0,0,10), 2.0);

    if (hit.didHit) {
        FragColor = vec4(hit.normal+1,1);
    } else {
        FragColor = BLACK;
    }
}