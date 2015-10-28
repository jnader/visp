/****************************************************************************
 *
 * $Id: servoSimuPoint3DCamVelocity.cpp 2457 2010-01-07 10:41:18Z nmelchio $
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
 * Simulation of a 3D visual servoing on a 3D point.
 *
 * Authors:
 * Eric Marchand
 * Fabien Spindler
 *
 *****************************************************************************/


/*!
  \example servoSimuPoint3DCamVelocity.cpp
  Simulation of a 3D visual servoing:
  - servo a 3D point,
  - eye-in-hand control law,
  - velocity computed in the camera frame,
  - no display.

*/


#include <stdlib.h>
#include <stdio.h>

#include <visp3/core/vpFeaturePoint3D.h>
#include <visp3/core/vpHomogeneousMatrix.h>
#include <visp3/core/vpMath.h>
#include <visp3/core/vpParseArgv.h>
#include <visp3/core/vpPoint.h>
#include <visp3/vs/vpServo.h>
#include <visp3/robot/vpSimulatorCamera.h>

// List of allowed command line options
#define GETOPTARGS	"h"

void usage(const char *name, const char *badparam);
bool getOptions(int argc, const char **argv);

/*!

Print the program options.

  \param name : Program name.
  \param badparam : Bad parameter name.

*/
void usage(const char *name, const char *badparam)
{
  fprintf(stdout, "\n\
Simulation of a 3D visual servoing:\n\
- servo a 3D point,\n\
- eye-in-hand control law,\n\
- velocity computed in the camera frame,\n\
- without display.\n\
          \n\
SYNOPSIS\n\
  %s [-h]\n", name);

  fprintf(stdout, "\n\
OPTIONS:                                               Default\n\
                  \n\
  -h\n\
     Print the help.\n");

  if (badparam)
    fprintf(stdout, "\nERROR: Bad parameter [%s]\n", badparam);
}

/*!

Set the program options.

  \param argc : Command line number of parameters.
  \param argv : Array of command line parameters.
  \return false if the program has to be stopped, true otherwise.

*/
bool getOptions(int argc, const char **argv)
{
  const char *optarg_;
  int	c;
  while ((c = vpParseArgv::parse(argc, argv, GETOPTARGS, &optarg_)) > 1) {

    switch (c) {
    case 'h': usage(argv[0], NULL); return false; break;

    default:
      usage(argv[0], optarg_);
      return false; break;
    }
  }

  if ((c == 1) || (c == -1)) {
    // standalone param or error
    usage(argv[0], NULL);
    std::cerr << "ERROR: " << std::endl;
    std::cerr << "  Bad argument " << optarg_ << std::endl << std::endl;
    return false;
  }

  return true;
}

int
main(int argc, const char ** argv)
{
  try {
    // Read the command line options
    if (getOptions(argc, argv) == false) {
      exit (-1);
    }

    vpServo task ;
    vpSimulatorCamera robot ;

    std::cout << std::endl ;
    std::cout << "-------------------------------------------------------" << std::endl ;
    std::cout << " Test program for vpServo "  <<std::endl ;
    std::cout << " Eye-in-hand task control, velocity computed in the camera frame" << std::endl ;
    std::cout << " Simulation " << std::endl ;
    std::cout << " task : servo a 3D point " << std::endl ;
    std::cout << "-------------------------------------------------------" << std::endl ;
    std::cout << std::endl ;

    // sets the initial camera location
    vpHomogeneousMatrix cMo ;
    cMo[0][3] = 0.1 ;
    cMo[1][3] = 0.2 ;
    cMo[2][3] = 2 ;
    // Compute the position of the object in the world frame
    vpHomogeneousMatrix wMc, wMo;
    robot.getPosition(wMc) ;
    wMo = wMc * cMo;

    // sets the point coordinates in the world frame
    vpPoint point(0,0,0) ;

    // computes the point coordinates in the camera frame
    point.track(cMo) ;

    std::cout << "Point coordinates in the camera frame: " << point.cP.t() ;

    vpFeaturePoint3D p ;
    p.buildFrom(point) ;

    // sets the desired position of the point
    vpFeaturePoint3D pd ;
    pd.set_XYZ(0,0,1) ;

    // define the task
    // - we want an eye-in-hand control law
    // - robot is controlled in the camera frame
    task.setServo(vpServo::EYEINHAND_CAMERA) ;

    // we want to see a point on a point
    std::cout << std::endl ;
    task.addFeature(p,pd) ;

    // set the gain") ;
    task.setLambda(1) ;

    // Display task information
    task.print() ;

    unsigned int iter=0 ;
    // loop
    while(iter++<200)
    {
      std::cout << "---------------------------------------------" << iter <<std::endl ;
      vpColVector v ;

      // get the robot position
      robot.getPosition(wMc) ;
      // Compute the position of the camera wrt the object frame
      cMo = wMc.inverse() * wMo;

      // new point position
      point.track(cMo) ;
      p.buildFrom(point) ;
      //   std::cout << p.cP.t() ;
      //   std::cout << (p.get_s()).t() ;

      // compute the control law
      v = task.computeControlLaw() ;
      // send the camera velocity to the controller
      robot.setVelocity(vpRobot::CAMERA_FRAME, v) ;

      std::cout << "|| s - s* || = " << ( task.getError() ).sumSquare() <<std::endl ;
    }

    // Display task information
    task.print() ;
    task.kill();
    return 0;
  }
  catch(vpException e) {
    std::cout << "Catch a ViSP exception: " << e << std::endl;
    return 1;
  }
}

