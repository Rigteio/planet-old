#version 430
in vec4 Pos;
in vec4 Norm;
out vec4 color;

// I probably tried to use texture as a storage for perlin vectors,
// but it turned out to be very slow
uniform sampler2D perlin;

uniform mat4 rot;
uniform int seed;
uniform int time;

#define lerp( t , a , b ) ( a + t * ( b - a ) )
//#define weight( a ) ( 6 * a * a * a * a * a - 15 * a * a * a * a + 10 * a * a * a )
#define weight( a ) ( 3 * a * a - 2 * a * a * a)

const vec3 nodev[16] = vec3[16](
  vec3(1,1,0), vec3(-1,1,0), vec3(1,-1,0), vec3(-1,-1,0),
  vec3(1,0,1), vec3(-1,0,1), vec3(1,0,-1), vec3(-1,0,-1),
  vec3(0,1,1), vec3(0,-1,1), vec3(0,1,-1), vec3(0,-1,-1),
  vec3(1,1,0), vec3(-1,1,0), vec3(0,-1,1), vec3(0,-1,-1)
);

float c_dot(vec3 a, int i) {
//It's not optimized as a jump table :(
    switch (i) {
	case 0: return a.x+a.y;
	case 1: return a.y-a.x;
	case 2: return a.x-a.y;
	case 3: return -a.x-a.y;
	case 4: return a.z+a.x;
	case 5: return a.z-a.x;
	case 6: return a.x-a.z;
	case 7: return -a.x-a.z;
	case 8: return a.y+a.z;
	case 9: return a.z-a.y;
	case 10: return a.y-a.z;
	case 11: return -a.y-a.z;
	case 12: return a.x+a.y;
	case 13: return a.y-a.x;
	case 14: return a.z-a.y;
	case 15: return -a.y-a.z;
    }
    return 0;
}

//layout(std430, binding = 3) buffer grads {
//    int indices[];
//};

float _noise(vec3 c, int size) {
    vec3 cr = c*size;
    vec3 cw = floor(cr) + 64;
    vec3 cf = fract(cr);

    float kernel = cw.x*65537 + cw.y*257 + cw.z*1023 + seed;

    float dot111 = dot( nodev[int(sin(kernel)         * 1000000) & 0xf], cf );
    float dot121 = dot( nodev[int(sin(kernel + 257)   * 1000000) & 0xf],cf - vec3(0,1,0));
    float dot211 = dot( nodev[int(sin(kernel + 65537) * 1000000) & 0xf],cf - vec3(1,0,0));
    float dot221 = dot( nodev[int(sin(kernel + 65794) * 1000000) & 0xf],cf - vec3(1,1,0));

    float dot112 = dot( nodev[int(sin(kernel + 1023)  * 1000000) & 0xf],cf - vec3(0,0,1));
    float dot122 = dot( nodev[int(sin(kernel + 1280)  * 1000000) & 0xf],cf - vec3(0,1,1));
    float dot212 = dot( nodev[int(sin(kernel + 66560) * 1000000) & 0xf],cf - vec3(1,0,1));
    float dot222 = dot( nodev[int(sin(kernel + 66817) * 1000000) & 0xf],cf - vec3(1,1,1));

    /*float dot111 = sin(sin(kernel + 0) * 1000000);
    float dot121 = sin(sin(kernel + 257) * 1000000);
    float dot211 = sin(sin(kernel + 65537) *1000000);
    float dot221 = sin(sin(kernel + 65794) *1000000);

    float dot112 = sin(sin(kernel + 1023) * 1000000);
    float dot122 = sin(sin(kernel + 1280) * 1000000);
    float dot212 = sin(sin(kernel + 66560) * 1000000);
    float dot222 = sin(sin(kernel + 66817) * 1000000);*/

    float wx = weight(cf.x);
    float wy = weight(cf.y);
    float wz = weight(cf.z);
    float ml = lerp(wy, lerp(wx, dot111, dot211), lerp(wx, dot121, dot221));
    float mh = lerp(wy, lerp(wx, dot112, dot212), lerp(wx, dot122, dot222));
    return lerp(wz, ml, mh);
}

float noise(vec3 co, int level) {
    vec3 c = (co + vec3(1,1,1))/2;
    float a = 0;
    int cl = 1;
    //float t = time/50000.;
    for (int i = 0; i<level; i++) {
	    += _noise(c, 4*cl)/cl;
        cl *= 2;
    }
    return (a+1)/2;
}

//noise texture here changes with time
float noise_t(vec3 co, int level) {
    vec3 c = (co + vec3(1,1,1))/2;
    float a = 0;
    int cl = 1;
    float t = time/50000.;
    for (int i = 0; i<level; i++) {
        a += _noise(mat3(cos(t), -sin(t), 0, sin(t), cos(t), 0, 0, 0, 1)*c, 4*cl)/cl;
        cl *= 2;
    }
    return (a+1)/2;
}

float turbulence(vec3 co, int level) {
    vec3 c = (co + vec3(1,1,1))/2;
    float a = 0;
    int cl = 1;
    float t = time/50000.;
    for (int i = 0; i<level; i++) {
        a += _noise(c, 4*cl)/cl;
        cl *= 2;
    }
    a -= .2;
    return a < 0 ? -sqrt(-a) : sqrt(a);
}

float turbulence_t(vec3 co, int level) {
    vec3 c = (co + vec3(1,1,1))/2;
    float a = 0;
    int cl = 1;
    float t = time/50000.;
    for (int i = 0; i<level; i++) {
        a += _noise(mat3(cos(t), -sin(t), 0, sin(t), cos(t), 0, 0, 0, 1)*c, 4*cl)/cl;
        cl *= 2;
    }
    return abs(a);
}

void main() {
    float b;
    //float delta = .0002;
    float temp = noise(Pos.xyz, 6);
    float shade;
    if (temp > .6) {
        float delta = .02;
        float point = noise(Pos.xyz, 7);
        float dx = noise(Pos.xyz - vec3(delta, 0, 0), 7) - point;
        float dy = noise(Pos.xyz - vec3(0, delta, 0), 7) - point;
        float dz = noise(Pos.xyz - vec3(0, 0, delta), 7) - point;
        shade = max(0, dot((rot*normalize(Norm + vec4(4*dx,4*dy,4*dz,0))).xyz, normalize(vec3(1,-1,.3))));
        color = vec4(temp*.1 + temp*shade, temp*(shade + .02), temp*shade,  0);
    }
    else {
        float delta = .0002;
        float point = turbulence(Pos.xyz, 5);
        float dx = turbulence(Pos.xyz - vec3(delta, 0, 0), 5) - point;  
        float dy = turbulence(Pos.xyz - vec3(0, delta, 0), 5) - point;  
        float dz = turbulence(Pos.xyz - vec3(0, 0, delta), 5) - point;
        shade = max(0, dot((rot*normalize(Norm + vec4(40*dx,40*dy,40*dz,0))).xyz, normalize(vec3(1,-1,.3))));
        color = vec4((.1 + .1*temp)*shade, (.1 + .3*temp)*shade, (.2 + .8*temp)*shade, 0);
    }
    float clouds = max(0, turbulence_t((Pos).xyz + vec3(1,1,1), 8) - .3)/.7;
    color += vec4(clouds, clouds, clouds, 0) * max(0, dot(rot*Norm, normalize(vec4(1,-1,.3,0))));
    //color =  vec4(shade, shade, shade, 0);
    //color = vec4(temp, temp, temp, 0);
}
