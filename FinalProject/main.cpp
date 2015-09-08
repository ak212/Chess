#include "glad\glad.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <map>
#include <cassert>
#include <cmath>
#include <stdio.h>
#include "GLSL.h"
#include "tiny_obj_loader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "RenderingHelper.h"

#include "Camera.h"
#include "Piece.h"

#define SQUARES 8
#define BOARD_SIZE 50
#define PIECES_PER_SIDE 16
#define BLACK 5
#define WHITE 2


GLFWwindow* window;
using namespace std;
using namespace glm;
using namespace tinyobj;

vector<material_t> materials;

vector<shape_t> cube;
vector<shape_t> cylinder;
vector<shape_t> sphere;
vector<shape_t> horse;
vector<shape_t> crossShape;
vector<shape_t> crown;
vector<shape_t> bowl;

int g_width, g_height, g_camera_id = 0, free_cam = 0;
float g_Camtrans = -2.5;
vec3 g_light(0, 10, 0);

Camera g_camera = Camera(vec3(23, 24, -8), 0.0f, -0.81f);
mat4 ProjectionMatrix, ViewMatrix;

float mouseSpeed = 0.005f;
double mouseX, mouseY;
float speed = 0.5f;
int selectedRank, selectedFile, selectedPiece = -1;

bool line = false;

float moon_angle = 0;
vec3 piece_positions[SQUARES][SQUARES];

//initialize white pieces
Piece wPawnA2, wPawnB2, wPawnC2, wPawnD2, wPawnE2, wPawnF2, wPawnG2, wPawnH2;
Piece wRookA1, wRookH1, wKnightB1, wKnightG1, wBishopC1, wBishopF1, wQueenD1, wKingE1;

//initialize black pieces
Piece bPawnA7, bPawnB7, bPawnC7, bPawnD7, bPawnE7, bPawnF7, bPawnG7, bPawnH7;
Piece bRookA8, bRookH8, bKnightB8, bKnightG8, bBishopC8, bBishopF8, bQueenD8, bKingE8;

//initialize vector to hold all of the pieces
vector<Piece> gamePieces;

//declare a matrix stack
RenderingHelper ModelTrans;

GLuint ShadeProg;

//Buffers
GLuint posBufObjG = 0;
GLuint norBufObjG = 0;

GLuint lightBoardBufObjG = 0;
GLuint lightBoardNorBufObjG = 0;
GLuint darkBoardBufObjG = 0;
GLuint darkBoardNorBufObjG = 0;

GLuint posBufObjBase = 0;
GLuint norBufObjBase = 0;
GLuint indBufObjBase = 0;

GLuint posBufObjCyl = 0;
GLuint norBufObjCyl = 0;
GLuint indBufObjCyl = 0;

GLuint posBufObjSph = 0;
GLuint norBufObjSph = 0;
GLuint indBufObjSph = 0;

GLuint posBufObjHor = 0;
GLuint norBufObjHor = 0;
GLuint indBufObjHor = 0;

GLuint posBufObjRing = 0;
GLuint norBufObjRing = 0;
GLuint indBufObjRing = 0;

GLuint posBufObjCross = 0;
GLuint norBufObjCross = 0;
GLuint indBufObjCross = 0;

GLuint posBufObjCrown = 0;
GLuint norBufObjCrown = 0;
GLuint indBufObjCrown = 0;

GLuint posBufObjBowl = 0;
GLuint norBufObjBowl = 0;
GLuint indBufObjBowl = 0;

//Handles to the shader data
GLint h_aPosition;
GLint h_aNormal;
GLint h_uModelMatrix;
GLint h_uViewMatrix;
GLint h_uProjMatrix;
GLint h_uLightPos;
GLint h_uMatAmb, h_uMatDif, h_uMatSpec, h_uMatShine;

/* helper function to make sure your matrix handle is correct */
inline void safe_glUniformMatrix4fv(const GLint handle, const GLfloat data[]) {
   if (handle >= 0)
      glUniformMatrix4fv(handle, 1, GL_FALSE, data);
}

/* helper function to send materials to the shader - you must create your own */
void SetMaterial(int i) {

   glUseProgram(ShadeProg);
   switch (i) {
   case 0: //brown
      glUniform3f(h_uMatAmb, 0.02, 0.02, 0.1);
      glUniform3f(h_uMatDif, 0.1, 0.08, 0.01);
      glUniform3f(h_uMatSpec, 0.32, 0.22, 0.05);
      glUniform1f(h_uMatShine, 10.0);
      break;
   case 2: //white
      glUniform3f(h_uMatAmb, 0.09, 0.07, 0.08);
      glUniform3f(h_uMatDif, 0.94, 0.94, 0.94);
      glUniform3f(h_uMatSpec, 1.0, 0.913, 0.8);
      glUniform1f(h_uMatShine, 200.0);
      break;
   case 3: // light brown
      glUniform3f(h_uMatAmb, 0.08, 0.06, 0.04);
      glUniform3f(h_uMatDif, 0.71, 0.53, 0.39);
      glUniform3f(h_uMatSpec, 0.05, 0.05, 0.05);
      glUniform1f(h_uMatShine, 4.0);
      break;
   case 4: // light tan
      glUniform3f(h_uMatAmb, 0.1, 0.1, 0.1);
      glUniform3f(h_uMatDif, 0.94, 0.85, 0.71);
      glUniform3f(h_uMatSpec, 0.04, 0.05, 0.05);
      glUniform1f(h_uMatShine, 2.0);
      break;
   case 5: // black
      glUniform3f(h_uMatAmb, 0.03, 0.03, 0.03);
      glUniform3f(h_uMatDif, 0.3, 0.3, 0.3);
      glUniform3f(h_uMatSpec, 0.03, 0.03, 0.03);
      glUniform1f(h_uMatShine, 20.0);
      break;
   }
}

void SetCamera(int i) {
   switch (i) {
   case 0: //white side perspective
      g_camera = Camera(vec3(23, 24, -8), 0.0f, -0.81f);
      break;
   case 1: //black side perspective
      g_camera = Camera(vec3(23, 24, 53), 3.14f, -0.81f);
      break;
   case 2: //overhead perspective
      g_camera = Camera(vec3(24, 30, 24), 4.72f, -1.56f);
      break;
   }
}

/* helper function to set projection matrix - don't touch */
void SetProjectionMatrix() {
   ProjectionMatrix = perspective(90.0f, (float)g_width / g_height, 0.1f, 100.f);
   safe_glUniformMatrix4fv(h_uProjMatrix, value_ptr(ProjectionMatrix));
}

