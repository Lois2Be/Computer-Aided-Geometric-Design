#ifndef HYPERBOLICCOMPOSITEPATCH3_H
#define HYPERBOLICCOMPOSITEPATCH3_H


#include "HyperbolicPatch3.h"
#include "../Core/TensorProductSurfaces3.h"
#include "../Core/Colors4.h"
#include "../Core/Materials.h"
#include <string.h>
#include <vector>

using namespace std;

namespace cagd {
  class HyperbolicCompositePatch3 {
  public:
    enum Direction{N=0,NE=1,E=2,SE=3,S=4,SW=5,W=6,NW=7};
    static const GLuint div_point_count = 100;
    constexpr static const GLdouble derivative_scale = 0.3;
    Color4 default_derivatives_colour;

    class PatchAttributes{
    public:
        HyperbolicPatch3 * patch;
        TriangulatedMesh3 * img;
        Material  material;
        Color4 * derivatives_color;
        PatchAttributes* neighbours[8];
        PatchAttributes():patch(0),img(0),material(MatFBEmerald){
          memset(neighbours,0,8*sizeof(PatchAttributes*));
        }
        PatchAttributes(const PatchAttributes & other);
        PatchAttributes& operator=(const PatchAttributes & other);
        ~PatchAttributes();

        GLboolean generateImage(GLuint max_order_of_derivatives){
          img = patch->GenerateImage(div_point_count,div_point_count);
          return img != 0;
        }
        GLboolean updateVBO(){
           if(!img)return GL_FALSE;
           return img->UpdateVertexBufferObjects();
        }
    };

  protected:
    vector<PatchAttributes*> _patches;
    GLuint _patch_count;
  public:
    int getSize(){return _patch_count;}
    PatchAttributes* getPatch(int index){return _patches[index];}
    HyperbolicCompositePatch3(GLuint max_curve_count):_patches(max_curve_count),_patch_count(0){
      default_derivatives_colour=Color4(0,0.5,0);
    }
    GLboolean insert(GLdouble alpha,GLuint max_order_of_derivatives,const ColumnMatrix<DCoordinate3>& _data,Material material=MatFBEmerald);
    GLboolean continueExisting(GLuint id,Direction direction,GLdouble alpha);
//    GLuint join(GLuint firstId, GLuint SecondID,Direction firstDirection,Direction secondDirection,GLdouble scale);
//    GLuint merge(GLuint firstId, GLuint SecondID,Direction firstDirection,Direction secondDirection);
//    GLboolean updatePosition(int arcindex,int pointindex,DCoordinate3 newcoord);
//    GLboolean updateArcForRendering( PatchAttributes*);
    void renderAll();
    ~HyperbolicCompositePatch3();
  };
}

#endif // HYPERBOLICCOMPOSITEPATCH3_H