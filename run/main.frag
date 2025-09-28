#version 330 core

out vec4 FragColor;

uniform float focal_length;
uniform vec2 resolution;
uniform vec3 camdir;
uniform vec3 camup;
uniform vec3 campos;

//const int NBR_SPHERE = 3;

struct Material {
    vec4 color;
};
Material getDefaultMaterial() {
    return Material(vec4(1, 0, 1,1));
}

struct Sphere {
    vec3 center;
    float radius;
    Material material;
};

Sphere getDefaultSphere() {
    return Sphere(vec3(0),0,getDefaultMaterial());
}

/*layout (std140) uniform SphereBlock {
    Sphere spheres[NBR_SPHERE];
};*/

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

HitInfo RaySphere(Ray ray, Sphere s)
{
    HitInfo hitInfo = getDefaultHitInfo();

    vec3 offsetRayOrigin = ray.origin - s.center;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(offsetRayOrigin, ray.direction);
    float c = dot(offsetRayOrigin, offsetRayOrigin) - s.radius * s.radius;

    float discriminant = b * b - 4.0 * a * c;

    if (discriminant >= 0.0) {
        float dst = (-b - sqrt(discriminant)) / (2.0 * a);
        if (dst >= 0.0) {
            hitInfo.didHit = true;
            hitInfo.dst = dst;
            hitInfo.hitPoint = ray.origin + ray.direction * dst;
            hitInfo.normal = normalize(hitInfo.hitPoint - s.center);
            hitInfo.sphere = s;   // ✅ copie la sphère pour la couleur
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

    const int sphereCount = 3;
    Sphere spheres[sphereCount];
    spheres[0] = Sphere(vec3(-5,0,10), 1.0, Material(vec4(1,0,0,1)));   // rouge
    spheres[1] = Sphere(vec3(0,0,10), 2.0, Material(vec4(0,1,0,1)));   // vert
    spheres[2] = Sphere(vec3(5,0,10), 1.0, Material(vec4(0,0,1,1)));   // bleu

    HitInfo closestHit = getDefaultHitInfo();

    // Search for the nearest sphere
    for(int i=0;i<sphereCount;++i) {
        Sphere s = spheres[i];
        HitInfo hit = RaySphere(r, s);
        if (hit.didHit && hit.dst<closestHit.dst) {
            closestHit = hit;
        }
    }

    // Change color based on the sphere found
    if (closestHit.didHit) {
        FragColor = closestHit.sphere.material.color;
    } else {
        FragColor = BLACK;
    }
}


// TODO Later: transparency, basic light