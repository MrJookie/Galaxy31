#version 330

in vec2 inTexCoord;

out vec4 color;

uniform sampler2D textureUniform;
uniform sampler2D textureSkinUniform;

void main() {
    //color = texture(textureUniform, inTexCoord);
    
    vec4 texShip = texture2D(textureUniform, inTexCoord);
    vec4 texSkin = texture2D(textureSkinUniform, inTexCoord);
    
    if(texShip.a > 0.0) {
		color = mix(texShip, texSkin, texSkin.a);
	} else {
		color = texShip;
	}
}
