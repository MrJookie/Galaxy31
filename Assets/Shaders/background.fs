#version 330

uniform vec2 shipPosition;
uniform vec2 windowSize;
uniform float time;

out vec4 color;

float Hash(float i){
	return fract((1.0 + cos(i)) * 415.92653);
}

float Noise2d(in vec2 i){
    float x = Hash(i.x * 37.0);
    float y = Hash(i.y * 57.0);
    return fract(x + y);
}

void main() {
    vec2 cameraOffset = shipPosition.xy / 20.0f;
    vec2 samplePosition = (gl_FragCoord.xy + floor( cameraOffset )) / windowSize.xy;

    vec3 finalColor = vec3(0.0);

    float threshold = 0.999;
    float star = Noise2d(samplePosition);
    if (star >= threshold) {
        star = pow((star - threshold)/(1.0 - threshold), 2.0);
		finalColor += vec3(star * cos(time * star));
    }
	
	color = vec4(finalColor, 1.0);
}
