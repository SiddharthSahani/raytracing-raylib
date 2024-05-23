
#version 330 core


uniform vec2 uImageSize;
uniform mat4 uInvViewMat;
uniform mat4 uInvProjMat;
uniform vec3 uCameraPos;


struct Ray {
    vec3 origin;
    vec3 direction;
};


struct Sphere {
    vec3 position;
    float radius;
};


Ray genRay() {
    vec2 coord = gl_FragCoord.xy / uImageSize * 2.0 - 1.0;
    vec4 target = uInvProjMat * vec4(coord, 1.0, 1.0);
    vec4 rayDirection = uInvViewMat * vec4(normalize(target.xyz / target.w), 0.0);

    return Ray(uCameraPos, rayDirection.xyz);
}


bool hitSphere(Sphere sphere, Ray ray) {
    vec3 oc = ray.origin - sphere.position;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float d = b * b - 4 * a * c;
    return d > 0.0;
}


void main() {
    Ray ray = genRay();
    
    Sphere sphere = Sphere(vec3(0.0, 0.0, 0.0), 1.0);

    if (hitSphere(sphere, ray)) {
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    } else {
        gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
    }
}
