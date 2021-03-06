#ifndef HYPERBOLICCOMPOSITEPATCH3_H
#define HYPERBOLICCOMPOSITEPATCH3_H


#include "HyperbolicPatch3.h"
#include "../Core/TensorProductSurfaces3.h"
#include "../Core/Colors4.h"
#include "../Core/Materials.h"
#include "./IndicatingSphere.h"
#include <string.h>
#include <vector>
#include <fstream>
#include "../Core/Texture/FreeImage.h"

using namespace std;

namespace cagd {
  class HyperbolicCompositePatch3 {
  public:
    enum Direction{N=0,NE=1,E=2,SE=3,S=4,SW=5,W=6,NW=7};
    static const GLuint div_point_count = 20;
    constexpr static const GLdouble derivative_scale = 0.3;
    Color4 default_derivatives_colour;
    IndicatingSphere * sphere;
    GenericCurve3* selectedPatchBorderCurve;

    class PatchAttributes{
    public:

        HyperbolicPatch3 * patch;
        TriangulatedMesh3 * img;
        GLboolean renderTexture;
        string currentTextureFilename;
        FIBITMAP * textureContent;
        BYTE * textureData;
        Material  material;
        Color4 * derivatives_color;
        PatchAttributes* neighbours[8];

        RowMatrix<GenericCurve3*>* ulines;
        RowMatrix<GenericCurve3*>* vlines;

        PatchAttributes():patch(0),img(0),material(MatFBEmerald),renderTexture(false),textureContent(0),textureData(0){
          memset(neighbours,0,8*sizeof(PatchAttributes*));
          ulines=0;
          vlines=0;
        }
        PatchAttributes(const PatchAttributes & other);
        PatchAttributes& operator=(const PatchAttributes & other);
        ~PatchAttributes();

        GLboolean generateImage(){
          if(img)delete img;
          img = patch->GenerateImage(div_point_count,div_point_count);
          if(renderTexture && img && textureContent && textureData){
              img->bindTextureImage(textureContent,textureData);
            }
          return img != 0;
        }

        void generatUIsoparametricLines(int line_count){
          if(!patch){
              cerr<<"Error, patch is null"<<endl;return;
            }
          if(ulines)delete ulines;
          ulines=patch->GenerateUIsoparametricLines(line_count,1,100);
          if(!ulines){
              cerr<<"Error, ulines is null"<<endl;return;
            }
          for (int i=0;i<line_count;++i) {
              ((*ulines)[i])->UpdateVertexBufferObjects(0.4);
            }
        }

