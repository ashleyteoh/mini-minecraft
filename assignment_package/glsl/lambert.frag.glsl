#version 150
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

uniform vec4 u_Color; // The color with which to render this instance of geometry.

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Pos;
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec4 fs_Col;

out vec4 out_Col; // This is the final output color that you will see on your
// screen for the pixel that is currently being processed.

// Added for texturing
in vec4 fs_UVs;
uniform sampler2D textureSampler;
uniform int u_Time;

// added for water animation
uniform vec3 u_PlayerPos;

// fbm functions taken from terrain visualiser on the course syllabus
vec2 smoothF(vec2 uv)
{
    return uv*uv*(3.-2.*uv);
}

float noise(in vec2 uv)
{
    const float k = 257.;
    vec4 l  = vec4(floor(uv),fract(uv));
    float u = l.x + l.y * k;
    vec4 v  = vec4(u, u+1.,u+k, u+k+1.);
    v       = fract(fract(1.23456789*v)*v/.987654321);
    l.zw    = smoothF(l.zw);
    l.x     = mix(v.x, v.y, l.z);
    l.y     = mix(v.z, v.w, l.z);
    return    mix(l.x, l.y, l.w);
}

float fbm(const in vec2 uv)
{
    float a = 0.5;
    float f = 5.0;
    float n = 0.;
    int it = 8;
    for(int i = 0; i < 32; i++)
    {
        if(i<it)
        {
            n += noise(uv*f)*a;
            a *= .5;
            f *= 2.;
        }
    }
    return n;
}


//float random1( vec2 p ) {
//    return fract(sin((dot(p, vec3(127.1,
//                                  311.7)))) *
//                 43758.5453);
//}


//vec2 noise2D( vec2 p ) {
//    return fract(sin(vec2(p.x * 127.1,
//                          p.y * 269.5) *
//                 43758.5453));
//}




//float interpNoise2D(float x, float y) {
////    int intX = int(floor(x));
////    float fractX = fract(x);
////    int intY = int(floor(y));
////    float fractY = fract(y);

////    float v1 = noise2D(vec2(intX, intY));
////    float v2 = noise2D(vec2(intX + 1, intY));
////    float v3 = noise2D(vec2(intX, intY + 1));
////    float v4 = noise2D(vec2(intX + 1, intY + 1));

////    float i1 = mix(v1, v2, fractX);
////    float i2 = mix(v3, v4, fractX);
////    return mix(i1, i2, fractY);
//    return 1.f;
//}



//float fbm(float x, float y) {
//    float total = 0;
//    float persistence = 0.5f;
//    int octaves = 8;
//    float freq = 2.f;
//    float amp = 0.5f;
//    for(int i = 1; i <= octaves; i++) {
//        total += interpNoise2D(x * freq,
//                               y * freq) * amp;

//        freq *= 2.f;
//        amp *= persistence;
//    }
//    return total;
//}