/* camera controls - do not change beyond the current set up to rotate*/
void SetView() {
   // Get mouse position
   double xpos, ypos;
   glfwGetCursorPos(window, &xpos, &ypos);

   //cout << "x: " << xpos << " y: " << ypos << endl;
   //cout << "adjusted x: " << xpos / g_width << " adjusted y: " << ypos / g_height << endl;

   if (free_cam) {

      // glfwGetTime is called only once, the first time this function is called
      static double lastTime = glfwGetTime();

      // Compute time difference between current and last frame
      double currentTime = glfwGetTime();
      float deltaTime = float(currentTime - lastTime);

      // Reset mouse position for next frame
      glfwSetCursorPos(window, g_width / 2, g_height / 2);

      // Compute new orientation
      g_camera.setTheta(g_camera.getTheta() + mouseSpeed * float(g_width / 2 - xpos));
      g_camera.setPhi(g_camera.getPhi() + mouseSpeed * float(g_height / 2 - ypos));

      //between 89 and -89 degrees
      g_camera.setPhi(max(g_camera.getPhi(), -1.56f));
      g_camera.setPhi(min(g_camera.getPhi(), 1.56f));
   }
   
   //cout << g_camera.getPosition().x << " " << g_camera.getPosition().y << " " << g_camera.getPosition().z << endl;

   //cout << "theta: " << g_camera.getTheta() << " phi: " << g_camera.getPhi() << endl;

   //Direction: Spherical coordinates to Cartesian coordinates conversion
   vec3 direction(cos(g_camera.getPhi()) * sin(g_camera.getTheta()), sin(g_camera.getPhi()), cos(g_camera.getPhi()) * cos(g_camera.getTheta()));

   //Right vector
   vec3 rightV = vec3(sin(g_camera.getTheta() - 3.14f / 2.0f), 0, cos(g_camera.getTheta() - 3.14f / 2.0f));

   //Up vector
   vec3 upV = cross(rightV, direction);

   if (free_cam) {
      // Move forward
      if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
         g_camera.setPosition(g_camera.getPosition() + direction * speed);
      }
      // Move backward
      if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
         g_camera.setPosition(g_camera.getPosition() - direction * speed);
      }
      // Strafe right
      if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
         g_camera.setPosition(g_camera.getPosition() + rightV * speed);
      }
      // Strafe left
      if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
         g_camera.setPosition(g_camera.getPosition() - rightV * speed);
      }
   }

   ViewMatrix = lookAt(
      g_camera.getPosition(),           // Camera is here
      g_camera.getPosition() + direction, // and looks here : at the same position, plus "direction"
      upV                  // Head is up (set to 0,-1,0 to look upside-down)
      );
   safe_glUniformMatrix4fv(h_uViewMatrix, value_ptr(ViewMatrix));
}

void nextTurn() {
   SetCamera(g_camera_id = (g_camera_id + 1) % 2);
   //Take the camera out of free cam if it is currently set
   free_cam = 0;

   //If there is a game piece selected
   if (selectedPiece != -1) {
      for (int i = 0; i < gamePieces.size(); i++) {
         //If there is another game piece drawn in the same square as the one selected
         if (gamePieces[i].rank == gamePieces[selectedPiece].rank && gamePieces[i].file == gamePieces[selectedPiece].file && i != selectedPiece) {
            if (gamePieces[i].material != gamePieces[selectedPiece].material)
               //Kill the other piece
               gamePieces[i].draw = 0;
         }
      }
      //Unselect the previously selected piece
      selectedPiece = -1;
   }
}

bool squareCheck(int rank, int file) {
   return !(selectedRank == rank && selectedFile == file);
}

bool pieceMoved(int rank, int file) {
   return !(gamePieces[selectedPiece].getRank() == rank && gamePieces[selectedPiece].getFile() == file);
}

bool pieceSelected() {
   for (int i = 0; i < gamePieces.size(); i++) {
      if (gamePieces[i].selected) {
         return true;
      }
   }

   return false;
}

//Function to select a game piece
void selectPiece(int rank, int file) {
   if (squareCheck(rank, file)) {
      //If there is a piece on the square that was selected, select that piece
      for (int i = 0; i < gamePieces.size(); i++) {
         if (gamePieces[i].rank == rank && gamePieces[i].file == file) {
            if (!pieceSelected()) {
               selectedPiece = i;
               selectedFile = file;
               gamePieces[i].selected = 1;
            }
         }
      }

      //Move the piece and switch sides
      if (pieceSelected() && pieceMoved(rank, file)) {
         gamePieces[selectedPiece].setRank(rank);
         gamePieces[selectedPiece].setFile(file);
         nextTurn();
      }

      //Unselect previously selected piece, if there was one
      for (int i = 0; i < gamePieces.size(); i++) {
         if (i != selectedPiece)
            gamePieces[i].selected = 0;
      }
   }
}

