﻿#include "GLWidget.h"
#include <GL/glu.h>
#include <iostream>
//mine
#include "Core/Matrices.h"
#include "Test/TestFunctions.h"
#include "../Core/Materials.h"
#include "../Core/Constants.h"
#include "../Parametric/ParametricSurfaces3.h"
#include "../Hyperbolic/HyperbolicArc3.h"
//eof mine
using namespace std;

#include <Core/Exceptions.h>
#define MAX_CURVE_COUNT 20
#define MAX_PATCH_COUNT 15
namespace cagd
{
    //--------------------------------
    // special and default constructor
    //--------------------------------
    GLWidget::GLWidget(QWidget *parent, const QGLFormat &format): QGLWidget(format, parent),surfaces(5),curves(5),derivative_scale(0.3),compositeCurve(0),compositePatch(0)
    {   //Parametric curve
//      switch(currentHomework){
//        case ParametricCurve:initialize_curves();break;
//        case DynamicVBO:{
//              _timer = new QTimer(this);
//              _timer->setInterval(0);
//              connect(_timer,SIGNAL(timeout()),this,SLOT(_animate()));
//          }break;
//        case ParametricSurface:{
//            initialize_surfaces();
//          }break;
//        case HyperbolicArc3:{

//          }break;
//      }

        for(GLuint i = 0; i < 4; ++i) {
            _shader[i] = new ShaderProgram();
        }

    initialize_curves();
    _timer = new QTimer(this);
                  _timer->setInterval(0);
                  connect(_timer,SIGNAL(timeout()),this,SLOT(_animate()));
    initialize_surfaces();
    }

