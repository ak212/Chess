uniform vec3 uLightPos;
uniform vec3 UaColor;
uniform vec3 UdColor;
uniform vec3 UsColor;
uniform float Ushine;

varying vec3 vCol;
varying vec3 vNor;
varying vec3 vLightDir;
varying vec3 view;
varying vec3 halfV;

void main() {
	//Phong shading
	//Light color
	vec3 normColor = normalize(vNor);

	//Ambient light
	vec3 ambient = vec3(0.1);

	//Diffuse light
	// max(N dot L, 0)
	vec3 diffuse = vec3(max(dot(normalize(vNor), normalize(vLightDir)), 0.0));

	//Specular light
	vec3 specular = vec3(pow(max(dot(vNor, halfV), 0.0), Ushine));

	//Coloring with material
	ambient *= UaColor;
	diffuse *= UdColor;
	specular *= UsColor;

	gl_FragColor = vec4(ambient, 1.0) + vec4(diffuse, 1.0) + vec4(specular, 1.0);	
}
