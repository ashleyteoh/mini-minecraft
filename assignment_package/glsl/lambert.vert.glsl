#version 150
// ^ Change this to version 130 if you have compatibility issues

//This is a vertex shader. While it is called a "shader" due to outdated conventions, this file
//is used to apply matrix transformations to the arrays of vertex data passed to it.
//Since this code is run on your GPU, each vertex is transformed simultaneously.
//If it were run on your CPU, each vertex would have to be processed in a FOR loop, one at a time.
//This simultaneous transformation allows your program to run much faster, especially when rendering
//geometry with millions of vertices.

uniform mat4 u_Model;       // The matrix that defines the transformation of the
                            // object we're rendering. In this assignment,
                            // this will be the result of traversing your scene graph.

uniform mat4 u_ModelInvTr;  // The inverse transpose of the model matrix.
                            // This allows us to transform the object's normals properly
                            // if the object has been non-uniformly scaled.

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
                            // We've written a static matrix for you to use for HW2,
                            // but in HW3 you'll have to generate one yourself

uniform vec4 u_Color;       // When drawing the cube instance, we'll set our uniform color to represent different block types.

in vec4 vs_Pos;             // The array of vertex positions passed to the shader

in vec4 vs_Nor;             // The array of vertex normals passed to the shader

in vec4 vs_Col;             // The array of vertex colors passed to the shader.



out vec4 fs_Pos;
out vec4 fs_Nor;            // The array of normals that has been transformed by u_ModelInvTr. This is implicitly passed to the fragment shader.
out vec4 fs_LightVec;       // The direction in which our virtual light lies, relative to each vertex. This is implicitly passed to the fragment shader.
out vec4 fs_Col;            // The color of each vertex. This is implicitly passed to the fragment shader.

const vec4 lightDir = normalize(vec4(0.5, 1, 0.75, 0));  // The direction of our virtual light, which is used to compute the shading of
                                        // the geometry in the fragment shader.

// added for texturing

in vec4 vs_UVs;
out vec4 fs_UVs;
uniform int u_Time;

// added for water animation
uniform vec3 u_PlayerPos;


void main()
{

    //mat3 invTranspose = mat3(u_ModelInvTr);
    //fs_Pos = vs_Pos + 8 * sin(vs_Pos);
    // if water/animatable
    float t = u_Time / 50.f;
    if (vs_UVs.z == 1) {

        // displace in y
        fs_Pos = vs_Pos;
        //fs_Pos.y -= 0.5;
        float y_disp = 0.125 * (sin(vs_Pos.x) + cos(vs_Pos.z)) * sin(t);
                //smoothstep(-0.25, 0.25, (sin(vs_Pos.x) + cos(vs_Pos.z)) * sin(t));
        //fs_Pos.y += 0.25 * (sin(vs_Pos.x) + cos(vs_Pos.z)) * sin(t);
        fs_Pos.y += y_disp - 0.5;


        // update normals
        // 1. find updated y values of surrounding positions
//        float sampleInterval = 0.5;
//        float xLeft = fs_Pos.x - sampleInterval;
//        float yTemp = 0.125 * (sin(xLeft) + cos(vs_Pos.z)) * sin(t);
//        vec3 left = vec3(xLeft, yTemp, fs_Pos.z);

//        float xRight = fs_Pos.x + sampleInterval;
//        yTemp = 0.125 * (sin(xRight) + cos(vs_Pos.z)) * sin(t);
//        vec3 right = vec3(xRight, yTemp, fs_Pos.z);
//        vec3 xPlane = normalize(right - left);

//        float zUp = fs_Pos.z - sampleInterval;
//        yTemp = 0.125 * (sin(vs_Pos.x) + cos(zUp)) * sin(t);
//        vec3 up = vec3(xLeft, yTemp, fs_Pos.z);

//        float zDown = fs_Pos.x + sampleInterval;
//        yTemp = 0.125 * (sin(vs_Pos.x) + cos(zDown)) * sin(t);
//        vec3 down = vec3(xRight, yTemp, fs_Pos.z);
//        vec3 zPlane = normalize(up - down);

//        fs_Nor = vec4(cross(zPlane, xPlane), 0);
        //fs_Nor = vec4(invTranspose * vec3(fs_Nor), 0);
        //fs_Pos = vs_Pos;

    } else {
        fs_Pos = vs_Pos;
        //fs_Nor = vec4(invTranspose * vec3(vs_Nor), 0);
    }




    fs_Col = vs_Col;                         // Pass the vertex colors to the fragment shader for interpolation
    fs_UVs = vs_UVs;

    mat3 invTranspose = mat3(u_ModelInvTr);
    fs_Nor = vec4(invTranspose * vec3(vs_Nor), 0);          // Pass the vertex normals to the fragment shader for interpolation.
                                                            // Transform the geometry's normals by the inverse transpose of the
                                                            // model matrix. This is necessary to ensure the normals remain
                                                            // perpendicular to the surface after the surface is transformed by
                                                            // the model matrix.


    vec4 modelposition = u_Model * fs_Pos;   // Temporarily store the transformed vertex positions for use below


    fs_LightVec = (lightDir);  // Compute the direction in which the light source lies

    gl_Position = u_ViewProj * modelposition;// gl_Position is a built-in variable of OpenGL which is
                                             // used to render the final positions of the geometry's vertices
}