//Function to determine where a mouse click was in space
void mouseClick() {
   GLbyte color[4];
   GLfloat depth;
   GLuint index;
   int i, j;
   int rank = -1, file = -1;

   cout << "mouse clicked at x: " << mouseX << " y: " << mouseY << endl;

   //Get the selected pixel
   glReadPixels(mouseX, g_height - mouseY - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, color);
   glReadPixels(mouseX, g_height - mouseY - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
   glReadPixels(mouseX, g_height - mouseY - 1, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);

   //printf("Clicked on pixel %f, %f, color %02hhx%02hhx%02hhx%02hhx, depth %f, stencil index %u\n",
      //mouseX, mouseY, color[0], color[1], color[2], color[3], depth, index);

   //Find where the selected pixel is in object coordinates
   vec4 viewport = vec4(0, 0, g_width, g_height);
   vec3 wincoord = vec3(mouseX, g_height - mouseY - 1, depth);
   vec3 objcoord = unProject(wincoord, ViewMatrix, ProjectionMatrix, viewport);

   //printf("Coordinates in object space: %f, %f, %f\n", objcoord.x, objcoord.y, objcoord.z);
  //cout << g_camera.getPosition().x << " " << g_camera.getPosition().y << " " << g_camera.getPosition().z << endl;

   //If the click was on the board, find the rank
   for (i = 0; i < SQUARES; i++) {
      if (objcoord.x > BOARD_SIZE / SQUARES * i && objcoord.x < BOARD_SIZE / SQUARES * (i + 1)) {
         rank = i;
      }
   }

   //If the click was on the board, find the file
   for (i = 0; i < SQUARES; i++) {
      if (objcoord.z > BOARD_SIZE / SQUARES * i && objcoord.z < BOARD_SIZE / SQUARES * (i + 1)) {
         file = i;
      }
   }

   if (rank > -1 && file > -1) {
      selectPiece(rank, file);
   }
   else {
      selectedPiece = -1;
   }
}

/* model transforms */
void SetModel(vec3 trans, float rot, float sc) {
   mat4 Trans = translate(mat4(1.0f), trans);
   mat4 RotateY = rotate(mat4(1.0f), rot, vec3(0.0f, 1, 0));
   mat4 Sc = scale(mat4(1.0f), vec3(sc));
   mat4 com = Trans*RotateY*Sc;
   safe_glUniformMatrix4fv(h_uModelMatrix, value_ptr(com));
}

//Given a vector of shapes which has already been read from an obj file
// resize all vertices to the range [-1, 1]
void resize_obj(vector<shape_t> &shapes) {
   float minX, minY, minZ;
   float maxX, maxY, maxZ;
   float scaleX, scaleY, scaleZ;
   float shiftX, shiftY, shiftZ;
   float epsilon = 0.001;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t i = 0; i < shapes.size(); i++) {
      for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
         if (shapes[i].mesh.positions[3 * v + 0] < minX) minX = shapes[i].mesh.positions[3 * v + 0];
         if (shapes[i].mesh.positions[3 * v + 0] > maxX) maxX = shapes[i].mesh.positions[3 * v + 0];

         if (shapes[i].mesh.positions[3 * v + 1] < minY) minY = shapes[i].mesh.positions[3 * v + 1];
         if (shapes[i].mesh.positions[3 * v + 1] > maxY) maxY = shapes[i].mesh.positions[3 * v + 1];

         if (shapes[i].mesh.positions[3 * v + 2] < minZ) minZ = shapes[i].mesh.positions[3 * v + 2];
         if (shapes[i].mesh.positions[3 * v + 2] > maxZ) maxZ = shapes[i].mesh.positions[3 * v + 2];
      }

      //From min and max compute necessary scale and shift for each dimension
      float maxExtent, xExtent, yExtent, zExtent;
      xExtent = maxX - minX;
      yExtent = maxY - minY;
      zExtent = maxZ - minZ;
      if (xExtent >= yExtent && xExtent >= zExtent) {
         maxExtent = xExtent;
      }
      if (yExtent >= xExtent && yExtent >= zExtent) {
         maxExtent = yExtent;
      }
      if (zExtent >= xExtent && zExtent >= yExtent) {
         maxExtent = zExtent;
      }
      scaleX = 2.0 / maxExtent;
      shiftX = minX + (xExtent / 2.0);
      scaleY = 2.0 / maxExtent;
      shiftY = minY + (yExtent / 2.0);
      scaleZ = 2.0 / maxExtent;
      shiftZ = minZ + (zExtent) / 2.0;

      //Go through all verticies shift and scale them
      for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
         shapes[i].mesh.positions[3 * v + 0] = (shapes[i].mesh.positions[3 * v + 0] - shiftX) * scaleX;
         assert(shapes[i].mesh.positions[3 * v + 0] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3 * v + 0] <= 1.0 + epsilon);
         shapes[i].mesh.positions[3 * v + 1] = (shapes[i].mesh.positions[3 * v + 1] - shiftY) * scaleY;
         assert(shapes[i].mesh.positions[3 * v + 1] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3 * v + 1] <= 1.0 + epsilon);
         shapes[i].mesh.positions[3 * v + 2] = (shapes[i].mesh.positions[3 * v + 2] - shiftZ) * scaleZ;
         assert(shapes[i].mesh.positions[3 * v + 2] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3 * v + 2] <= 1.0 + epsilon);
      }
   }
}

void loadShapes(const string &objFile, vector<shape_t>& shapes) {
   string err = LoadObj(shapes, materials, objFile.c_str());
   if (!err.empty()) {
      cerr << err << endl;
   }
   resize_obj(shapes);
}

vector<float> getNorBuf(vector<shape_t>& shapes) {
   vector<float> norBuf;
   int idx1, idx2, idx3;
   vec3 v1, v2, v3, normal;
   //for every vertex initialize a normal to 0
   for (int j = 0; j < shapes[0].mesh.positions.size() / 3; j++) {
      norBuf.push_back(0);
      norBuf.push_back(0);
      norBuf.push_back(0);
   }
   //compute the normals for every face
   //then add its normal to its associated vertex
   for (int i = 0; i < shapes[0].mesh.indices.size() / 3; i++) {
      idx1 = shapes[0].mesh.indices[3 * i + 0];
      idx2 = shapes[0].mesh.indices[3 * i + 1];
      idx3 = shapes[0].mesh.indices[3 * i + 2];
      v1 = vec3(shapes[0].mesh.positions[3 * idx1 + 0], shapes[0].mesh.positions[3 * idx1 + 1], shapes[0].mesh.positions[3 * idx1 + 2]);
      v2 = vec3(shapes[0].mesh.positions[3 * idx2 + 0], shapes[0].mesh.positions[3 * idx2 + 1], shapes[0].mesh.positions[3 * idx2 + 2]);
      v3 = vec3(shapes[0].mesh.positions[3 * idx3 + 0], shapes[0].mesh.positions[3 * idx3 + 1], shapes[0].mesh.positions[3 * idx3 + 2]);

      normal = normalize(cross(v2 - v1, v3 - v1));

      norBuf[3 * idx1 + 0] += normal.x;
      norBuf[3 * idx1 + 1] += normal.y;
      norBuf[3 * idx1 + 2] += normal.z;
      norBuf[3 * idx2 + 0] += normal.x;
      norBuf[3 * idx2 + 1] += normal.y;
      norBuf[3 * idx2 + 2] += normal.z;
      norBuf[3 * idx3 + 0] += normal.x;
      norBuf[3 * idx3 + 1] += normal.y;
      norBuf[3 * idx3 + 2] += normal.z;
   }

   return norBuf;
}

