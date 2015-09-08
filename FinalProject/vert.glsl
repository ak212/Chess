uniform mat4 uProjM;
uniform mat4 uViewM;
uniform mat4 uModelM;

uniform vec3 uLightPos;
uniform vec3 UaColor;
uniform vec3 UdColor;
uniform vec3 UsColor;
uniform float Ushine;

attribute vec4 aPos;
attribute vec3 aNor;

varying vec3 vCol;
varying vec3 vNor;
varying vec3 vLightDir;
varying vec3 view;
varying vec3 halfV;

void main() {
   //Light color
   vec3 normColor = normalize(aNor);

   //Position world to pixel
   vec3 MVVertex = vec3(uViewM * uModelM * aPos);
   //Normal world to pixel (N)
   vec3 MVNormal = normalize(vec3(uViewM * uModelM * vec4(aNor, 0.0)));

   // Light direction to vertex (L)
   vLightDir = normalize(uLightPos - MVVertex);

   // View vector and half angle vector
   	view = normalize(vec3(0) - vec3(aPos));
	halfV = normalize(vLightDir + view);

	//Phong shading
	vNor = MVNormal;

	gl_Position = uProjM * uViewM * uModelM * aPos;
}