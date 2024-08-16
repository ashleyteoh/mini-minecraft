#version 150

in vec4 fs_Col;
uniform int fluid;

out vec4 out_Col;

void main()
{
    vec4 blue = vec4(0.1529, 0.6118, 0.9137, 0);
    vec4 red = vec4(0.9843, 0.1922, 0.1294, 0);

    if(fluid == 1) {
        // water
        out_Col = fs_Col + blue;
    }
    else {
        //lava
        out_Col = fs_Col + red;
    }
}
