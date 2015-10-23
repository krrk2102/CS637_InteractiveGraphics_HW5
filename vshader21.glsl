uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 light1_pos;
uniform vec4 light1_diffuse_product;
uniform vec4 light2_pos;
uniform vec4 light2_diffuse_product;

attribute vec4 vPosition;
attribute vec4 vNormal;

varying vec4 color;

void
main()
{
    vec4 diffuse1, diffuse2;
    gl_Position = projection * modelview * vPosition;

    vec3 N = normalize( vNormal.xyz );
    vec3 L1 = normalize( light1_pos.xyz - (modelview*vPosition).xyz );
    vec3 L2 = normalize( light2_pos.xyz - (modelview*vPosition).xyz );
    float Kd1 = max( dot( L1, N ), 0 );
    float Kd2 = max( dot( L2, N ), 0 );
    diffuse1 = Kd1 * light1_diffuse_product;
    diffuse2 = Kd2 * light2_diffuse_product;
    color = vec4( (diffuse1+diffuse2).xyz, 1 );
}
