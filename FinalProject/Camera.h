using namespace std;

//Helper class to represent the camera
class Camera {
public:
   //Position of the camera in space
   vec3 position;
   //Represent the horizontal angle
   float theta;
   //Represent the vertical angle
   float phi;

   Camera() {}

   Camera(vec3 pos, float t, float p) {
      position = pos;
      theta = t;
      phi = p;
   }

   //Set the position of the camera
   void setPosition(vec3 pos) {
      position = pos;
   }

   //Set horizontal angle
   void setTheta(float t) {
      theta = t;
   }

   //Set vertical angle
   void setPhi(float p) {
      phi = p;
   }

   //Return camera position
   vec3 getPosition() {
      return position;
   }

   //Return horizontal angle
   float getTheta() {
      return theta;
   }

   //Return vertical angle
   float getPhi() {
      return phi;
   }
};