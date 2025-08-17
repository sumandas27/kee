#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform vec4 color;
uniform vec2 size;
uniform vec2 p0;
uniform vec2 p1;
uniform vec2 p2;

float dot2(in vec2 v)
{ 
    return dot(v,v); 
}

void main()
{
    vec4 fragColor = color;
    vec2 p = fragTexCoord * size;

    vec2 p0 = p0 * size;
    vec2 p1 = p1 * size;
    vec2 p2 = p2 * size;

    vec2 e0 = p1 - p0;
	vec2 e1 = p2 - p1;
	vec2 e2 = p0 - p2;

    vec2 v0 = p - p0; 
    vec2 v1 = p - p1; 
    vec2 v2 = p - p2; 

    float d0 = dot2(v0 - e0 * clamp(dot(v0, e0) / dot(e0, e0), 0.0, 1.0));
    float d1 = dot2(v1 - e1 * clamp(dot(v1, e1) / dot(e1, e1), 0.0, 1.0));
    float d2 = dot2(v2 - e2 * clamp(dot(v2, e2) / dot(e2, e2), 0.0, 1.0));
    
    float o = e0.x * e2.y - e0.y * e2.x;
    vec2 min12 = min(vec2(d0, o * (v0.x * e0.y - v0.y * e0.x)), vec2(d1, o * (v1.x * e1.y - v1.y * e1.x)));
    vec2 min123 = min(min12, vec2(d2, o * (v2.x * e2.y - v2.y * e2.x)));
    float dist = -sqrt(min123.x) * sign(min123.y);

	fragColor.a *= smoothstep(1.0, -1.0, dist);
	finalColor = fragColor;
}