//Map out the vertices for the board
void initBoard() {
   const int numData = SQUARES * SQUARES * 6 * 3 / 2;

   GLfloat g_light_board_data[numData], g_dark_board_data[numData];
   int count = 0;
   int inc = BOARD_SIZE / SQUARES;
   float x = 0.0f, y = 2.0f, z = 0.0f;
   int i, j;

   for (i = 0; i < SQUARES; i++, x += inc) {
      for (j = 0; j < SQUARES; j++, z += inc) {
         if (i % 2 == 0 && j % 2 == 0 || i % 2 == 1 && j % 2 == 1) {
            g_light_board_data[count++] = x;
            g_light_board_data[count++] = y;
            g_light_board_data[count++] = z;

            g_light_board_data[count++] = x;
            g_light_board_data[count++] = y;
            g_light_board_data[count++] = z + inc;

            g_light_board_data[count++] = x + inc;
            g_light_board_data[count++] = y;
            g_light_board_data[count++] = z;

            g_light_board_data[count++] = x + inc;
            g_light_board_data[count++] = y;
            g_light_board_data[count++] = z + inc;

            g_light_board_data[count++] = x;
            g_light_board_data[count++] = y;
            g_light_board_data[count++] = z + inc;

            g_light_board_data[count++] = x + inc;
            g_light_board_data[count++] = y;
            g_light_board_data[count++] = z;
         }
      }
      z = 0;
   }

   x = 0.0f;
   count = 0;

   for (i = 0; i < SQUARES; i++, x += inc) {
      for (j = 0; j < SQUARES; j++, z += inc) {
         if (i % 2 == 0 && j % 2 == 1 || i % 2 == 1 && j % 2 == 0) {
            g_dark_board_data[count++] = x;
            g_dark_board_data[count++] = y;
            g_dark_board_data[count++] = z;

            g_dark_board_data[count++] = x;
            g_dark_board_data[count++] = y;
            g_dark_board_data[count++] = z + inc;

            g_dark_board_data[count++] = x + inc;
            g_dark_board_data[count++] = y;
            g_dark_board_data[count++] = z;

            g_dark_board_data[count++] = x + inc;
            g_dark_board_data[count++] = y;
            g_dark_board_data[count++] = z + inc;

            g_dark_board_data[count++] = x;
            g_dark_board_data[count++] = y;
            g_dark_board_data[count++] = z + inc;

            g_dark_board_data[count++] = x + inc;
            g_dark_board_data[count++] = y;
            g_dark_board_data[count++] = z;
         }
      }
      z = 0;
   }


   GLfloat g_light_board_nor[numData], g_dark_board_nor[numData];
   y = 1.0f;

   for (count = 0; count < numData;) {
      g_light_board_nor[count++] = 0.0f;
      g_light_board_nor[count++] = y;
      g_light_board_nor[count++] = 0.0f;
   }

   for (count = 0; count < numData;) {
      g_dark_board_nor[count++] = 0.0f;
      g_dark_board_nor[count++] = y;
      g_dark_board_nor[count++] = 0.0f;
   }

   glGenBuffers(1, &lightBoardBufObjG);
   glBindBuffer(GL_ARRAY_BUFFER, lightBoardBufObjG);
   glBufferData(GL_ARRAY_BUFFER, sizeof(g_light_board_data), g_light_board_data, GL_STATIC_DRAW);

   glGenBuffers(1, &lightBoardNorBufObjG);
   glBindBuffer(GL_ARRAY_BUFFER, lightBoardNorBufObjG);
   glBufferData(GL_ARRAY_BUFFER, sizeof(g_light_board_nor), g_light_board_nor, GL_STATIC_DRAW);

   glGenBuffers(1, &darkBoardBufObjG);
   glBindBuffer(GL_ARRAY_BUFFER, darkBoardBufObjG);
   glBufferData(GL_ARRAY_BUFFER, sizeof(g_dark_board_data), g_dark_board_data, GL_STATIC_DRAW);

   glGenBuffers(1, &darkBoardNorBufObjG);
   glBindBuffer(GL_ARRAY_BUFFER, darkBoardNorBufObjG);
   glBufferData(GL_ARRAY_BUFFER, sizeof(g_dark_board_nor), g_dark_board_nor, GL_STATIC_DRAW);
}

//Initialize the buffers for each loaded shape
void initShape(vector<shape_t>& shapes, GLuint *shapePosBuf, GLuint *shapeNorBuf, GLuint *shapeIndBuf) {
   // Send the position array to the GPU
   const vector<float> &posBuf = shapes[0].mesh.positions;
   glGenBuffers(1, shapePosBuf);
   glBindBuffer(GL_ARRAY_BUFFER, *shapePosBuf);
   glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);

   // compute the normals per vertex 
   vector<float> norBuf = getNorBuf(shapes);

   glGenBuffers(1, shapeNorBuf);
   glBindBuffer(GL_ARRAY_BUFFER, *shapeNorBuf);
   glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);

   // Send the index array to the GPU
   const vector<unsigned int> &indBuf = shapes[0].mesh.indices;
   glGenBuffers(1, shapeIndBuf);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *shapeIndBuf);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);

   // Unbind the arrays
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   GLSL::checkVersion();
   assert(glGetError() == GL_NO_ERROR);
}

//Initialize the ground - NOT USED
void initGround() {

   float G_edge = 80;
   GLfloat g_backgnd_data[] = {
      -G_edge, 0.0f, -G_edge,
      -G_edge, 0.0f, G_edge,
      G_edge, 0.0f, -G_edge,
      -G_edge, 0.0f, G_edge,
      G_edge, 0.0f, -G_edge,
      G_edge, 0.0f, G_edge,
   };


   GLfloat nor_Buf_G[] = {
      0.0f, 1.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
   };

   glGenBuffers(1, &posBufObjG);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjG);
   glBufferData(GL_ARRAY_BUFFER, sizeof(g_backgnd_data), g_backgnd_data, GL_STATIC_DRAW);

   glGenBuffers(1, &norBufObjG);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjG);
   glBufferData(GL_ARRAY_BUFFER, sizeof(nor_Buf_G), nor_Buf_G, GL_STATIC_DRAW);

}

//Initialize the location on the square for each piece to move to
void initPiecePositions() {
   float center = BOARD_SIZE / SQUARES / 2, next = BOARD_SIZE / SQUARES;

   for (int i = 0; i < SQUARES; i++) {
      for (int j = 0; j < SQUARES; j++) {
         piece_positions[i][j] = vec3(center + (next * i), 2.15, center + (next * j));
      }
   }
}

//Initialize base values for all of the pieces
void initPieces() {
   //initialize white pieces
   wPawnA2 = Piece("pawn", 0, 1, WHITE, 0, 1);
   wPawnB2 = Piece("pawn", 1, 1, WHITE, 0, 1);
   wPawnC2 = Piece("pawn", 2, 1, WHITE, 0, 1);
   wPawnD2 = Piece("pawn", 3, 1, WHITE, 0, 1);
   wPawnE2 = Piece("pawn", 4, 1, WHITE, 0, 1);
   wPawnF2 = Piece("pawn", 5, 1, WHITE, 0, 1);
   wPawnG2 = Piece("pawn", 6, 1, WHITE, 0, 1);
   wPawnH2 = Piece("pawn", 7, 1, WHITE, 0, 1);
   wRookA1 = Piece("rook", 0, 0, WHITE, 0, 1);
   wRookH1 = Piece("rook", 7, 0, WHITE, 0, 1);
   wKnightB1 = Piece("knight", 1, 0, WHITE, 0, 1);
   wKnightG1 = Piece("knight", 6, 0, WHITE, 0, 1);
   wBishopC1 = Piece("bishop", 2, 0, WHITE, 0, 1);
   wBishopF1 = Piece("bishop", 5, 0, WHITE, 0, 1);
   wQueenD1 = Piece("queen", 3, 0, WHITE, 0, 1);
   wKingE1 = Piece("king", 4, 0, WHITE, 0, 1);

   //initialize black pieces
   bPawnA7 = Piece("pawn", 0, 6, BLACK, 180, 1);
   bPawnB7 = Piece("pawn", 1, 6, BLACK, 180, 1);
   bPawnC7 = Piece("pawn", 2, 6, BLACK, 180, 1);
   bPawnD7 = Piece("pawn", 3, 6, BLACK, 180, 1);
   bPawnE7 = Piece("pawn", 4, 6, BLACK, 180, 1);
   bPawnF7 = Piece("pawn", 5, 6, 5, 180, 1);
   bPawnG7 = Piece("pawn", 6, 6, BLACK, 180, 1);
   bPawnH7 = Piece("pawn", 7, 6, BLACK, 180, 1);
   bRookA8 = Piece("rook", 0, 7, BLACK, 180, 1);
   bRookH8 = Piece("rook", 7, 7, BLACK, 180, 1);
   bKnightB8 = Piece("knight", 1, 7, BLACK, 180, 1);
   bKnightG8 = Piece("knight", 6, 7, BLACK, 180, 1);
   bBishopC8 = Piece("bishop", 2, 7, BLACK, 180, 1);
   bBishopF8 = Piece("bishop", 5, 7, BLACK, 180, 1);
   bQueenD8 = Piece("queen", 3, 7, BLACK, 180, 1);
   bKingE8 = Piece("king", 4, 7, BLACK, 180, 1);

   gamePieces = { wPawnA2, wPawnB2, wPawnC2, wPawnD2, wPawnE2, wPawnF2, wPawnG2, wPawnH2,
      wRookA1, wRookH1, wKnightB1, wKnightG1, wBishopC1, wBishopF1, wQueenD1, wKingE1,
      bPawnA7, bPawnB7, bPawnC7, bPawnD7, bPawnE7, bPawnF7, bPawnG7, bPawnH7,
      bRookA8, bRookH8, bKnightB8, bKnightG8, bBishopC8, bBishopF8, bQueenD8, bKingE8 };
}