void main()
{
    // Material base color (before shading)
    //vec4 diffuseColor = fs_Col;

    // calculate animation
    vec4 diffuseColor;

    if (fs_UVs.z == 1) {
        //float t = floor(u_Time * 25.f) / 25.f;
        float t = u_Time / 1000.f;
        float y_Disp = -mod(t, 0.0625);
        //y_Disp = floor(y_Disp * 25.f) / 25.f;
        vec2 UVs = fs_UVs.xy + vec2(0.f, y_Disp);
        diffuseColor = texture(textureSampler, UVs);
    } else {
        diffuseColor = texture(textureSampler, fs_UVs.xy);
    }

    //vec4 diffuseColor = texture(textureSampler, fs_UVs.xy);

    // Add black lines between blocks (REMOVE WHEN YOU APPLY TEXTURES)
//    bool xBound = fract(fs_Pos.x) < 0.0125 || fract(fs_Pos.x) > 0.9875;
//    bool yBound = fract(fs_Pos.y) < 0.0125 || fract(fs_Pos.y) > 0.9875;
//    bool zBound = fract(fs_Pos.z) < 0.0125 || fract(fs_Pos.z) > 0.9875;
//    if((xBound && yBound) || (xBound && zBound) || (yBound && zBound)) {
//        diffuseColor.rgb = vec3(0,0,0);
//    }

    // Calculate the diffuse term for Lambert shading
    float diffuseTerm = dot(normalize(fs_Nor), normalize(fs_LightVec));
    // Avoid negative lighting values
    diffuseTerm = clamp(diffuseTerm, 0, 1);

    float ambientTerm = 0.2;
    float lightIntensity = diffuseTerm + ambientTerm;   //Add a small float value to the color multiplier
    //to simulate ambient lighting. This ensures that faces that are not
    //lit by our point light are not completely black.

    // Compute final shaded color
    out_Col = vec4(diffuseColor.rgb * lightIntensity, diffuseColor.a);



    if (fs_UVs.z > 0.9 && fs_UVs.z < 1.1) {


        // effect normals based on fbm
        float cellWidth = 1.0 / 8.0;
        float epsilon = cellWidth;

        //float x = floor(gl_FragCoord.x * 8.0) / 8.0;
        float x = fs_Pos.x + fs_Pos.x / 100.f;
        //x = floor(x * 8.0) / 8.0;
       // float x = sin(fs_Pos.x);
        //float z = floor(gl_FragCoord.z * 8.0) / 8.0;
        //float z = fs_Pos.z + fs_Pos.z / 100.f;
        //float y = floor(gl_FragCoord.y * 8.0) / 8.0;
        float y = gl_FragCoord.y;
        //y = floor(y * 8.0) / 8.0;

        //float nx = fbm(vec2(x + epsilon, z)) - fbm(vec2(x - epsilon, z));
        //float nz = fbm(vec2(x, z + epsilon)) - fbm(vec2(x, z - epsilon));

//        vec3 xLeft = vec3(x - epsilon, fbm(vec2(x - epsilon, z)), z);
//        vec3 xRight = vec3(x + epsilon, fbm(vec2(x + epsilon, z)), z);
//        vec3 xPlane = normalize(xRight - xLeft);

        vec3 xLeft = vec3(x - epsilon, fbm(vec2(x - epsilon, y)), y);
        vec3 xRight = vec3(x + epsilon, fbm(vec2(x + epsilon, y)), y);
        vec3 xPlane = normalize(xRight - xLeft);

//        vec3 zUp = vec3(x, fbm(vec2(x, z + epsilon)), z + epsilon);
//        vec3 zDown = vec3(x, fbm(vec2(x, z - epsilon)), z - epsilon);
//        vec3 zPlane = normalize(zUp - zDown);

        vec3 yUp = vec3(x, fbm(vec2(x, y + epsilon) / 256), y + epsilon);
        vec3 yDown = vec3(x, fbm(vec2(x, y - epsilon) / 256), y - epsilon);
        vec3 yPlane = normalize(yUp - yDown);
        //vec3 perturbedNormal = vec3(nx, nz, 1.0);



//        vec3 normal = vec3((xRight.y - xLeft.y) / cellWidth * 3,
//                           1.0,
//                           (yUp.y - yDown.y) / cellWidth * 3);

        //vec3 normal = vec3(fs_Nor) - perturbedNormal;
        vec3 normal = normalize(cross(yPlane, xPlane));


        // Blinn-phong

        // view vector
        vec3 cam_Pos = vec3(u_PlayerPos.x, u_PlayerPos.y + 1.5, u_PlayerPos.z);
        vec3 view = normalize(cam_Pos - vec3(fs_Pos));
        // Light vector
        vec3 light = (vec3(fs_LightVec));

        //normal = vec3(0, 0.5, 0);

        float specE = 100;

        vec3 H = normalize((view + light) / 2.0);
        vec3 N = vec3(normalize(normal));
        //vec3 N = vec3(normalize(fs_Nor));


        float specularIntensity = max(pow(dot(H, N), specE), 0);
        //out_Col = diffuseColor + specularIntensity;
        out_Col += specularIntensity;

        //out_Col.xyz = N;
    }








    //out_Col = texture(textureSampler, fs_UVs.xy);
}
