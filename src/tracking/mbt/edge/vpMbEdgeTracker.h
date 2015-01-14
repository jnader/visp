/****************************************************************************
 *
 * $Id$
 *
 * This file is part of the ViSP software.
 * Copyright (C) 2005 - 2014 by INRIA. All rights reserved.
 * 
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * ("GPL") version 2 as published by the Free Software Foundation.
 * See the file LICENSE.txt at the root directory of this source
 * distribution for additional information about the GNU GPL.
 *
 * For using ViSP with software that can not be combined with the GNU
 * GPL, please contact INRIA about acquiring a ViSP Professional 
 * Edition License.
 *
 * See http://www.irisa.fr/lagadic/visp/visp.html for more information.
 * 
 * This software was developed at:
 * INRIA Rennes - Bretagne Atlantique
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * France
 * http://www.irisa.fr/lagadic
 *
 * If you have questions regarding the use of this file, please contact
 * INRIA at visp@inria.fr
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 * Description:
 * Make the complete tracking of an object by using its CAD model
 *
 * Authors:
 * Nicolas Melchior
 * Romain Tallonneau
 * Eric Marchand
 *
 *****************************************************************************/

/*!
 \file vpMbEdgeTracker.h
 \brief Make the complete tracking of an object by using its CAD model.
*/

#ifndef vpMbEdgeTracker_HH
#define vpMbEdgeTracker_HH

#include <visp/vpPoint.h>
#include <visp/vpMbTracker.h>
#include <visp/vpMe.h>
#include <visp/vpMbtMeLine.h>
#include <visp/vpMbtDistanceLine.h>
#include <visp/vpMbtDistanceCircle.h>
#include <visp/vpMbtDistanceCylinder.h>
#include <visp/vpXmlParser.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <list>

#if defined(VISP_HAVE_COIN)
//Inventor includes
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/VRMLnodes/SoVRMLIndexedFaceSet.h>
#include <Inventor/VRMLnodes/SoVRMLIndexedLineSet.h>
#include <Inventor/VRMLnodes/SoVRMLCoordinate.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/misc/SoChildList.h>
#include <Inventor/actions/SoGetMatrixAction.h>
#include <Inventor/actions/SoGetPrimitiveCountAction.h>
#include <Inventor/actions/SoToVRML2Action.h>
#include <Inventor/VRMLnodes/SoVRMLGroup.h>
#include <Inventor/VRMLnodes/SoVRMLShape.h>
#endif

#ifdef VISP_HAVE_OPENCV
#  if VISP_HAVE_OPENCV_VERSION >= 0x020101
#    include <opencv2/core/core.hpp>
#    include <opencv2/imgproc/imgproc.hpp>
#    include <opencv2/imgproc/imgproc_c.h>
#  else
#    include <cv.h>
#  endif
#endif

