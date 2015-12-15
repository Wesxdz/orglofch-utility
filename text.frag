// Interpolated values from the vertex shaders
in vec2 UV;

// Output data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2D uFontTexture;

void main(){
	color = texture(uFontTexture, UV);
}