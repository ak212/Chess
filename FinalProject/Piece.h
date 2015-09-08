#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cassert>
#include <cmath>
#include <stdio.h>
#include "GLSL.h"
#include "tiny_obj_loader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "RenderingHelper.h"

#include "tiny_obj_loader.h"

using namespace glm;
using namespace std;
using namespace tinyobj;

void drawPawn(int rank, int file, int material, int rot);
void drawKing(int rank, int file, int material, int rot);
void drawQueen(int rank, int file, int material, int rot);
void drawBishop(int rank, int file, int material, int rot);
void drawKnight(int rank, int file, int material, int rot);
void drawRook(int rank, int file, int material);

//Class to represent a game piece
class Piece {
public:
   char* type; //what kind of piece it is (pawn, rook, etc.)
   int rank;
   int file;
   int material; //black or white
   int rotation;
   int draw; //whether or not to draw the piece
   int selected; //whether or not the piece is selected

   Piece();
   Piece(char* t, int r, int f, int m, int rot, int d);
   void setRank(int r);
   void setFile(int f);
   int getRank();
   int getFile();
   void drawPiece();   //Draw the piece based on the type
   void checkPosition();    //Make sure the piece is on the board
   void print();
   void resize_obj(vector<shape_t> &shapes);
   void loadShapes(const string &objFile, vector<shape_t>& shapes);
   vector<float> getNorBuf(vector<shape_t>& shapes);
};