/*!
  \class vpMbEdgeTracker
  \ingroup ModelBasedTracking 
  \brief Make the complete tracking of an object by using its CAD model.

  This class allows to track an object or a scene given its 3D model. A
  video can be found in the \e http://www.irisa.fr/lagadic/visp/computer-vision.html  web page. The \ref tutorial-tracking-mb is also a good starting point to use this class.

  The tracker requires the knowledge of the 3D model that could be provided in a vrml
  or in a cao file. The cao format is described in loadCAOModel().
  It may also use an xml file used to tune the behavior of the tracker and an
  init file used to compute the pose at the very first image.

  The following code shows the simplest way to use the tracker.

\code
#include <visp/vpMbEdgeTracker.h>
#include <visp/vpImage.h>
#include <visp/vpImageIo.h>
#include <visp/vpHomogeneousMatrix.h>
#include <visp/vpCameraParameters.h>
#include <visp/vpException.h>
#include <visp/vpDisplayX.h>

int main()
{
  vpMbEdgeTracker tracker; // Create a model based tracker.
  vpImage<unsigned char> I;
  vpHomogeneousMatrix cMo; // Pose computed using the tracker. 
  vpCameraParameters cam;
  
  // Acquire an image
  vpImageIo::read(I, "cube.pgm");
  
#if defined VISP_HAVE_X11
  vpDisplayX display;
  display.init(I,100,100,"Mb Edge Tracker");
#endif

#if defined VISP_HAVE_XML2
  tracker.loadConfigFile("cube.xml"); // Load the configuration of the tracker
#endif
  tracker.getCameraParameters(cam);   // Get the camera parameters used by the tracker (from the configuration file).
  tracker.loadModel("cube.cao");      // Load the 3d model in cao format. No 3rd party library is required
  tracker.initClick(I, "cube.init");  // Initialise manually the pose by clicking on the image points associated to the 3d points contained in the cube.init file.

  while(true){
    // Acquire a new image
    vpDisplay::display(I);
    tracker.track(I);     // Track the object on this image
    tracker.getPose(cMo); // Get the pose
    
    tracker.display(I, cMo, cam, vpColor::darkRed, 1); // Display the model at the computed pose.
    vpDisplay::flush(I);
  }

  // Cleanup memory allocated by xml library used to parse the xml config file in vpMbEdgeTracker::loadConfigFile()
  vpXmlParser::cleanup();

  return 0;
}
\endcode

  For application with large inter-images displacement, multi-scale tracking is also possible, by setting the number of scales used and by activating (or not) them 
  using a vector of booleans, as presented in the following code:

\code
  ...
  vpHomogeneousMatrix cMo; // Pose computed using the tracker.
  vpCameraParameters cam;

  std::vector< bool > scales(3); //Three scales used
  scales.push_back(true); //First scale : active
  scales.push_back(false); //Second scale (/2) : not active
  scales.push_back(true); //Third scale (/4) : active
  tracker.setScales(scales); // Set active scales for multi-scale tracking

  tracker.loadConfigFile("cube.xml"); // Load the configuration of the tracker
  tracker.getCameraParameters(cam); // Get the camera parameters used by the tracker (from the configuration file).
  ...
  // Cleanup memory allocated by xml library used to parse the xml config file in vpMbEdgeTracker::loadConfigFile()
  vpXmlParser::cleanup();
\endcode

  The tracker can also be used without display, in that case the initial pose
  must be known (object always at the same initial pose for example) or computed
  using another method:

\code
#include <visp/vpMbEdgeTracker.h>
#include <visp/vpImage.h>
#include <visp/vpHomogeneousMatrix.h>
#include <visp/vpCameraParameters.h>
#include <visp/vpImageIo.h>

int main()
{
  vpMbEdgeTracker tracker; // Create a model based tracker.
  vpImage<unsigned char> I;
  vpHomogeneousMatrix cMo; // Pose used in entry (has to be defined), then computed using the tracker. 
  
  //acquire an image
  vpImageIo::read(I, "cube.pgm"); // Example of acquisition

#if defined VISP_HAVE_XML2
  tracker.loadConfigFile("cube.xml"); // Load the configuration of the tracker
#endif
  tracker.loadModel("cube.cao"); // load the 3d model, to read .wrl model coin is required, if coin is not installed .cao file can be used.
  tracker.initFromPose(I, cMo); // initialize the tracker with the given pose.

  while(true){
    // acquire a new image
    tracker.track(I); // track the object on this image
    tracker.getPose(cMo); // get the pose 
  }
  
  // Cleanup memory allocated by xml library used to parse the xml config file in vpMbEdgeTracker::loadConfigFile()
  vpXmlParser::cleanup();

  return 0;
}
\endcode

  Finally it can be used not to track an object but just to display a model at a
  given pose:

\code
#include <visp/vpMbEdgeTracker.h>
#include <visp/vpImage.h>
#include <visp/vpImageIo.h>
#include <visp/vpHomogeneousMatrix.h>
#include <visp/vpCameraParameters.h>
#include <visp/vpDisplayX.h>

int main()
{
  vpMbEdgeTracker tracker; // Create a model based tracker.
  vpImage<unsigned char> I;
  vpHomogeneousMatrix cMo; // Pose used to display the model. 
  vpCameraParameters cam;
  
  // Acquire an image
  vpImageIo::read(I, "cube.pgm");
  
#if defined VISP_HAVE_X11
  vpDisplayX display;
  display.init(I,100,100,"Mb Edge Tracker");
#endif

#if defined VISP_HAVE_XML2
  tracker.loadConfigFile("cube.xml"); // Load the configuration of the tracker
#endif
  tracker.getCameraParameters(cam); // Get the camera parameters used by the tracker (from the configuration file).
  tracker.loadModel("cube.cao"); // load the 3d model, to read .wrl model coin is required, if coin is not installed .cao file can be used.

  while(true){
    // acquire a new image
    // Get the pose using any method
    vpDisplay::display(I);
    tracker.display(I, cMo, cam, vpColor::darkRed, 1, true); // Display the model at the computed pose.
    vpDisplay::flush(I);
  }
  
  // Cleanup memory allocated by xml library used to parse the xml config file in vpMbEdgeTracker::loadConfigFile()
  vpXmlParser::cleanup();

  return 0;
}
\endcode

*/