//Call all of the initialization functions
void initGL() {
   // Set the background color
   glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
   // Enable Z-buffer test
   glEnable(GL_DEPTH_TEST);

   //initialize the ModelTrans matrix stack
   ModelTrans.useModelViewMatrix();
   ModelTrans.loadIdentity();

   initPiecePositions();
   initBoard();
   initPieces();
   initShape(cube, &posBufObjBase, &norBufObjBase, &indBufObjBase);
   initShape(horse, &posBufObjHor, &norBufObjHor, &indBufObjHor);
   initShape(sphere, &posBufObjSph, &norBufObjSph, &indBufObjSph);
   initShape(crossShape, &posBufObjCross, &norBufObjCross, &indBufObjCross);
   initShape(cylinder, &posBufObjCyl, &norBufObjCyl, &indBufObjCyl);
   initShape(crown, &posBufObjCrown, &norBufObjCrown, &indBufObjCrown);
   initShape(bowl, &posBufObjBowl, &norBufObjBowl, &indBufObjBowl);

   //initGround();
}

bool installShaders(const string &vShaderName, const string &fShaderName) {
   GLint rc;

   // Create shader handles
   GLuint VS = glCreateShader(GL_VERTEX_SHADER);
   GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);

   // Read shader sources
   const char *vshader = GLSL::textFileRead(vShaderName.c_str());
   const char *fshader = GLSL::textFileRead(fShaderName.c_str());
   glShaderSource(VS, 1, &vshader, NULL);
   glShaderSource(FS, 1, &fshader, NULL);

   // Compile vertex shader
   glCompileShader(VS);
   GLSL::printError();
   glGetShaderiv(VS, GL_COMPILE_STATUS, &rc);
   GLSL::printShaderInfoLog(VS);
   if (!rc) {
      printf("Error compiling vertex shader %s\n", vShaderName.c_str());
      return false;
   }

   // Compile fragment shader
   glCompileShader(FS);
   GLSL::printError();
   glGetShaderiv(FS, GL_COMPILE_STATUS, &rc);
   GLSL::printShaderInfoLog(FS);
   if (!rc) {
      printf("Error compiling fragment shader %s\n", fShaderName.c_str());
      return false;
   }

   // Create the program and link
   ShadeProg = glCreateProgram();
   glAttachShader(ShadeProg, VS);
   glAttachShader(ShadeProg, FS);
   glLinkProgram(ShadeProg);

   GLSL::printError();
   glGetProgramiv(ShadeProg, GL_LINK_STATUS, &rc);
   GLSL::printProgramInfoLog(ShadeProg);
   if (!rc) {
      printf("Error linking shaders %s and %s\n", vShaderName.c_str(), fShaderName.c_str());
      return false;
   }

   /* get handles to attribute data */
   h_aPosition = GLSL::getAttribLocation(ShadeProg, "aPos");
   h_aNormal = GLSL::getAttribLocation(ShadeProg, "aNor");
   h_uProjMatrix = GLSL::getUniformLocation(ShadeProg, "uProjM");
   h_uViewMatrix = GLSL::getUniformLocation(ShadeProg, "uViewM");
   h_uModelMatrix = GLSL::getUniformLocation(ShadeProg, "uModelM");
   h_uLightPos = GLSL::getUniformLocation(ShadeProg, "uLightPos");
   h_uMatAmb = GLSL::getUniformLocation(ShadeProg, "UaColor");
   h_uMatDif = GLSL::getUniformLocation(ShadeProg, "UdColor");
   h_uMatSpec = GLSL::getUniformLocation(ShadeProg, "UsColor");
   h_uMatShine = GLSL::getUniformLocation(ShadeProg, "Ushine");

   assert(glGetError() == GL_NO_ERROR);
   return true;
}

//NOT USED
void drawGround() {
   //draw the ground
   SetModel(vec3(0, 0, 0), 0, 40);
   SetMaterial(5);
   glEnableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjG);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjG);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   glDrawArrays(GL_TRIANGLES, 0, 2 * 3);
   GLSL::disableVertexAttribArray(h_aPosition);
   GLSL::disableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

//Draw the game board
void drawBoard() {
   //Draw tan squares
   SetModel(vec3(0, 0, 0), 0, 1);
   SetMaterial(4);
   glEnableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, lightBoardBufObjG);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, lightBoardNorBufObjG);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   glDrawArrays(GL_TRIANGLES, 0, SQUARES * SQUARES * 3);
   GLSL::disableVertexAttribArray(h_aPosition);
   GLSL::disableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   //Draw brown squares
   SetMaterial(3);
   glEnableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, darkBoardBufObjG);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, darkBoardNorBufObjG);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   glDrawArrays(GL_TRIANGLES, 0, SQUARES * SQUARES * 3);
   GLSL::disableVertexAttribArray(h_aPosition);
   GLSL::disableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   //draw the board base
   // Enable and bind position array for drawing
   SetMaterial(0);
   GLSL::enableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjBase);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Enable and bind normal array for drawing
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjBase);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Bind index array for drawing
   int nIndices = (int)cube[0].mesh.indices.size();
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjBase);

   //create the model transforms 
   ModelTrans.loadIdentity();
   ModelTrans.translate(vec3(BOARD_SIZE / 2 - 1, 0, BOARD_SIZE / 2 - 1));
   ModelTrans.scale(BOARD_SIZE / 2, 1.99, BOARD_SIZE / 2);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
}

//Initialize the base of each piece
void setBase(int rank, int file, int rot) {
   //create the model transforms 
   ModelTrans.loadIdentity();
   //Move the piece to the passed square
   ModelTrans.translate(vec3(piece_positions[rank][file].x, piece_positions[rank][file].y, piece_positions[rank][file].z));
   ModelTrans.rotate(rot, glm::vec3(0, 1, 0));
   ModelTrans.pushMatrix();

   //Bottom neck
   ModelTrans.translate(vec3(0, 0.3, 0));
   ModelTrans.pushMatrix();

   //Bottom neck small
   ModelTrans.translate(vec3(0, 0.3, 0));
   ModelTrans.pushMatrix();

   //Bottom neck big
   ModelTrans.translate(vec3(0, 0.2, 0));
   ModelTrans.pushMatrix();
}