        void generatVIsoparametricLines(int line_count){
          if(!patch){
              cerr<<"Error, patch is null"<<endl;return;
            }
          if(vlines)delete vlines;
          vlines=patch->GenerateVIsoparametricLines(line_count,1,100);
          for (int i=0;i<line_count;++i) {
              ((*vlines)[i])->UpdateVertexBufferObjects(0.4);
            }
        }
        void clearULines(){
          if(ulines){
              for (int i=0;i<ulines->GetColumnCount();++i) {
                  delete (*ulines)[i];
                }
              delete ulines;
              ulines=0;
            }
        }
        void clearVLines(){
          if(vlines){
              for (int i=0;i<vlines->GetColumnCount();++i) {
                  delete (*vlines)[i];
                }
              delete vlines;
              vlines=0;
            }
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
    void clear(){
      for (int i=0;i<_patch_count;++i) {
          delete _patches[i];
          _patches[i]=0;
        }
      _patch_count=0;
      if(selectedPatchBorderCurve){
          delete selectedPatchBorderCurve;
        selectedPatchBorderCurve=0;
      }
      if(sphere->_image){
          delete sphere->_image;
          sphere->_image=0;
        }
    }    
    DCoordinate3 getCoord(int i, int j, int index){
      DCoordinate3 coord;
      _patches[index]->patch->GetData(i,j,coord);
      return coord;
    }
    int getSize(){return _patch_count;}
    PatchAttributes* getPatch(int index){return _patches[index];}
    void updateSpherePosition(GLuint row,GLuint column,GLuint patchIndex){
      if(patchIndex >= _patch_count){
          cerr<<"Error: invalid patch index selected"<<endl;
          return;
        }
      if(row>=4|| column >=4){
          cerr<<"Error: invalid row, or column selected"<<endl;
          return;
        }
      DCoordinate3 pointCurrentPosition;
      (_patches[patchIndex]->patch)->GetData(row,column,pointCurrentPosition);
      sphere->updateImage(pointCurrentPosition);
    }
    void updateSelectionCurve(GLuint patchIndex, Direction selectedDirection){
      if(patchIndex>=_patch_count || selectedDirection == NE || selectedDirection == NW ||selectedDirection == SE ||selectedDirection == SW){
          cerr<<"Error: patch index out of bounds, or wrong direction"<<endl;
          return;
        }

      if(selectedPatchBorderCurve)delete  selectedPatchBorderCurve;
      selectedPatchBorderCurve = 0;
      switch (selectedDirection) {
        case N:{
          RowMatrix<GenericCurve3*>* isocurves = _patches[patchIndex]->patch->GenerateUIsoparametricLines(2,0,100);
          selectedPatchBorderCurve = (*isocurves)[0];
          if(!selectedPatchBorderCurve){
              cerr<<"Error generating isoparametric border line "<<endl;
              return;
          }
          if(!selectedPatchBorderCurve->UpdateVertexBufferObjects()){
              cerr<<"Error updating vbo of selectedPatchBorderCurve"<<endl;
              return;
            }
          delete (*isocurves)[1];
          }break;
        case S:{
          RowMatrix<GenericCurve3*>* isocurves = _patches[patchIndex]->patch->GenerateUIsoparametricLines(2,0,100);
          selectedPatchBorderCurve = (*isocurves)[1];
          if(!selectedPatchBorderCurve){
              cerr<<"Error generating isoparametric border line "<<endl;
              return;
          }
          if(!selectedPatchBorderCurve->UpdateVertexBufferObjects()){
              cerr<<"Error updating vbo of selectedPatchBorderCurve"<<endl;
              return;
            }
          delete (*isocurves)[0];
          }break;
        case W:{
          RowMatrix<GenericCurve3*>* isocurves = _patches[patchIndex]->patch->GenerateVIsoparametricLines(2,0,100);
          selectedPatchBorderCurve = (*isocurves)[0];
          if(!selectedPatchBorderCurve){
              cerr<<"Error generating isoparametric border line "<<endl;
              return;
          }
          if(!selectedPatchBorderCurve->UpdateVertexBufferObjects()){
              cerr<<"Error updating vbo of selectedPatchBorderCurve"<<endl;
              return;
            }
          delete (*isocurves)[1];
          }break;
        case E:{
          RowMatrix<GenericCurve3*>* isocurves = _patches[patchIndex]->patch->GenerateVIsoparametricLines(2,0,100);
          selectedPatchBorderCurve = (*isocurves)[1];
          if(!selectedPatchBorderCurve){
              cerr<<"Error generating isoparametric border line "<<endl;
              return;
          }
          if(!selectedPatchBorderCurve->UpdateVertexBufferObjects()){
              cerr<<"Error updating vbo of selectedPatchBorderCurve"<<endl;
              return;
            }
          delete (*isocurves)[0];
          }break;
        }
    }
    void clearSelectionCurve(){
      if(selectedPatchBorderCurve)delete selectedPatchBorderCurve;
      selectedPatchBorderCurve = 0;
    }
    HyperbolicCompositePatch3(GLuint max_curve_count):_patches(max_curve_count),_patch_count(0),selectedPatchBorderCurve(0){
      default_derivatives_colour=Color4(0,0.5,0);
      sphere = new IndicatingSphere(0.02);
    }

    GLboolean insert(GLdouble alpha,GLuint max_order_of_derivatives,const ColumnMatrix<DCoordinate3>& _data,Material material=MatFBEmerald);
    GLboolean continueExisting(GLuint id,Direction direction,GLdouble alpha,Material material);
    GLuint join(GLuint firstId, GLuint SecondID,Direction firstDirection,Direction secondDirection);
    GLuint merge(GLuint firstId, GLuint SecondID,Direction firstDirection,Direction secondDirection);
    GLboolean update(int i,int j,int patchindex,DCoordinate3 newcoord);
    GLboolean updatePatchForRendering( PatchAttributes*);

    void setULines(int patchIndex,int lineCount){
      if(patchIndex<0 || patchIndex>=_patch_count){
          cerr<<"Invalid patch index"<<endl;
          return;
        }
      if(lineCount<=0){
          cerr<<"Invalid line count"<<endl;
          return;
        }
      _patches[patchIndex]->generatUIsoparametricLines(lineCount);
    }
    void setVLines(int patchIndex,int lineCount){
      if(patchIndex<0 || patchIndex>=_patch_count){
          cerr<<"Invalid patch index"<<endl;
          return;
        }
      if(lineCount<=0){
          cerr<<"Invalid line count"<<endl;
          return;
        }
      _patches[patchIndex]->generatVIsoparametricLines(lineCount);
    }
    void clearULines(int patchIndex){
      if(patchIndex<0 || patchIndex>=_patch_count){
          cerr<<"Invalid patch index"<<endl;
          return;
        }
      _patches[patchIndex]->clearULines();
    }
    void clearVLines(int patchIndex){
      if(patchIndex<0 || patchIndex>=_patch_count){
          cerr<<"Invalid patch index"<<endl;
          return;
        }
      _patches[patchIndex]->clearVLines();
    }
    void renderAll();
    ~HyperbolicCompositePatch3();
    int getNeighbourIndex(PatchAttributes* neighbour){
      for (int i=0;i<_patch_count;++i) {
          if(_patches[i] == neighbour)
            return i;
        }
      return -1;
    }
    void saveToFile(string filename){
      ofstream file;
      file.open((filename).c_str());
      if(!file.is_open()){
          cerr<<"error creating "<<filename<<endl;
          return;
        }
      file<<_patch_count<<endl;
      DCoordinate3 controlPoint;
      for (int t = 0; t < _patch_count; ++t) {
          for (int i=0;i<4;++i) {
            for (int j=0;j<4;++j) {
                _patches[t]->patch->GetData(i,j,controlPoint);
              file<<controlPoint;
              file<<endl;
            }
          }
      }

      for (int i = 0; i < _patch_count; ++i) {
          for(int n=0; n < 8; n++){
            file<<i<<" "<<getNeighbourIndex(_patches[i]->neighbours[n])<<endl;
          }
      }
      file.close();
      cout<<"File"<<filename<<"saved"<<endl;
    }

    void readFromFile(string filename){
      cout<<"reading from"<<filename<<endl;
      ifstream file;
      file.open((filename).c_str());
      if(!file.is_open()){
        cerr<<filename<<" could not be opened"<<endl;
        return;
        }
      int was = _patch_count;
      _patch_count=0;
      for(int i=0;i<was;++i){
          if(_patches[i])
            delete _patches[i];
      }
      int num_of_patches;
      file>>num_of_patches;
      cout<<num_of_patches<<endl;
      ColumnMatrix<DCoordinate3> points(16);
      for (int i = 0; i < num_of_patches; ++i) {
          for(int j=0;j<16;++j){
          file>>points[j];
          cout<<points[j]<<endl;
          }
          insert(5,1,points);
      }

      int index,neighboureIndex;
      for (int i = 0; i < num_of_patches; ++i) {
        for(int n=0; n < 8; n++){
          file>>index>>neighboureIndex;
          if(neighboureIndex!=-1){
            _patches[index]->neighbours[n]=_patches[neighboureIndex];
          }
        }
      }
    }
    FREE_IMAGE_FORMAT GetFileFormat(const char * filename);
    FIBITMAP * GetFileContent(FREE_IMAGE_FORMAT fif, const char * filename);

    void loadTextureData(GLuint patchIndex,string filename){
      if(_patches[patchIndex]->textureContent){
         FreeImage_Unload(_patches[patchIndex]->textureContent);
        }
      if(patchIndex >= _patch_count){
          cerr<<"Error in applying texture, invalid index"<<endl;
          return;
        }
      FREE_IMAGE_FORMAT ext = GetFileFormat(filename.c_str());
        if (ext == FIF_UNKNOWN) {
                cerr<<"Invalid Texture File Format!"<<endl;
                return;
        }
        _patches[patchIndex]->textureContent = GetFileContent(ext, filename.c_str());
        if (!_patches[patchIndex]->textureContent) {
                throw("Invalid Texture File Content!");
        }
     _patches[patchIndex]->textureData = FreeImage_GetBits(_patches[patchIndex]->textureContent);
    }

    GLboolean applyTexture(GLuint patchIndex, std::string filename){
      if(patchIndex >= _patch_count){
          cerr<<"Error in applying texture, invalid index"<<endl;
          return GL_FALSE;
        }
      ifstream file;
      file.open(filename.c_str());
      if(!file.is_open()){
          cerr<<filename<<" could not be read"<<endl;
          file.close();
          return GL_FALSE;
      }
      file.close();
      if(_patches[patchIndex]->img){
          loadTextureData(patchIndex,filename);
          if(!_patches[patchIndex]->img->bindTextureImage(_patches[patchIndex]->textureContent,_patches[patchIndex]->textureData)){
              return GL_FALSE;
            }
          _patches[patchIndex]->renderTexture=true;
          _patches[patchIndex]->currentTextureFilename=filename;
      }else{
          cerr<<"Error in applying texture, invalid img pointer"<<endl;
          return GL_FALSE;
        }
    }
    void disableTexture(GLuint patchIndex){
      if(patchIndex >= _patch_count){
          cerr<<"Error in applying texture, invalid index"<<endl;
          return;
      }
      _patches[patchIndex]->renderTexture=false;
    }
  };
}

#endif // HYPERBOLICCOMPOSITEPATCH3_H