class VISP_EXPORT vpMbEdgeTracker: virtual public vpMbTracker
{
  protected :
    
    /*! If this flag is true, the interaction matrix
     extracted from the feature set is computed at each
     iteration in the visual servoing loop.
    */
    int compute_interaction;
    //! The gain of the virtual visual servoing stage. 
    double lambda;
    
    //! The moving edges parameters. 
    vpMe me;
    //! Vector of list of all the lines tracked (each line is linked to a list of moving edges). Each element of the vector is for a scale (element 0 = level 0 = no subsampling).
    std::vector< std::list< vpMbtDistanceLine*> > lines;

    //! Vector of the tracked circles.
    std::vector< std::list< vpMbtDistanceCircle*> > circles;

    //! Vector of the tracked cylinders.
    std::vector< std::list< vpMbtDistanceCylinder*> > cylinders;

    //! Index of the polygon to add, and total number of polygon extracted so far. 
    unsigned int nline;

    //! Index of the circle to add, and total number of circles extracted so far.
    unsigned int ncircle;

    //! Index of the cylinder to add, and total number of cylinders extracted so far.
    unsigned int ncylinder;
    
    //! Number of polygon (face) currently visible. 
    unsigned int nbvisiblepolygone;
    
    //! Percentage of good points over total number of points below which tracking is supposed to have failed.
    double percentageGdPt;
    
    //! Vector of scale level to use for the multi-scale tracking.
    std::vector<bool> scales;
    
    //! Pyramid of image associated to the current image. This pyramid is computed in the init() and in the track() methods.
    std::vector< const vpImage<unsigned char>* > Ipyramid;
    
    //! Current scale level used. This attribute must not be modified outside of the downScale() and upScale() methods, as it used to specify to some methods which set of distanceLine use. 
    unsigned int scaleLevel;

public:
  
  vpMbEdgeTracker(); 
  virtual ~vpMbEdgeTracker();
  
  void display(const vpImage<unsigned char>& I, const vpHomogeneousMatrix &cMo, const vpCameraParameters &cam,
               const vpColor& col , const unsigned int thickness=1, const bool displayFullModel = false);
  void display(const vpImage<vpRGBa>& I, const vpHomogeneousMatrix &cMo, const vpCameraParameters &cam,
               const vpColor& col , const unsigned int thickness=1, const bool displayFullModel = false);

  inline double getFirstThreshold() const { return percentageGdPt;}
  
  /*!
    Get the value of the gain used to compute the control law.
    
    \return the value for the gain.
  */
  virtual inline double getLambda() const {return lambda;}
  
  void getLline(std::list<vpMbtDistanceLine *>& linesList, const unsigned int level = 0);
  void getLcircle(std::list<vpMbtDistanceCircle *>& circlesList, const unsigned int level = 0);
  void getLcylinder(std::list<vpMbtDistanceCylinder *>& cylindersList, const unsigned int level = 0);

  /*!
    Get the moving edge parameters.
    
    \return an instance of the moving edge parameters used by the tracker.
  */
  inline void getMovingEdge(vpMe &p_me ) const { p_me = this->me;}

  unsigned int getNbPoints(const unsigned int level=0) const;
  
  /*!
    Return the scales levels used for the tracking. 
    
    \return The scales levels used for the tracking. 
  */
  std::vector<bool> getScales() const {return scales;}
  
  void loadConfigFile(const std::string &configFile);
  void loadConfigFile(const char* configFile);
  void reInitModel(const vpImage<unsigned char>& I, const std::string &cad_name, const vpHomogeneousMatrix& cMo_,
		  const bool verbose=false);
  void reInitModel(const vpImage<unsigned char>& I, const char* cad_name, const vpHomogeneousMatrix& cMo,
		  const bool verbose=false);
  void resetTracker();
  
  /*!
    Set the camera parameters.

    \param camera : the new camera parameters
  */
  virtual void setCameraParameters(const vpCameraParameters& camera) {
    this->cam = camera;

    for (unsigned int i = 0; i < scales.size(); i += 1){
      if(scales[i]){
        for(std::list<vpMbtDistanceLine*>::const_iterator it=lines[i].begin(); it!=lines[i].end(); ++it){
          (*it)->setCameraParameters(cam);
        }

        for(std::list<vpMbtDistanceCylinder*>::const_iterator it=cylinders[i].begin(); it!=cylinders[i].end(); ++it){
          (*it)->setCameraParameters(cam);
        }

        for(std::list<vpMbtDistanceCircle*>::const_iterator it=circles[i].begin(); it!=circles[i].end(); ++it){
          (*it)->setCameraParameters(cam);
        }
      }
    }
  }