//Draw the base of each piece
void drawBase(int nIndices) {
   //Draw bottom neck big
   ModelTrans.scale(1.0, 0.1, 1.0);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw bottom neck small
   ModelTrans.scale(0.8, 0.2, 0.8);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw bottom neck
   ModelTrans.scale(1.3, 0.3, 1.3);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw base
   ModelTrans.scale(1.5, 0.1, 1.5);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
}

//Initialize the pawn
void setPawn(int rank, int file, int rot) {
   setBase(rank, file, rot);

   //Neck
   ModelTrans.translate(vec3(0, 0.9, 0));
   ModelTrans.pushMatrix();

   //Top Neck
   ModelTrans.translate(vec3(0, 0.9, 0));
   ModelTrans.pushMatrix();

   //Top
   ModelTrans.translate(vec3(0, 0.8, 0));
   ModelTrans.pushMatrix();

   //Moon
   float moon_rad = 1;
   float x = moon_rad * cos(moon_angle);
   float z = moon_rad * sin(moon_angle);
   ModelTrans.translate(vec3(x, 0, z));
}

//Draw all parts of the pawn
void drawPawn(int rank, int file, int material, int rot) {
   setPawn(rank, file, rot);

   //draw pawn
   SetMaterial(material);
   GLSL::enableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjSph);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Enable and bind normal array for drawing
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjSph);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Bind index array for drawing
   int nIndices = (int)sphere[0].mesh.indices.size();
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjSph);

   //Draw moon
   ModelTrans.scale(0.2, 0.2, 0.2);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw top
   ModelTrans.scale(0.8, 0.8, 0.8);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   // Enable and bind position array for drawing
   GLSL::enableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjCyl);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Enable and bind normal array for drawing
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjCyl);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Bind index array for drawing
   nIndices = (int)cylinder[0].mesh.indices.size();
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjCyl);

   //Draw top neck
   ModelTrans.scale(1.0, 0.1, 1.0);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw neck
   ModelTrans.scale(0.5, 1, 0.5);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   drawBase(nIndices);
}

//Initialize the king
void setKing(int rank, int file, int rot) {
   setBase(rank, file, rot);

   //Neck
   ModelTrans.translate(vec3(0, 0.9, 0));
   ModelTrans.pushMatrix();

   //Top Neck bottom
   ModelTrans.translate(vec3(0, 1.5, 0));
   ModelTrans.pushMatrix();

   //Top neck middle
   ModelTrans.translate(vec3(0, 0.15, 0));
   ModelTrans.pushMatrix();

   //Top neck small
   ModelTrans.translate(vec3(0, 0.15, 0));
   ModelTrans.pushMatrix();

   //Top neck ridge
   ModelTrans.translate(vec3(0, 0.15, 0));
   ModelTrans.pushMatrix();

   // Bowl
   ModelTrans.translate(vec3(0, 0.3, 0));
   ModelTrans.rotate(90, glm::vec3(1, 0, 0));
   ModelTrans.pushMatrix();

   // Top
   ModelTrans.rotate(-90, glm::vec3(1, 0, 0));
   ModelTrans.translate(vec3(0, -0.1, 0));
   ModelTrans.pushMatrix();

   // Top ball
   ModelTrans.translate(vec3(0, 0.65, 0));
   ModelTrans.pushMatrix();

   //Top cross
   ModelTrans.translate(vec3(0, 0.35, 0));
}

//Draw all parts of the king
void drawKing(int rank, int file, int material, int rot) {
   setKing(rank, file, rot);

   //draw king
   SetMaterial(material);

   GLSL::enableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjCross);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Enable and bind normal array for drawing
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjCross);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Bind index array for drawing
   int nIndices = (int)crossShape[0].mesh.indices.size();
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjCross);

   //Draw cross
   ModelTrans.scale(0.3, 0.3, 0.1);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   GLSL::enableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjSph);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Enable and bind normal array for drawing
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjSph);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Bind index array for drawing
   nIndices = (int)sphere[0].mesh.indices.size();
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjSph);

   //Draw top ball
   ModelTrans.scale(0.15, 0.15, 0.15);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw top
   ModelTrans.scale(0.6, 0.6, 0.6);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   GLSL::enableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjBowl);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Enable and bind normal array for drawing
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjBowl);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Bind index array for drawing
   nIndices = (int)bowl[0].mesh.indices.size();
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjBowl);

   //Draw bowl/crown
   ModelTrans.scale(0.8, 0.8, 0.8);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   // Enable and bind position array for drawing
   GLSL::enableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjCyl);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Enable and bind normal array for drawing
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjCyl);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Bind index array for drawing
   nIndices = (int)cylinder[0].mesh.indices.size();
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjCyl);

   //Draw top neck ridge
   ModelTrans.scale(0.7, 0.1, 0.7);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw top neck small
   ModelTrans.scale(0.6, 0.1, 0.6);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw top neck middle
   ModelTrans.scale(0.8, 0.1, 0.8);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw top neck
   ModelTrans.scale(1.0, 0.1, 1.0);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw neck
   ModelTrans.scale(0.6, 1.8, 0.6);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   drawBase(nIndices);
}

//Initialize the queen
void setQueen(int rank, int file, int rot) {
   setBase(rank, file, rot);

   //Neck
   ModelTrans.translate(vec3(0, 0.9, 0));
   ModelTrans.pushMatrix();

   //Top Neck bottom
   ModelTrans.translate(vec3(0, 1.5, 0));
   ModelTrans.pushMatrix();

   //Top neck middle
   ModelTrans.translate(vec3(0, 0.15, 0));
   ModelTrans.pushMatrix();

   //Top neck small
   ModelTrans.translate(vec3(0, 0.15, 0));
   ModelTrans.pushMatrix();

   //Top neck ridge
   ModelTrans.translate(vec3(0, 0.15, 0));
   ModelTrans.pushMatrix();

   // Crown
   ModelTrans.translate(vec3(0, 0.75, 0));
   ModelTrans.pushMatrix();

   // Top
   ModelTrans.translate(vec3(0, -0.1, 0));
   ModelTrans.pushMatrix();

   // Top ball
   ModelTrans.translate(vec3(0, 0.65, 0));
}

