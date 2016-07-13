#version 330

uniform vec2 iMouse;
uniform vec2 iResolution;

out vec4 color;

float Hash( float n )
{
	return fract( (1.0 + cos(n)) * 415.92653);
}

float Noise2d( in vec2 x )
{
    float xhash = Hash( x.x * 37.0 );
    float yhash = Hash( x.y * 57.0 );
    return fract( xhash + yhash );
}

void main() {
    // Add a camera offset in "FragCoord-space".
    vec2 vCameraOffset = iMouse.xy;
    vec2 vSamplePos = ( gl_FragCoord.xy + floor( vCameraOffset ) ) / iResolution.xy;

    vec3 vColor  = vec3(0.0, 0.0, 0.0);
	
	// Sky Background Color
	vColor += vec3( 0.0, 0.0, 0.0 );

    // Stars
    // Note: Choose fThreshhold in the range [0.99, 0.9999].
    // Higher values (i.e., closer to one) yield a sparser starfield.
    float fThreshhold = 0.999;
    float StarVal = Noise2d( vSamplePos );
    if ( StarVal >= fThreshhold )
    {
        StarVal = pow( (StarVal - fThreshhold)/(1.0 - fThreshhold), 6.0 );
		vColor += vec3( StarVal );
    }
	
	color = vec4(vColor, 1.0);
}