  virtual void setClipping(const unsigned int &flags);

  virtual void setFarClippingDistance(const double &dist);

  virtual void setNearClippingDistance(const double &dist);

  /*!
    Use Ogre3D for visibility tests

    \warning This function has to be called before the initialization of the tracker.

    \param v : True to use it, False otherwise
  */
  virtual void setOgreVisibilityTest(const bool &v){
      vpMbTracker::setOgreVisibilityTest(v);
#ifdef VISP_HAVE_OGRE
      faces.getOgreContext()->setWindowName("MBT Edge");
#endif
  }

  /*!
    Set the first threshold used to check if the tracking failed. It corresponds to the percentage of good point which is necessary.
    
    The condition which has to be be satisfied is the following : \f$ nbGoodPoint > threshold1 \times (nbGoodPoint + nbBadPoint)\f$.
    
    The threshold is ideally between 0 and 1.
    
    \param threshold1 : The new value of the threshold. 
  */
  void setFirstThreshold(const double  threshold1) {percentageGdPt = threshold1;}
  
  /*!
    Set the value of the gain used to compute the control law.
    
    \param gain : the desired value for the gain.
  */
  virtual inline void setLambda(const double gain) {this->lambda = gain;}
  
  void setMovingEdge(const vpMe &me);

  virtual void setPose(const vpImage<unsigned char> &I, const vpHomogeneousMatrix& cdMo);
  
  void setScales(const std::vector<bool>& _scales);

  void track(const vpImage<unsigned char> &I);

protected:
  bool samePoint(const vpPoint &P1, const vpPoint &P2);
  void addCircle(const vpPoint &P1, const vpPoint &P2, const vpPoint &P3, const double r, int idFace = -1, const std::string& name = "");
  void addCylinder(const vpPoint &P1, const vpPoint &P2, const double r, int idFace = -1, const std::string& name = "");
  void addLine(vpPoint &p1, vpPoint &p2, int polygon = -1, std::string name = "");
  void addPolygon(vpMbtPolygon &p) ;
  void cleanPyramid(std::vector<const vpImage<unsigned char>* >& _pyramid);
  void computeVVS(const vpImage<unsigned char>& _I);
  void downScale(const unsigned int _scale);
  void init(const vpImage<unsigned char>& I);
  virtual void initCircle(const vpPoint& p1, const vpPoint &p2, const vpPoint &p3, const double radius,
                          const int idFace=0, const std::string &name="");
  virtual void initCylinder(const vpPoint& p1, const vpPoint &p2, const double radius, const int idFace=0,
                            const std::string &name="");
  virtual void initFaceFromCorners(vpMbtPolygon &polygon);
  virtual void initFaceFromLines(vpMbtPolygon &polygon);
  void initMovingEdge(const vpImage<unsigned char> &I, const vpHomogeneousMatrix &_cMo) ;
  void initPyramid(const vpImage<unsigned char>& _I, std::vector<const vpImage<unsigned char>* >& _pyramid);
  void reInitLevel(const unsigned int _lvl);
  void reinitMovingEdge(const vpImage<unsigned char> &I, const vpHomogeneousMatrix &_cMo);
  void removeCircle(const std::string& name);
  void removeCylinder(const std::string& name);
  void removeLine(const std::string& name);
  void testTracking();
  void trackMovingEdge(const vpImage<unsigned char> &I) ;
  void updateMovingEdge(const vpImage<unsigned char> &I) ;
  void upScale(const unsigned int _scale); 
  void visibleFace(const vpImage<unsigned char> &_I, const vpHomogeneousMatrix &_cMo, bool &newvisibleline) ; 
  
#ifdef VISP_BUILD_DEPRECATED_FUNCTIONS
  /*!
    @name Deprecated functions
  */
  vp_deprecated void visibleFace(const vpHomogeneousMatrix &_cMo, bool &newvisibleline);

public:
  /*!
    \deprecated Use vpMbTracker::setDisplayFeatures() instead.
    Enable to display the points along the line with a color corresponding to their state.

    - If green : The vpMeSite is a good point.
    - If blue : The point is removed because of the vpMeSite tracking phase (contrast problem).
    - If purple : The point is removed because of the vpMeSite tracking phase (threshold problem).
    - If red : The point is removed because of the robust method in the virtual visual servoing.

    \param displayMe : set it to true to display the points.
  */
  vp_deprecated void setDisplayMovingEdges(const bool displayMe) {displayFeatures = displayMe;}
#endif
};

#endif