//Draw all parts of the queen
void drawQueen(int rank, int file, int material, int rot) {
   setQueen(rank, file, rot);

   //draw queen
   SetMaterial(material);

   GLSL::enableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjSph);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Enable and bind normal array for drawing
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjSph);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Bind index array for drawing
   int nIndices = (int)sphere[0].mesh.indices.size();
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjSph);

   //Draw top ball
   ModelTrans.scale(0.15, 0.15, 0.15);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw top
   ModelTrans.scale(0.6, 0.6, 0.6);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   // Enable and bind position array for drawing
   GLSL::enableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjCrown);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Enable and bind normal array for drawing
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjCrown);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Bind index array for drawing
   nIndices = (int)crown[0].mesh.indices.size();
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjCrown);

   //Draw crown
   ModelTrans.scale(0.8, 1.5, 0.8);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   // Enable and bind position array for drawing
   GLSL::enableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjCyl);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Enable and bind normal array for drawing
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjCyl);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Bind index array for drawing
   nIndices = (int)cylinder[0].mesh.indices.size();
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjCyl);

   //Draw top neck ridge
   ModelTrans.scale(0.7, 0.1, 0.7);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw top neck small
   ModelTrans.scale(0.6, 0.1, 0.6);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw top neck middle
   ModelTrans.scale(0.8, 0.1, 0.8);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw top neck
   ModelTrans.scale(1.0, 0.1, 1.0);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw neck
   ModelTrans.scale(0.6, 1.8, 0.6);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   drawBase(nIndices);
}

//Initialize the bishop
void setBishop(int rank, int file, int rot) {
   setBase(rank, file, rot);

   //Neck
   ModelTrans.translate(vec3(0, 0.9, 0));
   ModelTrans.pushMatrix();

   //Top Neck
   ModelTrans.translate(vec3(0, 0.9, 0));
   ModelTrans.pushMatrix();

   //Top
   ModelTrans.translate(vec3(0, 0.8, 0));
   ModelTrans.pushMatrix();

   //Top ball
   ModelTrans.translate(vec3(0, 1.5, 0));
   ModelTrans.pushMatrix();

   //Cross
   ModelTrans.translate(vec3(0, -0.6, 0.5));
}

//Draw all parts of the bishop
void drawBishop(int rank, int file, int material, int rot) {
   setBishop(rank, file, rot);

   //draw bishop
   SetMaterial(material);

   GLSL::enableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjCross);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Enable and bind normal array for drawing
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjCross);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Bind index array for drawing
   int nIndices = (int)crossShape[0].mesh.indices.size();
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjCross);

   //Draw cross
   ModelTrans.scale(0.3, 0.3, 0.1);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   GLSL::enableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjSph);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Enable and bind normal array for drawing
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjSph);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Bind index array for drawing
   nIndices = (int)sphere[0].mesh.indices.size();
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjSph);

   //Draw top ball
   ModelTrans.scale(0.15, 0.15, 0.15);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw top
   ModelTrans.scale(0.6, 1.5, 0.6);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   // Enable and bind position array for drawing
   GLSL::enableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjCyl);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Enable and bind normal array for drawing
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjCyl);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Bind index array for drawing
   nIndices = (int)cylinder[0].mesh.indices.size();
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjCyl);

   //Draw top neck
   ModelTrans.scale(1.0, 0.1, 1.0);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw neck
   ModelTrans.scale(0.5, 1, 0.5);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   drawBase(nIndices);
}

//Initialize the knight
void setKnight(int rank, int file, int rot) {
   setBase(rank, file, rot);

   //Neck
   ModelTrans.translate(vec3(0, 0.9, 0));
   ModelTrans.pushMatrix();

   //Horse
   ModelTrans.rotate(-15, glm::vec3(1, 0, 0));
   ModelTrans.translate(vec3(0, 1.5, 1));
}

//Draw all parts of the knight
void drawKnight(int rank, int file, int material, int rot) {
   setKnight(rank, file, rot);

   //draw knight
   SetMaterial(material);
   GLSL::enableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjHor);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Enable and bind normal array for drawing
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjHor);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Bind index array for drawing
   int nIndices = (int)horse[0].mesh.indices.size();
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjHor);

   //Draw top
   ModelTrans.scale(2, 2, 2);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   // Enable and bind position array for drawing
   GLSL::enableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjCyl);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Enable and bind normal array for drawing
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjCyl);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Bind index array for drawing
   nIndices = (int)cylinder[0].mesh.indices.size();
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjCyl);

   //Draw neck
   ModelTrans.scale(0.7, 1, 0.7);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   drawBase(nIndices);
}

//Initialize the rook
void setRook(int rank, int file) {
   //create the model transforms 
   ModelTrans.loadIdentity();
   ModelTrans.translate(vec3(piece_positions[rank][file].x, piece_positions[rank][file].y, piece_positions[rank][file].z));
   ModelTrans.pushMatrix();

   //Bottom neck
   ModelTrans.translate(vec3(0, 0.3, 0));
   ModelTrans.pushMatrix();

   //Neck
   ModelTrans.translate(vec3(0, 0.9, 0));
   ModelTrans.pushMatrix();

   //Top Neck
   ModelTrans.translate(vec3(0, 1.3, 0));
   ModelTrans.pushMatrix();

   //Crown
   ModelTrans.translate(vec3(0, 0.4, 0));
}

//Draw all parts of the rook
void drawRook(int rank, int file, int material) {
   setRook(rank, file);

   //draw rook
   SetMaterial(material);

   GLSL::enableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjCrown);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Enable and bind normal array for drawing
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjCrown);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Bind index array for drawing
   int nIndices = (int)crown[0].mesh.indices.size();
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjCrown);

   ModelTrans.scale(1.2, 1.5, 1.2);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   // Enable and bind position array for drawing
   GLSL::enableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjCyl);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Enable and bind normal array for drawing
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjCyl);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Bind index array for drawing
   nIndices = (int)cylinder[0].mesh.indices.size();
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjCyl);

   //Draw top neck
   ModelTrans.scale(1.2, 0.4, 1.2);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw neck
   ModelTrans.scale(1, 1.5, 1);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw bottom neck
   ModelTrans.scale(1.3, 0.3, 1.3);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
   ModelTrans.popMatrix();

   //Draw base
   ModelTrans.scale(1.5, 0.1, 1.5);
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
}

void drawGL() {
   // Clear the screen
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Use our GLSL program
   glUseProgram(ShadeProg);

   SetProjectionMatrix();
   SetView();

   if (line) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   }
   else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   }


   glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);

   //drawGround();
   drawBoard();

   //draw pieces
   for (int i = 0; i < gamePieces.size(); i++) {
      gamePieces[i].checkPosition();
      gamePieces[i].drawPiece();
   }

   moon_angle += 0.01;

   // Disable and unbind
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   glUseProgram(0);
   assert(glGetError() == GL_NO_ERROR);
}

void window_size_callback(GLFWwindow* window, int w, int h) {
   glViewport(0, 0, (GLsizei)w, (GLsizei)h);
   g_width = w;
   g_height = h;
}

