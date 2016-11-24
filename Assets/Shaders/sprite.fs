#version 330

in vec2 inTexCoord;

out vec4 color;

uniform bool hasSkin;

uniform sampler2D textureUniform;
uniform sampler2D textureSkinUniform;

void main() {
    if(hasSkin) {
		vec4 texShip = texture2D(textureUniform, inTexCoord);
		vec4 texSkin = texture2D(textureSkinUniform, inTexCoord);
		
		if(texShip.a > 0.9) {
			color = mix(texShip, texSkin, 0.6);
		} else {
			color = texShip;
		}
	} else {
		color = texture(textureUniform, inTexCoord);
	}
}
