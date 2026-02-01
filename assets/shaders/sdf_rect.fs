#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform vec4 colorTR;
uniform vec4 colorBR;
uniform vec4 colorBL;
uniform vec4 colorTL;

uniform vec2 size;
uniform float roundnessSize;

uniform vec4 outlineColor;
uniform float outlineThickness;

void main()
{
	vec4 bot = mix(colorBL, colorBR, fragTexCoord.x);
	vec4 top = mix(colorTL, colorTR, fragTexCoord.x);
	vec4 fragColor = mix(bot, top, fragTexCoord.y);

	vec2 d = abs((fragTexCoord - 0.5) * size) - (size * 0.5 - roundnessSize);
	float dist = length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - roundnessSize;

	float outlineA = smoothstep(-outlineThickness, -outlineThickness + 2.0, dist);
	fragColor = mix(fragColor, outlineColor, outlineA);

	fragColor.a *= smoothstep(1.0, -1.0, dist);
	finalColor = fragColor;
}