void mouse_callback(GLFWwindow* window, int button, int action, int mods) {
   if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
      glfwGetCursorPos(window, &mouseX, &mouseY);
      //cout << "mouse clicked at x: " << mouseX << "y: " << mouseY;
      mouseClick();
   }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
   //SPACE BAR - changes the view between sides
   if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
         nextTurn();
   }
   //F KEY - Put the camera in free cam mode or take it out of free cam mode
   if (key == GLFW_KEY_F && action == GLFW_PRESS) {
      free_cam = !free_cam;
   }
   //O KEY - Put the camera in an overhead position
   if (key == GLFW_KEY_O && action == GLFW_PRESS) {
      g_camera_id = (g_camera_id + 1) % 2;
      SetCamera(2);
      free_cam = 0;
   }
   //R KEY - Restart the game
   if (key == GLFW_KEY_R && action == GLFW_PRESS) {
      initPieces();
   }
   //If there is a piece selected
   if (selectedPiece != -1) {
      //UP ARROW / NUM PAD UP - Move the selected piece up one
      if ((key == GLFW_KEY_UP || key == GLFW_KEY_KP_8) && action == GLFW_RELEASE) {
         //Up is relative to the side
         if (gamePieces[selectedPiece].material == WHITE)
            gamePieces[selectedPiece].setFile(gamePieces[selectedPiece].file + 1);
         else
            gamePieces[selectedPiece].setFile(gamePieces[selectedPiece].file - 1);
      }
      //DOWN ARROW / NUM PAD DOWN - Move the selected piece back one
      if ((key == GLFW_KEY_DOWN || key == GLFW_KEY_KP_2) && action == GLFW_RELEASE) {
         if (gamePieces[selectedPiece].material == WHITE)
            gamePieces[selectedPiece].setFile(gamePieces[selectedPiece].file - 1);
         else
            gamePieces[selectedPiece].setFile(gamePieces[selectedPiece].file + 1);
      }
      //LEFT ARROW / NUM PAD LEFT - Move the selected piece left one
      if ((key == GLFW_KEY_LEFT || key == GLFW_KEY_KP_4) && action == GLFW_RELEASE) {
         if (gamePieces[selectedPiece].material == WHITE)
            gamePieces[selectedPiece].setRank(gamePieces[selectedPiece].rank + 1);
         else
            gamePieces[selectedPiece].setRank(gamePieces[selectedPiece].rank - 1);
      }
      //RIGHT ARROW / NUM PAD RIGHT - Move the selected piece right one
      if ((key == GLFW_KEY_RIGHT || key == GLFW_KEY_KP_6) && action == GLFW_RELEASE) {
         if (gamePieces[selectedPiece].material == WHITE)
            gamePieces[selectedPiece].setRank(gamePieces[selectedPiece].rank - 1);
         else
            gamePieces[selectedPiece].setRank(gamePieces[selectedPiece].rank + 1);
      }
      //NUM PAD 7 (UP AND LEFT) - Move the selected piece up one and left one
      if (key == GLFW_KEY_KP_7 && action == GLFW_RELEASE) {
         if (gamePieces[selectedPiece].material == WHITE) {
            gamePieces[selectedPiece].setRank(gamePieces[selectedPiece].rank + 1);
            gamePieces[selectedPiece].setFile(gamePieces[selectedPiece].file + 1);
         }
         else {
            gamePieces[selectedPiece].setRank(gamePieces[selectedPiece].rank - 1);
            gamePieces[selectedPiece].setFile(gamePieces[selectedPiece].file - 1);
         }
      }
      //NUM PAD 3 (DOWN AND RIGHT) - Move the selected piece back one and right one
      if (key == GLFW_KEY_KP_3 && action == GLFW_RELEASE) {
         if (gamePieces[selectedPiece].material == WHITE) {
            gamePieces[selectedPiece].setRank(gamePieces[selectedPiece].rank - 1);
            gamePieces[selectedPiece].setFile(gamePieces[selectedPiece].file - 1);
         }
         else {
            gamePieces[selectedPiece].setRank(gamePieces[selectedPiece].rank + 1);
            gamePieces[selectedPiece].setFile(gamePieces[selectedPiece].file + 1);
         }
      }
      //NUM PAD 9 (UP AND RIGHT) - Move the selected piece up one and right one
      if (key == GLFW_KEY_KP_9 && action == GLFW_RELEASE) {
         if (gamePieces[selectedPiece].material == WHITE) {
            gamePieces[selectedPiece].setRank(gamePieces[selectedPiece].rank - 1);
            gamePieces[selectedPiece].setFile(gamePieces[selectedPiece].file + 1);
         }
         else {
            gamePieces[selectedPiece].setRank(gamePieces[selectedPiece].rank + 1);
            gamePieces[selectedPiece].setFile(gamePieces[selectedPiece].file - 1);
         }
      }
      //NUM PAD 1 (DOWN AND LEFT) - Move the selected piece down one and left one
      if (key == GLFW_KEY_KP_1 && action == GLFW_RELEASE) {
         if (gamePieces[selectedPiece].material == WHITE) {
            gamePieces[selectedPiece].setRank(gamePieces[selectedPiece].rank + 1);
            gamePieces[selectedPiece].setFile(gamePieces[selectedPiece].file - 1);
         }
         else {
            gamePieces[selectedPiece].setRank(gamePieces[selectedPiece].rank - 1);
            gamePieces[selectedPiece].setFile(gamePieces[selectedPiece].file + 1);
         }
      }
      if (key == GLFW_KEY_L && action == GLFW_RELEASE) {
         line = !line;

      }
   }
}

int main(int argc, char **argv) {

   // Initialise GLFW
   if (!glfwInit())
   {
      fprintf(stderr, "Failed to initialize GLFW\n");
      return -1;
   }

   glfwWindowHint(GLFW_SAMPLES, 4);
   glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

   // Open a window and create its OpenGL context
   g_width = 1024;
   g_height = 768;

   window = glfwCreateWindow(g_width, g_height, "Chess", NULL, NULL);
   if (window == NULL){
      fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
      glfwTerminate();
      return -1;
   }
   glfwMakeContextCurrent(window);
   glfwSetKeyCallback(window, key_callback);
   glfwSetMouseButtonCallback(window, mouse_callback);
   glfwSetWindowSizeCallback(window, window_size_callback);
   loadShapes("cube.obj", cube);
   loadShapes("cylinder.obj", cylinder);
   loadShapes("sphere.obj", sphere);
   loadShapes("horse.obj", horse);
   loadShapes("cross.obj", crossShape);
   loadShapes("crown.obj", crown);
   loadShapes("bowl.obj", bowl);

   // Initialize glad
   if (!gladLoadGL()) {
      fprintf(stderr, "Unable to initialize glad");
      glfwDestroyWindow(window);
      glfwTerminate();
      return 1;
   }

   // Ensure we can capture the escape key being pressed below
   glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   initGL();
   installShaders("vert.glsl", "frag.glsl");

   do{
      drawGL();
      // Swap buffers
      glfwSwapBuffers(window);
      glfwPollEvents();
   } // Check if the ESC key was pressed or the window was closed
   while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
   glfwWindowShouldClose(window) == 0);

   // Close OpenGL window and terminate GLFW
   glfwTerminate();

   return 0;
}
