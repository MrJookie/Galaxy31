#version 330

uniform vec2 shipPosition;
uniform vec2 windowSize;
uniform float time;

out vec4 color;

float hashIndex(float i) {
	return fract((1.0 + sin(i)) * 123);
}

float randomNoise(in vec2 i) {
    float x = hashIndex(i.x * 50.0);
    float y = hashIndex(i.y * 50.0);
    return fract(x + y);
}

void main() {
    vec2 cameraOffset = shipPosition.xy / 20.0f;
    vec2 samplePosition = (gl_FragCoord.xy + floor(cameraOffset)) / windowSize.xy;

    vec3 finalColor = vec3(0.0);

    float threshold = 0.999;
    float star = randomNoise(samplePosition);
    if (star >= threshold) {
        star = pow((star - threshold) / (1.0 - threshold), 2.0);
		finalColor += vec3(star * cos((time + 10.0) * star));
    }
	
	color = vec4(finalColor, 1.0);
}