    //mine
    GLWidget::~GLWidget(){
      if(_pc){
        delete _pc;
        _pc=nullptr;
        }
      if(_image_of_pc){
        delete _image_of_pc;
          _image_of_pc=nullptr;
        }      
      if(curves[0]){
          for(int i=0;i<curves.GetColumnCount();i++){
              delete curves[i];
            }
        }
      if(surfaces[0]){
          for(int i=0;i<surfaces.GetColumnCount();i++){
              delete surfaces[i];
            }
        }
      if(hyperbolicArc3)
        delete hyperbolicArc3;
      if(_hyperbolicArc3Image)
        delete _hyperbolicArc3Image;

      if(tensorSurface3DataGrid)
        delete tensorSurface3DataGrid;
      if(hyperbolicPatch3Image)
        delete hyperbolicPatch3Image;
      if(hyperbolicPatch3InterpolationImage)
        delete hyperbolicPatch3InterpolationImage;
      if(cyclicCurve3)
        delete cyclicCurve3;
      if(_cyclicCurve3Image)
        delete _cyclicCurve3Image;
      if(_cyclicCurve3InterpolationImage)
        delete _cyclicCurve3InterpolationImage;
      if(UISOLINES){
        for(int i=0;i<UISOLINES->GetColumnCount();i++){
          delete (*UISOLINES)[i];
        }
        delete UISOLINES;
      }
      if(VISOLINES){
        for(int i=0;i<VISOLINES->GetColumnCount();i++){
          delete (*VISOLINES)[i];
        }
        delete VISOLINES;
      }
      if(compositeCurve){
          delete  compositeCurve;
        }
    }
    //eof mine
    //--------------------------------------------------------------------------------------
    // this virtual function is called once before the first call to paintGL() or resizeGL()
    //--------------------------------------------------------------------------------------
    void GLWidget::initializeGL()
    {
        // creating a perspective projection matrix
        glMatrixMode(GL_PROJECTION);

        glLoadIdentity();

        _aspect = (float)width() / (float)height();
        _z_near = 1.0;
        _z_far = 1000.0;
        _fovy = 45.0;

        gluPerspective(_fovy, _aspect, _z_near, _z_far);

        // setting the model view matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        _eye[0] = _eye[1] = 0.0, _eye[2] = 6.0;
        _center[0] = _center[1] = _center[2] = 0.0;
        _up[0] = _up[2] = 0.0, _up[1] = 1.0;

        gluLookAt(_eye[0], _eye[1], _eye[2], _center[0], _center[1], _center[2], _up[0], _up[1], _up[2]);

        // enabling the depth test
        glEnable(GL_DEPTH_TEST);

        // setting the background color
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // initial values of transformation parameters
        _angle_x = _angle_y = _angle_z = 0.0;
        _trans_x = _trans_y = _trans_z = 0.0;
        _zoom = 1.0;

        try
        {
            //mine
            glEnable(GL_POINT_SMOOTH);
            glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);
            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
            glEnable(GL_POLYGON_SMOOTH);
            glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
            glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
            glEnable(GL_DEPTH_TEST);
            //testing vbo
           // if(currentHomework == DynamicVBO || currentHomework == ParametricSurface){
              glEnable(GL_LIGHTING);
              glEnable(GL_LIGHT0);
              glEnable(GL_NORMALIZE);
           // }
            //eof mine
            // initializing the OpenGL Extension Wrangler library
            GLenum error = glewInit();

            if (error != GLEW_OK)
            {
                throw Exception("Could not initialize the OpenGL Extension Wrangler Library!");
            }

            if (!glewIsSupported("GL_VERSION_2_0"))
            {
                throw Exception("Your graphics card is not compatible with OpenGL 2.0+! "
                                "Try to update your driver or buy a new graphics adapter!");
            }

            // create and store your geometry in display lists or vertex buffer objects
            // ...
            //Parametric Curves
          //  mine
            //if(currentHomework == ParametricCurve){
                RowMatrix<ParametricCurve3::Derivative> derivative(3);
                derivative(0)=rose::d0;
                derivative(1)=rose::d1;
                derivative(2)=rose::d2;

                _pc=nullptr;
                _pc=new (nothrow) ParametricCurve3(derivative,rose::u_min,rose::u_max);

                if(!_pc){
                    cout<<"Error in GLWidget, parametric curve"<<endl;
                 }
                GLuint div_point_count = 200;
                GLenum usage_flag = GL_STATIC_DRAW;

                _image_of_pc = nullptr;
                _image_of_pc = _pc->GenerateImage(div_point_count,usage_flag);

                if(!_image_of_pc){
                    cout<<"Error in GLWidget, parametric curve, image of pc"<<endl;
                 }
                if(!_image_of_pc->UpdateVertexBufferObjects(0.2,usage_flag)){
                    cout<<"could not create vertex buffer object of the parametric curve"<<endl;
                }
            //}

            //Testing dynamic vertex buffer objects
            if(currentHomework == DynamicVBO){
                if(_mouse.LoadFromOFF("/home/nandor/egyetem4/cs-bsc-ii-2/Grafika/build-QtFramework-Desktop_Qt_5_12_1_GCC_64bit-Debug/Models/elephant.off",true)){//miert nem megy relativ pathel ?
                  if(_mouse.UpdateVertexBufferObjects((GL_DYNAMIC_DRAW))){
                      _angle=0.0;
                      _timer->start();
                      cout<<"started timer"<<endl;
                    }

                  }else{
                    cout<<"could not read off"<<endl;
                  }
            }
            //Testing Parametric surface
            //if(currentHomework == ParametricSurface){
                TriangularMatrix<ParametricSurface3::PartialDerivative> derivatives(3);
                derivatives(0,0)=surface1::d00;
                derivatives(1,0)=surface1::d10;
                derivatives(1,1)=surface1::d01;

                ParametricSurface3 surface(derivatives,surface1::u_min,surface1::u_max,surface1::v_min,surface1::v_max);
                _surface = *(surface.GenerateImage(200,200,true));
                              if(_surface.UpdateVertexBufferObjects((GL_DYNAMIC_DRAW))){
    //                              _angle=0.0;
    //                              _timer->start();
                                }
           // }
           // if(currentHomework == HyperbolicArc3){

              hyperbolicArc3 = new (nothrow)   class HyperbolicArc3(4);
              (*hyperbolicArc3)[0]=DCoordinate3(0,0,0);
              (*hyperbolicArc3)[1]=DCoordinate3(0,1,0);
              (*hyperbolicArc3)[2]=DCoordinate3(0,1,1);
              (*hyperbolicArc3)[3]=DCoordinate3(1,1,1);
              hyperbolicArc3->UpdateVertexBufferObjectsOfData();
              _hyperbolicArc3Image =hyperbolicArc3->GenerateImage(2,200);
              if(!_hyperbolicArc3Image){
                  cout<<"error";
                }
              _hyperbolicArc3Image->UpdateVertexBufferObjects(derivative_scale,GL_STATIC_DRAW);
           // }
            //CyclicCurve3
              int cyclicCurvePoints = 6;
              cyclicCurve3 = new CyclicCurve3(cyclicCurvePoints/2);
              (*cyclicCurve3)[0]=DCoordinate3(0,1,0);
              (*cyclicCurve3)[1]=DCoordinate3(0,0.5,0.5*sqrt(3));
              (*cyclicCurve3)[2]=DCoordinate3(0,-0.5,0.5*sqrt(3));
              (*cyclicCurve3)[3]=DCoordinate3(0,-1,0);
              (*cyclicCurve3)[4]=DCoordinate3(1,-0.5,-0.5*sqrt(3));
              (*cyclicCurve3)[5]=DCoordinate3(1,0.5,-0.5*sqrt(3));
              (*cyclicCurve3)[6]=(*cyclicCurve3)[0];
              cyclicCurve3->UpdateVertexBufferObjectsOfData();
              _cyclicCurve3Image =cyclicCurve3->GenerateImage(2,200);
              if(!_cyclicCurve3Image){
                  cout<<"error";
                }
              _cyclicCurve3Image->UpdateVertexBufferObjects(derivative_scale,GL_STATIC_DRAW);
              //let us interpolate
              ColumnMatrix<DCoordinate3> cyclicCurveDataPointsToInterpolate(cyclicCurvePoints+1);
              for(int i=0;i<cyclicCurvePoints+1;i++){
                cyclicCurveDataPointsToInterpolate[i]=(*cyclicCurve3)[i];
              }
              ColumnMatrix<GLdouble> knot_vector(cyclicCurvePoints+1);
              for(int i=0;i<cyclicCurvePoints+1;i++){
                knot_vector[i]=TWO_PI*( i / (GLdouble) cyclicCurvePoints+1);
              }

              if(cyclicCurve3->UpdateDataForInterpolation(knot_vector,cyclicCurveDataPointsToInterpolate)){
                  _cyclicCurve3InterpolationImage =cyclicCurve3->GenerateImage(0,200);
                  if(_cyclicCurve3InterpolationImage){
                      _cyclicCurve3InterpolationImage->UpdateVertexBufferObjects();
                  }
              }

            //TensorProductSurface3
              tensorSurface3DataGrid = new HyperbolicPatch3(1);
//              for(int i=0;i<4;i++){
//                for(int j=0;j<4;j++){
//                  tensorSurface3DataGrid->SetData(i,j,i,j,rand()%3);
//                }
//              }

              tensorSurface3DataGrid->SetData(0,0,-2.0,-2.0,0.0);
              tensorSurface3DataGrid->SetData(0,1,-2.0,-1.0,0.0);
              tensorSurface3DataGrid->SetData(0,2,-2.0,1.0,0.0);
              tensorSurface3DataGrid->SetData(0,3,-2.0,2.0,0.0);

              tensorSurface3DataGrid->SetData(1,0,-1.0,-2.0,0.0);
              tensorSurface3DataGrid->SetData(1,1,-1.0,-1.0,2.0);
              tensorSurface3DataGrid->SetData(1,2,-1.0,1.0,2.0);
              tensorSurface3DataGrid->SetData(1,3,-1.0,2.0,0.0);

              tensorSurface3DataGrid->SetData(2,0,1.0,-2.0,0.0);
              tensorSurface3DataGrid->SetData(2,1,1.0,-1.0,2.0);
              tensorSurface3DataGrid->SetData(2,2,1.0,1.0,2.0);
              tensorSurface3DataGrid->SetData(2,3,1.0,2.0,0.0);

              tensorSurface3DataGrid->SetData(3,0,2.0,-2.0,0.0);
              tensorSurface3DataGrid->SetData(3,1,2.0,-1.0,0.0);
              tensorSurface3DataGrid->SetData(3,2,2.0,1.0,0.0);
              tensorSurface3DataGrid->SetData(3,3,2.0,2.0,0.0);
              //Generate isoparametric lines
              UISOLINES = tensorSurface3DataGrid->GenerateUIsoparametricLines(10,1,200);
              for(int i=0;i<UISOLINES->GetColumnCount();i++){
                  (*UISOLINES)[i]->UpdateVertexBufferObjects(derivative_scale);
              }

              VISOLINES = tensorSurface3DataGrid->GenerateVIsoparametricLines(10,1,200);
              for(int i=0;i<VISOLINES->GetColumnCount();i++){
                  (*VISOLINES)[i]->UpdateVertexBufferObjects(derivative_scale);
              }

              tensorSurface3DataGrid->UpdateVertexBufferObjectsOfData();
              hyperbolicPatch3Image = tensorSurface3DataGrid->GenerateImage(200,200);
              hyperbolicPatch3Image->UpdateVertexBufferObjects();
                //Interpolation example
              RowMatrix<GLdouble> u_knot_vector(4);
              ColumnMatrix<GLdouble> v_knot_vector(4);
              u_knot_vector(0) = 0.0;
              u_knot_vector(1) = 1.0 / 3.0;
              u_knot_vector(2) = 2.0 / 3.0;
              u_knot_vector(3) = 1.0;

              v_knot_vector(0) = 0.0;
              v_knot_vector(1) = 1.0 / 3.0;
              v_knot_vector(2) = 2.0 / 3.0;
              v_knot_vector(3) = 1.0;
              Matrix<DCoordinate3> data_points_to_interpolate(4,4);
              for (int i = 0; i < 4; ++i) {
                  for (int j = 0; j < 4; ++j) {
                     tensorSurface3DataGrid->GetData(i,j,data_points_to_interpolate(i,j));
                    }
              }
              if(tensorSurface3DataGrid->UpdateDataForInterpolation(u_knot_vector,v_knot_vector,data_points_to_interpolate)){
                  hyperbolicPatch3InterpolationImage = tensorSurface3DataGrid->GenerateImage(200,200);
                  if(hyperbolicPatch3InterpolationImage){
                     hyperbolicPatch3InterpolationImage->UpdateVertexBufferObjects();
                  }
              }
//              compositeCurve = new HyperbolicCompositeCurve3(5);

//              ColumnMatrix<DCoordinate3> _curve1(4);
//              _curve1[0]=DCoordinate3(0,0,0);
//              _curve1[1]=DCoordinate3(0,0,2);
//              _curve1[2]=DCoordinate3(0,2,2);
//              _curve1[3]=DCoordinate3(2,2,2);
//              if(!compositeCurve->insert(2,2,_curve1,0.2)){
//                  std::cout<<"Error inserting arc"<<endl;
//              }
//              if(!compositeCurve->continueExisting(0,HyperbolicCompositeCurve3::Right,2,2,0.2)){
//                  std::cout<<"Error continuing arc"<<endl;
//              }
//              _curve1[0]=DCoordinate3(2,2,2);
//              _curve1[1]=DCoordinate3(2,2,4);
//              _curve1[2]=DCoordinate3(2,4,4);
//              _curve1[3]=DCoordinate3(4,4,4);
//              if(!compositeCurve->insert(2,2,_curve1,0.2)){
//                  std::cout<<"Error inserting arc"<<endl;
//              }

              // shaders

              installShaders();
            //eof mine

        }
        catch (Exception &e)
        {
            cout << e << endl;
        }
    }   
    //-----------------------
    // the rendering function
    //-----------------------
    void GLWidget::paintGL()
    {
        // clears the color and depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // stores/duplicates the original model view matrix
        glPushMatrix();

            // applying transformations//Triangle stuff
            glRotatef(_angle_x, 1.0, 0.0, 0.0);
            glRotatef(_angle_y, 0.0, 1.0, 0.0);
            glRotatef(_angle_z, 0.0, 0.0, 1.0);
            glTranslated(_trans_x, _trans_y, _trans_z);
            glScaled(_zoom, _zoom, _zoom);
             if(currentHomework == DynamicVBO || currentHomework == ParametricSurface){
               glEnable(GL_LIGHTING);
               glEnable(GL_LIGHT0);
               glEnable(GL_NORMALIZE);
               }else{
                 glDisable(GL_LIGHTING);
                 glDisable(GL_LIGHT0);
                 glDisable(GL_NORMALIZE);
               }

//            // render your geometry (this is oldest OpenGL rendering technique, later we will use some advanced methods)

//            glColor3f(1.0f, 1.0f, 1.0f);
//            glBegin(GL_LINES);
//                glVertex3f(0.0f, 0.0f, 0.0f);
//                glVertex3f(1.1f, 0.0f, 0.0f);

//                glVertex3f(0.0f, 0.0f, 0.0f);
//                glVertex3f(0.0f, 1.1f, 0.0f);

//                glVertex3f(0.0f, 0.0f, 0.0f);
//                glVertex3f(0.0f, 0.0f, 1.1f);
//            glEnd();

//            glBegin(GL_TRIANGLES);
//                // attributes
//                glColor3f(1.0f, 0.0f, 0.0f);
//                // associated with position
//                glVertex3f(1.0f, 0.0f, 0.0f);

//                // attributes
//                glColor3f(0.0, 1.0, 0.0);
//                // associated with position
//                glVertex3f(0.0, 1.0, 0.0);

//                // attributes
//                glColor3f(0.0f, 0.0f, 1.0f);
//                // associated with position
//                glVertex3f(0.0f, 0.0f, 1.0f);
//            glEnd();
        //Parametric curves
        if(currentHomework == ParametricCurve){
              if(_image_of_pc){
                glColor3f(1.0f,0.0f,0.0f);
                _image_of_pc->RenderDerivatives(0,GL_LINE_STRIP);
                glPointSize(5.0f);
                    glColor3f(0.0f,0.5f,0.0f);
                    _image_of_pc->RenderDerivatives(1,GL_LINES);
                    _image_of_pc->RenderDerivatives(1,GL_POINTS);

                    glColor3f(0.1f,0.5f,0.9f);
                    _image_of_pc->RenderDerivatives(2,GL_LINES);
                    _image_of_pc->RenderDerivatives(2,GL_POINTS);
                glPointSize(1.0f);
              }
         }
        //eof Parametric curves
        //Testing dynamic vertex buffers
        if(currentHomework == DynamicVBO ){
              //MatFBPearl.Apply();
              MatFBEmerald.Apply();
              _mouse.Render();
              }
        if( currentHomework== ParametricSurface){
              //MatFBPearl.Apply();
              MatFBEmerald.Apply();
              _surface.Render();
              }
        //Hyperbolic Arc3
        if(currentHomework == HyperbolicArc3){
              if(_hyperbolicArc3Image){
                glColor3f(1.0f,0.0f,0.0f);
                _hyperbolicArc3Image->RenderDerivatives(0,GL_LINE_STRIP);
                glPointSize(5.0f);
                    glColor3f(0.0f,0.5f,0.0f);
                    _hyperbolicArc3Image->RenderDerivatives(1,GL_LINES);
                    //_arc->RenderDerivatives(1,GL_POINTS);
                    _hyperbolicArc3Image->RenderDerivatives(1,GL_LINES);
                    glColor3f(1.0f,0.3f,0.3f);
                    _hyperbolicArc3Image->RenderDerivatives(2,GL_LINES);
                    //_arc->RenderDerivatives(2,GL_POINTS);
                    glColor3f(0.2f,0.2f,0.6f);
                hyperbolicArc3->RenderData();
                }
         }

        if(currentHomework == CyclicCurve){
              if(_cyclicCurve3Image){
                glColor3f(1.0f,0.0f,0.0f);
                _cyclicCurve3Image->RenderDerivatives(0,GL_LINE_STRIP);
                glPointSize(5.0f);
                    glColor3f(0.0f,0.5f,0.0f);
                    _cyclicCurve3Image->RenderDerivatives(1,GL_LINES);
                    //_arc->RenderDerivatives(1,GL_POINTS);
                    _cyclicCurve3Image->RenderDerivatives(1,GL_LINES);
                    glColor3f(1.0f,0.3f,0.3f);
                    _cyclicCurve3Image->RenderDerivatives(2,GL_LINES);
                    //_arc->RenderDerivatives(2,GL_POINTS);
                    glColor3f(0.2f,0.2f,0.6f);
                    cyclicCurve3->RenderData();
                }
              if(_cyclicCurve3InterpolationImage){
                glColor3f(0.0f,0.5f,0.5f);
                _cyclicCurve3InterpolationImage->RenderDerivatives(0,GL_LINE_STRIP);
                }
         }
        //TensorProductSurface3
        if(currentHomework == TensorProductSurface3){
            glColor3f(0.5f,0.2f,0.0f);
            glPointSize(5.0f);
            if(tensorSurface3DataGrid){
              tensorSurface3DataGrid->RenderData();
            }
            if(hyperbolicPatch3Image){
              glEnable(GL_LIGHTING);
              glEnable(GL_LIGHT0);
              glEnable(GL_NORMALIZE);
              MatFBEmerald.Apply();
              hyperbolicPatch3Image->Render();
              glDisable(GL_LIGHTING);
              glDisable(GL_LIGHT0);
              glDisable(GL_NORMALIZE);
            }
            if(hyperbolicPatch3InterpolationImage){
              glEnable(GL_LIGHTING);
              glEnable(GL_LIGHT0);
              glEnable(GL_NORMALIZE);

              glEnable(GL_BLEND);
              glDepthMask(GL_FALSE);
              glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                MatFBTurquoise.Apply();
                hyperbolicPatch3InterpolationImage->Render();
              glDepthMask(GL_TRUE);
              glDisable(GL_BLEND);

              glDisable(GL_LIGHTING);
              glDisable(GL_LIGHT0);
              glDisable(GL_NORMALIZE);
            }

            if(UISOLINES){
                for(int i=0;i<UISOLINES->GetColumnCount();i++){
                  glColor3f(0.2f,0.6f,0.6f);
                  (*UISOLINES)[i]->RenderDerivatives(0,GL_LINE_STRIP);
                  glColor3f(0.2f,0.7f,0.9f);
                  (*UISOLINES)[i]->RenderDerivatives(1,GL_LINES);
//                  glColor3f(0.8f,0.7f,0.0f);
//                  (*UISOLINES)[i]->RenderDerivatives(2,GL_LINES);
                }
            }

            if(VISOLINES){
                for(int i=0;i<VISOLINES->GetColumnCount();i++){
                  glColor3f(0.2f,0.6f,0.6f);
                  (*VISOLINES)[i]->RenderDerivatives(0,GL_LINE_STRIP);
                  glColor3f(0.2f,0.7f,0.9f);
                  (*VISOLINES)[i]->RenderDerivatives(1,GL_LINES);
//                  glColor3f(0.8f,0.7f,0.0f);
//                  (*UISOLINES)[i]->RenderDerivatives(2,GL_LINES);
                }
            }            
        }

        if(currentHomework == CompositeCurve){
            if(compositeCurve){
                compositeCurve->renderAll(0);
            }
        }
        if(currentHomework == CompositePatch){
            if(compositePatch){
                if(_show_shader) {
                    _shader[_shader_to_show]->Enable();
                }
               compositePatch->renderAll();

               if(_show_shader) {
                   _shader[_shader_to_show]->Disable();
               }
            }
        }
        //EOF Testing dynamic vertex buffers
        // pops the current matrix stack, replacing the current matrix with the one below it on the stack,
        // i.e., the original model view matrix is restored
        glPopMatrix();
    }

    //----------------------------------------------------------------------------
    // when the main window is resized one needs to redefine the projection matrix
    //----------------------------------------------------------------------------
    void GLWidget::resizeGL(int w, int h)
    {
        // setting the new size of the rendering context
        glViewport(0, 0, w, h);

        // redefining the projection matrix
        glMatrixMode(GL_PROJECTION);

        glLoadIdentity();

        _aspect = (float)w / (float)h;

        gluPerspective(_fovy, _aspect, _z_near, _z_far);

        // switching back to the model view matrix
        glMatrixMode(GL_MODELVIEW);

        updateGL();
    }


    //-----------------------------------
    // implementation of the public slots
    //-----------------------------------

    void GLWidget::set_angle_x(int value)
    {
        if (_angle_x != value)
        {
            _angle_x = value;
            updateGL();
        }
    }

    void GLWidget::set_angle_y(int value)
    {
        if (_angle_y != value)
        {
            _angle_y = value;
            updateGL();
        }
    }

    void GLWidget::set_angle_z(int value)
    {
        if (_angle_z != value)
        {
            _angle_z = value;
            updateGL();
        }
    }

    void GLWidget::set_zoom_factor(double value)
    {
        if (_zoom != value)
        {
            _zoom = value;
            updateGL();
        }
    }

    void GLWidget::set_trans_x(double value)
    {
        if (_trans_x != value)
        {
            _trans_x = value;
            updateGL();
        }
    }

    void GLWidget::set_trans_y(double value)
    {
        if (_trans_y != value)
        {
            _trans_y = value;
            updateGL();
        }
    }

    void GLWidget::set_trans_z(double value)
    {
        if (_trans_z != value)
        {
            _trans_z = value;
            updateGL();
        }
    }

    //my stuff
    void GLWidget::change_curve(int index){
         currentHomework = ParametricCurve;
        _image_of_pc=curves[index];
        _image_of_pc->UpdateVertexBufferObjects(derivative_scale,GL_STATIC_DRAW);
         updateGL();
    }
    void GLWidget::change_scale(int scale){
      derivative_scale=(float)scale/100;
        _image_of_pc->UpdateVertexBufferObjects(derivative_scale,GL_STATIC_DRAW);
         updateGL();
    }
    void GLWidget::initialize_curves(){
      derivative_scale=0.3;
      RowMatrix<ParametricCurve3::Derivative> derivative(3);
      derivative(0)=ellipse::d0;
      derivative(1)=ellipse::d1;
      derivative(2)=ellipse::d2;

      _pc=nullptr;
      _pc=new (nothrow) ParametricCurve3(derivative,ellipse::u_min,ellipse::u_max);

          if(!_pc){
              cout<<"Error in GLWidget, parametric curve"<<endl;
           }
          GLuint div_point_count = 200;
          GLenum usage_flag = GL_STATIC_DRAW;

          _image_of_pc = nullptr;
          _image_of_pc = _pc->GenerateImage(div_point_count,usage_flag);
          curves[0]=_image_of_pc;

          derivative(0)=helix::d0;
          derivative(1)=helix::d1;
          derivative(2)=helix::d2;

          _pc=nullptr;
          _pc=new (nothrow) ParametricCurve3(derivative,ellipse::u_min,ellipse::u_max);

           if(!_pc){
                cout<<"Error in GLWidget, parametric curve"<<endl;
           }
           div_point_count = 200;
           usage_flag = GL_STATIC_DRAW;

              _image_of_pc = nullptr;
              _image_of_pc = _pc->GenerateImage(div_point_count,usage_flag);
              curves[1]=_image_of_pc;

              derivative(0)=lissajous::d0;
              derivative(1)=lissajous::d1;
              derivative(2)=lissajous::d2;

              _pc=nullptr;
              _pc=new (nothrow) ParametricCurve3(derivative,lissajous::u_min,lissajous::u_max);

               if(!_pc){
                    cout<<"Error in GLWidget, parametric curve"<<endl;
               }
               div_point_count = 200;
               usage_flag = GL_STATIC_DRAW;

                  _image_of_pc = nullptr;
                  _image_of_pc = _pc->GenerateImage(div_point_count,usage_flag);
                  curves[2]=_image_of_pc;

                  derivative(0)=sphere::d0;
                  derivative(1)=sphere::d1;
                  derivative(2)=sphere::d2;

                  _pc=nullptr;
                  _pc=new (nothrow) ParametricCurve3(derivative,sphere::u_min,sphere::u_max);

                   if(!_pc){
                        cout<<"Error in GLWidget, parametric curve"<<endl;
                   }
                   div_point_count = 200;
                   usage_flag = GL_STATIC_DRAW;

                      _image_of_pc = nullptr;
                      _image_of_pc = _pc->GenerateImage(div_point_count,usage_flag);
                      curves[3]=_image_of_pc;

                      derivative(0)=rose::d0;
                      derivative(1)=rose::d1;
                      derivative(2)=rose::d2;

                      _pc=nullptr;
                      _pc=new (nothrow) ParametricCurve3(derivative,rose::u_min,rose::u_max);

                       if(!_pc){
                            cout<<"Error in GLWidget, parametric curve"<<endl;
                       }
                       div_point_count = 200;
                       usage_flag = GL_STATIC_DRAW;

                          _image_of_pc = nullptr;
                          _image_of_pc = _pc->GenerateImage(div_point_count,usage_flag);
                          curves[4]=_image_of_pc;
    }
    //eof mine

    void GLWidget::_animate(){
      GLfloat * vertex = _mouse.MapVertexBuffer(GL_READ_WRITE);
      GLfloat * normal = _mouse.MapNormalBuffer(GL_READ_ONLY);
      _angle += DEG_TO_RADIAN;
      if(_angle >=TWO_PI )_angle -=TWO_PI;

      GLfloat scale = sin(_angle)/3000.0;
      for(GLuint i=0;i<_mouse.VertexCount();++i){
          for(GLuint coordinate = 0;coordinate<3;++coordinate, ++vertex,++normal){
            *vertex+=scale*(*normal);
          }
      }
      _mouse.UnmapVertexBuffer();
      _mouse.UnmapNormalBuffer();

      updateGL();
    }
    void GLWidget::change_surface(int index){
        currentHomework = ParametricSurface;
        _surface=*(surfaces[index]);
        _surface.UpdateVertexBufferObjects((GL_DYNAMIC_DRAW));
         updateGL();
    }
    void GLWidget::initialize_surfaces(){
      TriangularMatrix<ParametricSurface3::PartialDerivative> derivatives(3);
      derivatives(0,0)=surface1::d00;
      derivatives(1,0)=surface1::d10;
      derivatives(1,1)=surface1::d01;

      ParametricSurface3 surf1(derivatives,surface1::u_min,surface1::u_max,surface1::v_min,surface1::v_max);
      surfaces[0]= surf1.GenerateImage(200,200,true);


      derivatives(0,0)=surface2::d00;
      derivatives(1,0)=surface2::d10;
      derivatives(1,1)=surface2::d01;
      ParametricSurface3 surf2(derivatives,surface2::u_min,surface2::u_max,surface2::v_min,surface2::v_max);
      surfaces[1]= surf2.GenerateImage(200,200,true);

      derivatives(0,0)=surface3::d00;
      derivatives(1,0)=surface3::d10;
      derivatives(1,1)=surface3::d01;
      ParametricSurface3 surf3(derivatives,surface3::u_min,surface3::u_max,surface3::v_min,surface3::v_max);
      surfaces[2]= surf3.GenerateImage(200,200,true);

      derivatives(0,0)=surface4::d00;
      derivatives(1,0)=surface4::d10;
      derivatives(1,1)=surface4::d01;
      ParametricSurface3 surf4(derivatives,surface4::u_min,surface4::u_max,surface4::v_min,surface4::v_max);
      surfaces[3]= surf4.GenerateImage(200,200,true);

      derivatives(0,0)=surface5::d00;
      derivatives(1,0)=surface5::d10;
      derivatives(1,1)=surface5::d01;
      ParametricSurface3 surf5(derivatives,surface5::u_min,surface5::u_max,surface5::v_min,surface5::v_max);
      surfaces[4]= surf5.GenerateImage(200,200,true);
    }

    void GLWidget::change_ArcAlpha(double alpha){
        hyperbolicArc3->setAlpha(alpha);
        //hyperbolicArc3->UpdateVertexBufferObjectsOfData();
        if(_hyperbolicArc3Image)delete  _hyperbolicArc3Image;
        _hyperbolicArc3Image =hyperbolicArc3->GenerateImage(2,200);
        _hyperbolicArc3Image->UpdateVertexBufferObjects(derivative_scale,GL_STATIC_DRAW);
        if(UISOLINES){
          for(int i=0;i<UISOLINES->GetColumnCount();i++){
              (*UISOLINES)[i]->UpdateVertexBufferObjects(derivative_scale);
          }
        }
        updateGL();
    }

    void GLWidget::change_homework(int index){
      switch(index){
        case 0:currentHomework = ParametricCurve;break;
        case 1:currentHomework = ParametricSurface;break;
        case 2:currentHomework = DynamicVBO;break;
        case 3:currentHomework = HyperbolicArc3;break;
        case 4:currentHomework = TensorProductSurface3;break;
        case 5:currentHomework = CyclicCurve;break;
        case 6:currentHomework = CompositeCurve;break;
        case 7:currentHomework = CompositePatch;break;
        }
      initializeGL();
      updateGL();
    }

    void GLWidget::insertArc(){
      if(!compositeCurve){
          compositeCurve = new HyperbolicCompositeCurve3(MAX_CURVE_COUNT);
      }
      ColumnMatrix<DCoordinate3> coords(4);
      coords[0]=DCoordinate3(_sideWidget->ArcInsertCoord00->value(),_sideWidget->ArcInsertCoord01->value(),_sideWidget->ArcInsertCoord02->value());
      coords[1]=DCoordinate3(_sideWidget->ArcInsertCoord10->value(),_sideWidget->ArcInsertCoord11->value(),_sideWidget->ArcInsertCoord12->value());
      coords[2]=DCoordinate3(_sideWidget->ArcInsertCoord20->value(),_sideWidget->ArcInsertCoord21->value(),_sideWidget->ArcInsertCoord22->value());
      coords[3]=DCoordinate3(_sideWidget->ArcInsertCoord30->value(),_sideWidget->ArcInsertCoord31->value(),_sideWidget->ArcInsertCoord32->value());
      Color4 color(_sideWidget->ArcInsertColorR->value(),_sideWidget->ArcInsertColorG->value(),_sideWidget->ArcInsertColorB->value());
      compositeCurve->insert(_sideWidget->ArcInsertAlphaSpinBox->value(),2,coords,_sideWidget->ArcInsertScaleSpinBox->value(),color);
      updateGL();
    }
    void GLWidget::continueArc(){
      if(compositeCurve){
          if(_sideWidget->ArcContinueDirection->currentIndex() == 0)
            compositeCurve->continueExisting(_sideWidget->ArcContinueSpinBox->value(),HyperbolicCompositeCurve3::Right,
                                             _sideWidget->ArcContinueAlpha->value(),2,_sideWidget->ArcContinueScale->value());
          else
            compositeCurve->continueExisting(_sideWidget->ArcContinueSpinBox->value(),
                                             HyperbolicCompositeCurve3::Left,_sideWidget->ArcContinueAlpha->value(),2,_sideWidget->ArcContinueScale->value());
          updateGL();
      }
    }

    void GLWidget::updateSpheresOnArcSelected(int index){
      if(compositeCurve){
            compositeCurve->updateSpheresLocationByindex(index);
            updateGL();
        }
    }

    void GLWidget::joinArcs(){
      if(compositeCurve){
          HyperbolicCompositeCurve3::Direction firstDirection;
          HyperbolicCompositeCurve3::Direction secondDirection;

          if(_sideWidget->ArcJoinFirstDirection->currentIndex() == 0 ){
            firstDirection = HyperbolicCompositeCurve3::Direction::Right;
            }
          else {
            firstDirection = HyperbolicCompositeCurve3::Direction::Left;
          }
          if(_sideWidget->ArcJoinSecondDirection->currentIndex() == 0 ){
            secondDirection = HyperbolicCompositeCurve3::Direction::Right;
            }
          else {
            secondDirection = HyperbolicCompositeCurve3::Direction::Left;
          }
          int firstIndex=_sideWidget->ArcJoinFirstIndex->value();
          int secondIndex=_sideWidget->ArcJoinSecondIndex->value();
          compositeCurve->join(firstIndex,secondIndex,firstDirection,secondDirection,_sideWidget->ArcJoinScale->value());
          //  std::cerr<<" error, in joining curves"<<std::endl;
          compositeCurve->updateSpheresLocationByindex(secondIndex);
          updateGL();
      }
    }

    void GLWidget::changeTransformPointIndex(int id){
      if(compositeCurve){
          int currentArcIndex =  _sideWidget->ArcTransformArcIndex->value();
          if(currentArcIndex>= 0  &&  currentArcIndex<  compositeCurve->getSize() && id>=0 && id<4){
              isBeingTransformed=true;
            _sideWidget->ArcTransformX->setValue((*(compositeCurve->getArc(currentArcIndex)->arc))[id][0]);
            _sideWidget->ArcTransformY->setValue((*(compositeCurve->getArc(currentArcIndex)->arc))[id][1]);
            _sideWidget->ArcTransformZ->setValue((*(compositeCurve->getArc(currentArcIndex)->arc))[id][2]);
            isBeingTransformed=false;
            updateGL();
          }
      }
    }

    void GLWidget::changeTransformX(double x){
      if(isBeingTransformed)return;
      int currentArcIndex =  _sideWidget->ArcTransformArcIndex->value();
      int point_id = _sideWidget->ArcTransformPointIndex->value();
      updateXYZ(currentArcIndex,point_id);
      updateSpheresOnArcSelected(currentArcIndex);

    }

    void GLWidget::changeTransformY(double y){
      if(isBeingTransformed)return;
      int currentArcIndex =  _sideWidget->ArcTransformArcIndex->value();
      int point_id = _sideWidget->ArcTransformPointIndex->value();
      updateXYZ(currentArcIndex,point_id);
      updateSpheresOnArcSelected(currentArcIndex);
    }
    void GLWidget::changeTransformZ(double z){
      if(isBeingTransformed)return;
      int currentArcIndex =  _sideWidget->ArcTransformArcIndex->value();
      int point_id = _sideWidget->ArcTransformPointIndex->value();
      updateXYZ(currentArcIndex,point_id);
      updateSpheresOnArcSelected(currentArcIndex);
    }

    void GLWidget::updateXYZ(int arcIndex, int point_id){
      if(compositeCurve){          
          if(arcIndex>= 0  &&  arcIndex<  compositeCurve->getSize() && point_id>=0 && point_id<4){
            DCoordinate3 newcoord(_sideWidget->ArcTransformX->value(),_sideWidget->ArcTransformY->value(),_sideWidget->ArcTransformZ->value());
            if(!compositeCurve->updatePosition(arcIndex,point_id,newcoord)){
                std::cerr<<"error updating arc positions"<<std::endl;
              }
          }
      }
      updateGL();
    }

    void GLWidget::mergeArcs(){
      if(compositeCurve){
          int firstIndex    = _sideWidget->ArcMergeFirstIndex->value();
          int secondIndex    = _sideWidget->ArcMergeSecondIndex->value();
          HyperbolicCompositeCurve3::Direction firstDirection = _sideWidget->ArcMergeFirstDirection->currentIndex() == 0 ?HyperbolicCompositeCurve3::Right:HyperbolicCompositeCurve3::Left;
          HyperbolicCompositeCurve3::Direction secondDirection = _sideWidget->ArcMergeSecondDirection->currentIndex() == 0 ?HyperbolicCompositeCurve3::Right:HyperbolicCompositeCurve3::Left;
          compositeCurve->merge(firstIndex,secondIndex,firstDirection,secondDirection);
          updateSpheresOnArcSelected(secondIndex);
          updateGL();
      }
    }
    void GLWidget::insertPatchX(){
      if(!compositePatch){
         compositePatch = new HyperbolicCompositePatch3(MAX_PATCH_COUNT);
         patchMaterials[0]=MatFBEmerald;
         patchMaterials[1]=MatFBGold;
         patchMaterials[2]=MatFBBrass;
      }

      ColumnMatrix<DCoordinate3> data(16);
      data[0]=DCoordinate3(5,0,0);
      data[1]=DCoordinate3(5,1,0);
      data[2]=DCoordinate3(5,2,1);
      data[3]=DCoordinate3(5,3,0);

      data[4]=DCoordinate3(6,0,0);
      data[5]=DCoordinate3(6,1,0);
      data[6]=DCoordinate3(6,2,0);
      data[7]=DCoordinate3(6,3,0);

      data[8]=DCoordinate3(7,0,0);
      data[9]=DCoordinate3(7,1,0);
      data[10]=DCoordinate3(7,2,0);
      data[11]=DCoordinate3(7,3,0);

      data[12]=DCoordinate3(8,0,0);
      data[13]=DCoordinate3(8,1,0);
      data[14]=DCoordinate3(8,2,0);
      data[15]=DCoordinate3(8,3,0);
      compositePatch->insert(5,1,data);
      updateGL();
    }

    void GLWidget::insertPatchY(){
      if(!compositePatch){
         compositePatch = new HyperbolicCompositePatch3(MAX_PATCH_COUNT);
         patchMaterials[0]=MatFBEmerald;
         patchMaterials[1]=MatFBGold;
         patchMaterials[2]=MatFBBrass;
      }

      ColumnMatrix<DCoordinate3> data(16);
      data[0]=DCoordinate3(0,5,0);
      data[1]=DCoordinate3(0,6,0);
      data[2]=DCoordinate3(0,7,1);
      data[3]=DCoordinate3(0,8,0);

      data[4]=DCoordinate3(1,5,0);
      data[5]=DCoordinate3(1,6,0);
      data[6]=DCoordinate3(1,7,0);
      data[7]=DCoordinate3(1,8,0);

      data[8]=DCoordinate3(2,5,0);
      data[9]=DCoordinate3(2,6,0);
      data[10]=DCoordinate3(2,7,0);
      data[11]=DCoordinate3(2,8,0);

      data[12]=DCoordinate3(3,5,0);
      data[13]=DCoordinate3(3,6,0);
      data[14]=DCoordinate3(3,7,0);
      data[15]=DCoordinate3(3,8,0);
      compositePatch->insert(5,1,data);
      updateGL();
    }

    void GLWidget::clearPatch(){
      if(compositePatch){
          compositePatch->clear();
          updateGL();
        }
    }
    void GLWidget::insertPatch(){
      if(!compositePatch){
         compositePatch = new HyperbolicCompositePatch3(MAX_PATCH_COUNT);
         patchMaterials[0]=MatFBEmerald;
         patchMaterials[1]=MatFBGold;
         patchMaterials[2]=MatFBBrass;
      }

      ColumnMatrix<DCoordinate3> coords(16);
      coords[0]=DCoordinate3(_sideWidget->PatchInsertCoord00X->value(),_sideWidget->PatchInsertCoord00Y->value(),_sideWidget->PatchInsertCoord00Z->value());
      coords[1]=DCoordinate3(_sideWidget->PatchInsertCoord01X->value(),_sideWidget->PatchInsertCoord01Y->value(),_sideWidget->PatchInsertCoord01Z->value());
      coords[2]=DCoordinate3(_sideWidget->PatchInsertCoord02X->value(),_sideWidget->PatchInsertCoord02Y->value(),_sideWidget->PatchInsertCoord02Z->value());
      coords[3]=DCoordinate3(_sideWidget->PatchInsertCoord03X->value(),_sideWidget->PatchInsertCoord03Y->value(),_sideWidget->PatchInsertCoord03Z->value());
      coords[4]=DCoordinate3(_sideWidget->PatchInsertCoord10X->value(),_sideWidget->PatchInsertCoord10Y->value(),_sideWidget->PatchInsertCoord10Z->value());
      coords[5]=DCoordinate3(_sideWidget->PatchInsertCoord11X->value(),_sideWidget->PatchInsertCoord11Y->value(),_sideWidget->PatchInsertCoord11Z->value());
      coords[6]=DCoordinate3(_sideWidget->PatchInsertCoord12X->value(),_sideWidget->PatchInsertCoord12Y->value(),_sideWidget->PatchInsertCoord12Z->value());
      coords[7]=DCoordinate3(_sideWidget->PatchInsertCoord13X->value(),_sideWidget->PatchInsertCoord13Y->value(),_sideWidget->PatchInsertCoord13Z->value());

      coords[8]=DCoordinate3(_sideWidget->PatchInsertCoord20X->value(),_sideWidget->PatchInsertCoord20Y->value(),_sideWidget->PatchInsertCoord20Z->value());
      coords[9]=DCoordinate3(_sideWidget->PatchInsertCoord21X->value(),_sideWidget->PatchInsertCoord21Y->value(),_sideWidget->PatchInsertCoord21Z->value());
      coords[10]=DCoordinate3(_sideWidget->PatchInsertCoord22X->value(),_sideWidget->PatchInsertCoord22Y->value(),_sideWidget->PatchInsertCoord22Z->value());
      coords[11]=DCoordinate3(_sideWidget->PatchInsertCoord23X->value(),_sideWidget->PatchInsertCoord23Y->value(),_sideWidget->PatchInsertCoord23Z->value());
      coords[12]=DCoordinate3(_sideWidget->PatchInsertCoord30X->value(),_sideWidget->PatchInsertCoord30Y->value(),_sideWidget->PatchInsertCoord30Z->value());
      coords[13]=DCoordinate3(_sideWidget->PatchInsertCoord31X->value(),_sideWidget->PatchInsertCoord31Y->value(),_sideWidget->PatchInsertCoord31Z->value());
      coords[14]=DCoordinate3(_sideWidget->PatchInsertCoord32X->value(),_sideWidget->PatchInsertCoord32Y->value(),_sideWidget->PatchInsertCoord32Z->value());
      coords[15]=DCoordinate3(_sideWidget->PatchInsertCoord33X->value(),_sideWidget->PatchInsertCoord33Y->value(),_sideWidget->PatchInsertCoord33Z->value());
      if(compositePatch->insert(_sideWidget->PatchInsertAlphaSpinBox->value(),1,coords,patchMaterials[_sideWidget->PatchInsertMaterial->currentIndex()])){
          cout<<"inserting patch"<<endl;
        }else{
          cout<<"not inserting patch"<<endl;
        }
      updateGL();
    }

    void GLWidget::continuePatch(){
      if(compositePatch){
          compositePatch->continueExisting(_sideWidget->PatchContinueSpinBox->value(),(HyperbolicCompositePatch3::Direction)_sideWidget->PatchContinueDirection->currentIndex(),_sideWidget->PatchContinueAlpha->value(),
                                           patchMaterials[_sideWidget->PatchContinueMaterial->currentIndex()]);
          updateGL();
      }
    }

    void GLWidget::updateSelectionCurveOnContinue(){
      GLuint patchIndex = _sideWidget->PatchContinueSpinBox->value();
      HyperbolicCompositePatch3::Direction direction =(HyperbolicCompositePatch3::Direction) _sideWidget->PatchContinueDirection->currentIndex();
      updateSelectionCurveOnPatchSelected(patchIndex,direction);

    }

    void GLWidget::updateSelectionCurveOnJoinFirst(){
      GLuint patchIndex = _sideWidget->PatchJoinFirstIndex->value();
      HyperbolicCompositePatch3::Direction direction =(HyperbolicCompositePatch3::Direction) _sideWidget->PatchJoinFirstDirection->currentIndex();
      updateSelectionCurveOnPatchSelected(patchIndex,direction);
    }
    void GLWidget::updateSelectionCurveOnJoinSecond(){
      GLuint patchIndex = _sideWidget->PatchJoinSecondIndex->value();
      HyperbolicCompositePatch3::Direction direction =(HyperbolicCompositePatch3::Direction) _sideWidget->PatchJoinSecondDirection->currentIndex();
      updateSelectionCurveOnPatchSelected(patchIndex,direction);
    }
    void GLWidget::updateSelectionCurveOnMergeFirst(){
      GLuint patchIndex = _sideWidget->PatchMergeFirstIndex->value();
      HyperbolicCompositePatch3::Direction direction =(HyperbolicCompositePatch3::Direction) _sideWidget->PatchMergeFirstDirection->currentIndex();
      updateSelectionCurveOnPatchSelected(patchIndex,direction);
    }
    void GLWidget::updateSelectionCurveOnMergeSecond(){
      GLuint patchIndex = _sideWidget->PatchMergeSecondIndex->value();
      HyperbolicCompositePatch3::Direction direction =(HyperbolicCompositePatch3::Direction) _sideWidget->PatchMergeSecondDirection->currentIndex();
      updateSelectionCurveOnPatchSelected(patchIndex,direction);
    }
    void GLWidget::joinPatch(){
      if(compositePatch){
        compositePatch->join(_sideWidget->PatchJoinFirstIndex->value(),_sideWidget->PatchJoinSecondIndex->value(),
                             (HyperbolicCompositePatch3::Direction)(_sideWidget->PatchJoinFirstDirection->currentIndex()),
                             (HyperbolicCompositePatch3::Direction)(_sideWidget->PatchJoinSecondDirection->currentIndex()));
        updateGL();
      }
    }

    void GLWidget::changeTransformPointsIndex(){
      if(compositePatch){
          if(_sideWidget->PatchTransformPatchIndex->value()<0 || _sideWidget->PatchTransformPatchIndex->value()>=compositePatch->getSize()){
            return;
          }
        DCoordinate3 currentCoord =compositePatch->getCoord(_sideWidget->PatchTransformPointI->value(),
                                                            _sideWidget->PatchTransformPointJ->value(),
                                                            _sideWidget->PatchTransformPatchIndex->value());
        _sideWidget->PatchTransformX->setValue(currentCoord.x());
        _sideWidget->PatchTransformY->setValue(currentCoord.y());
        _sideWidget->PatchTransformZ->setValue(currentCoord.z());
        updateSphereOnPatchSelected();
      }
    }
    void GLWidget::transformPatchX(){
      updatePatchXYZ(_sideWidget->PatchTransformPointI->value(),
                     _sideWidget->PatchTransformPointJ->value(),
                     _sideWidget->PatchTransformPatchIndex->value());
    }
    void GLWidget::transformPatchY(){
      updatePatchXYZ(_sideWidget->PatchTransformPointI->value(),
                     _sideWidget->PatchTransformPointJ->value(),
                     _sideWidget->PatchTransformPatchIndex->value());
    }
    void GLWidget::transformPatchZ(){
      updatePatchXYZ(_sideWidget->PatchTransformPointI->value(),
                     _sideWidget->PatchTransformPointJ->value(),
                     _sideWidget->PatchTransformPatchIndex->value());
    }

    void GLWidget::updateSphereOnPatchSelected(){
      if(compositePatch){

        GLuint selectedPatchIndex = _sideWidget->PatchTransformPatchIndex->value();
        GLuint selectedRow =_sideWidget->PatchTransformPointI->value();
        GLuint selectedColumn =_sideWidget->PatchTransformPointJ->value();

        if(selectedPatchIndex<compositePatch->getSize() && selectedRow < 4 && selectedColumn < 4){
          compositePatch->updateSpherePosition(selectedRow,selectedColumn,selectedPatchIndex);
          updateGL();
          }
        }
    }

    void GLWidget::updateSelectionCurveOnPatchSelected(GLuint patchIndex,HyperbolicCompositePatch3::Direction selectedDirection){
      if(compositePatch){
          if(patchIndex>=compositePatch->getSize()){
              cerr<<"Error updating selection curve,invalid patch index"<<endl;
              return;
          }
        compositePatch->updateSelectionCurve(patchIndex,selectedDirection);
        updateGL();
        }
    }

    void GLWidget::updatePatchXYZ(int i, int j, int patchIndex){
      if(compositePatch){
          DCoordinate3 newCoord(_sideWidget->PatchTransformX->value(),_sideWidget->PatchTransformY->value(),_sideWidget->PatchTransformZ->value());
          compositePatch->update(i,j,patchIndex,newCoord);
          updateSphereOnPatchSelected();
        }
    }

    void GLWidget::mergePatches(){
      if(compositePatch){
          int firstId = _sideWidget->PatchMergeFirstIndex->value();
          int secondId = _sideWidget->PatchMergeSecondIndex->value();
          compositePatch->merge(firstId,secondId,
                                (HyperbolicCompositePatch3::Direction)(_sideWidget->PatchMergeFirstDirection->currentIndex()),
                                (HyperbolicCompositePatch3::Direction)(_sideWidget->PatchMergeSecondDirection->currentIndex()));
          updateGL();
        }
    }

    void GLWidget::installShaders() {
        if (!_shader[0]->InstallShaders("/home/nandor/egyetem4/cs-bsc-ii-2/Grafika/build-QtFramework-Desktop_Qt_5_12_1_GCC_64bit-Debug/Shaders/directional_light.vert", "/home/nandor/egyetem4/cs-bsc-ii-2/Grafika/build-QtFramework-Desktop_Qt_5_12_1_GCC_64bit-Debug/Shaders/directional_light.frag")) {
          throw Exception("Could not install shader!");
        }

        if (!_shader[1]->InstallShaders("/home/nandor/egyetem4/cs-bsc-ii-2/Grafika/build-QtFramework-Desktop_Qt_5_12_1_GCC_64bit-Debug/Shaders/two_sided_lighting.vert", "/home/nandor/egyetem4/cs-bsc-ii-2/Grafika/build-QtFramework-Desktop_Qt_5_12_1_GCC_64bit-Debug/Shaders/two_sided_lighting.frag")) {
          throw Exception("Could not install shade!");
        }

        if (!_shader[2]->InstallShaders("/home/nandor/egyetem4/cs-bsc-ii-2/Grafika/build-QtFramework-Desktop_Qt_5_12_1_GCC_64bit-Debug/Shaders/toon.vert", "/home/nandor/egyetem4/cs-bsc-ii-2/Grafika/build-QtFramework-Desktop_Qt_5_12_1_GCC_64bit-Debug/Shaders/toon.frag")) {
          throw Exception("Could not install shader!");
        }
        else {
            _shader[2]->Enable();
            // _shader[2]->SetUniformVariable4f("default_outline_color", 1.0f, 1.0f, 1.0f, 1.0f);
            _shader[2]->Disable();
        }

        if (!_shader[3]->InstallShaders("/home/nandor/egyetem4/cs-bsc-ii-2/Grafika/build-QtFramework-Desktop_Qt_5_12_1_GCC_64bit-Debug/Shaders/reflection_lines.vert", "/home/nandor/egyetem4/cs-bsc-ii-2/Grafika/build-QtFramework-Desktop_Qt_5_12_1_GCC_64bit-Debug/Shaders/reflection_lines.frag")) {
          throw Exception("Could not install shader!");
        }
        else {
            _shader[3]->Enable();
            _shader[3]->SetUniformVariable1f("scale_factor", 4.0f);
            _shader[3]->SetUniformVariable1f("smoothing", 2.0f);
            _shader[3]->SetUniformVariable1f("shading", 1.0f);
            _shader[3]->Disable();
        }
    }

    void GLWidget::setShaderOnOrOff(bool on){
      _show_shader=on;
      updateGL();
    }

    void GLWidget::changeSelectedShader(int shaderIndex){
      if(shaderIndex<0 || shaderIndex>=4)return;
      _shader_to_show=shaderIndex;
      updateGL();
    }

    void GLWidget::shaderChangeReflectionScale(double s){
      _shader[3]->Enable();
      _shader[3]->SetUniformVariable1f("scale_factor", s);
      _shader[3]->Disable();
      updateGL();
    }
    void GLWidget::shaderChangeReflectionSmoothing(double s){
      _shader[3]->Enable();
      _shader[3]->SetUniformVariable1f("smoothing", s);
      _shader[3]->Disable();
      updateGL();

    }
    void GLWidget::shaderChangeReflectionShading(double s){
      _shader[3]->Enable();
      _shader[3]->SetUniformVariable1f("shading", s);
      _shader[3]->Disable();
      updateGL();
    }

    void GLWidget:: shaderChangeToonOutline(){
      double x,y,z,alpha;
      x = _sideWidget->ShaderToonOutline0->value();
      y = _sideWidget->ShaderToonOutline1->value();
      z = _sideWidget->ShaderToonOutline2->value();
      alpha = _sideWidget->ShaderToonOutline3->value();

      _shader[2]->Enable();
      _shader[2]->SetUniformVariable4f("default_outline_color",x, y, z, alpha);
      _shader[2]->Disable();
      updateGL();
    }

    void GLWidget::togglePatchUIso(bool on){
      if(compositePatch){
          int patchIndex = _sideWidget->PatchIsoPatchIndex->value();
          int lineCount = _sideWidget->PatchIsoUcount->value();
          if(on){
              compositePatch->setULines(patchIndex,lineCount);
            }else{
              compositePatch->clearULines(patchIndex);
            }
          updateGL();
        }
    }

    void GLWidget::togglePatchVIso(bool on){
      if(compositePatch){
          int patchIndex = _sideWidget->PatchIsoPatchIndex->value();
          int lineCount = _sideWidget->PatchIsoVcount->value();
          if(on){
              compositePatch->setVLines(patchIndex,lineCount);
            }else{
              compositePatch->clearVLines(patchIndex);
            }
          updateGL();
        }
    }

    void GLWidget::saveArcs(){
      if(compositeCurve){
          string empty = "";
          if(_sideWidget->ArcFilename->text().toUtf8().constData() != empty){
            compositeCurve->saveToFile(_sideWidget->ArcFilename->text().toUtf8().constData());
          }
        }
    }
    void GLWidget::readArcs(){      
      if(!compositeCurve){
          compositeCurve = new HyperbolicCompositeCurve3(MAX_CURVE_COUNT);
      }
      string empty = "";
      if(_sideWidget->ArcFilenameToRead->text().toUtf8().constData() != empty){
         compositeCurve->readFromFile(_sideWidget->ArcFilenameToRead->text().toUtf8().constData());
         updateGL();
      }
    }
    void GLWidget::savePatches(){
      if(compositePatch){
          string empty = "";
          if(_sideWidget->PatchFilename->text().toUtf8().constData() != empty){
            compositePatch->saveToFile(_sideWidget->PatchFilename->text().toUtf8().constData());
          }
        }
    }
    void GLWidget::readPatches(){
      if(!compositePatch){
          compositePatch = new HyperbolicCompositePatch3(MAX_PATCH_COUNT);
          patchMaterials[0]=MatFBEmerald;
          patchMaterials[1]=MatFBGold;
          patchMaterials[2]=MatFBBrass;
      }
      string empty = "";
      if(_sideWidget->PatchFilenameToRead->text().toUtf8().constData() != empty){
         compositePatch->readFromFile(_sideWidget->PatchFilenameToRead->text().toUtf8().constData());
         updateGL();
      }
    }

    void GLWidget::applyTexture(){
      if(compositePatch){
        int currentPatch = _sideWidget->PatchTexturePatchIndex->value();
        string textureFilename = _sideWidget->PatchTextureFilename->text().toUtf8().constData();
        string empty = "";
        if(textureFilename != empty){
          compositePatch->applyTexture(currentPatch,textureFilename);
          updateGL();
        }
      }
    }

    void GLWidget::disableTexture(){
      if(compositePatch){
        int currentPatch = _sideWidget->PatchTexturePatchIndex->value();
          compositePatch->disableTexture(currentPatch);
          updateGL();
      }
    }
}

