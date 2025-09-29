float random(int seed) {
    seed = seed * 747796405 + 2891336453;
    int result = ((seed >> ((seed >> 28) + 4)) ^ seed) * 277803737;
    result = (result >> 22) ^ result;
    return result / 4294967295.0;
}

float randomValueNormalDistribution(int seed) {
    float theta = 2 * 3.1415926535897 * random(seed);
    float rho = sqrt(-2 * log(random(seed)));
    return rho * cos(theta);
}

vec3 randomDirection(int seed) {
    float x = randomValueNormalDistribution(seed);
    float y = randomValueNormalDistribution(seed);
    float z = randomValueNormalDistribution(seed);
    return normalize(vec3(x,y,z));
}

// Random direction in the hemisphere oriented around the given normal vector
vec3 randomHemisphereDirection(vec3 normal,int seed) {
    vec3 dir = randomDirection(seed);
    return dir * sign(dot(normal,dir));
}