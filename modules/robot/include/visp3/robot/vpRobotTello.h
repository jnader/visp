/****************************************************************************
 *
 * ViSP, open source Visual Servoing Platform software.
 * Copyright (C) 2005 - 2022 by Inria. All rights reserved.
 *
 * This software is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file LICENSE.txt at the root directory of this source
 * distribution for additional information about the GNU GPL.
 *
 * For using ViSP with software that can not be combined with the GNU
 * GPL, please contact Inria about acquiring a ViSP Professional
 * Edition License.
 *
 * See http://visp.inria.fr for more information.
 *
 * This software was developed at:
 * Inria Rennes - Bretagne Atlantique
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * France
 *
 * If you have questions regarding the use of this file, please contact
 * Inria at visp@inria.fr
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Description:
 * Interface for the DJI Ryze tello drone.
 *
 * Authors:
 * Joudy Nader
 *
 *****************************************************************************/

#ifndef _vpRobotTello_h_
#define _vpRobotTello_h_

#include <visp3/core/vpConfig.h>

#include <visp3/core/vpImage.h>
#include <visp3/core/vpUDPClient.h>
#include <visp3/core/vpUDPServer.h>

#include <mutex>
#include <signal.h>
#include <string>

/*!
  \class vpRobotTello

  \ingroup group_robot_real_drone

  Interface for DJI Tello SDK, allowing to control DJI Ryze Tello drone.
*/
class VISP_EXPORT vpRobotTello
{
public:
  vpRobotTello();
  // vpRobotTello(bool verbose = false, bool setDefaultSettings = true, std::string ipAddress = "192.168.10.1",
  //               int discoveryPort = 8889);
  virtual ~vpRobotTello();

  std::string getIpAddress();
  int getDiscoveryPort();

  void doFlatTrim();
  unsigned int getBatteryLevel();
  void setVerbose(bool verbose);
  void resetAllSettings();

  bool isFlying();
  bool isHovering();
  bool isLanded();
  bool isRunning();
  bool isStreaming();

  void cutMotors();
  double getMaxTilt();
  void setMaxTilt(double maxTilt);
  void setPitch(int value);
  void setPosition(float dX, float dY, float dZ, float dPsi, bool blocking);
  void setPosition(const vpHomogeneousMatrix &M, bool blocking);
  void setRoll(int value);
  void setVelocity(const vpColVector &vel, double delta_t);
  void setVerticalSpeed(int value);
  void setYawSpeed(int value);
  void stopMoving();
  void takeOff(bool blocking = true);
  static void land();

  void getGrayscaleImage(vpImage<unsigned char> &I);
  void getRGBaImage(vpImage<vpRGBa> &I);
  int getVideoHeight();
  int getVideoWidth();
  void setExposure(float expo);
  void setStreamingMode(int mode);
  void setVideoResolution(int mode);
  void setVideoStabilisationMode(int mode);
  void startStreaming();
  void stopStreaming();

  double getCameraHorizontalFOV() const;
  double getCurrentCameraPan() const;
  double getMaxCameraPan() const;
  double getMinCameraPan() const;
  double getCurrentCameraTilt() const;
  double getMaxCameraTilt() const;
  double getMinCameraTilt() const;
  void setCameraOrientation(double tilt, double pan, bool blocking = false);
  void setCameraPan(double pan, bool blocking = false);
  void setCameraTilt(double tilt, bool blocking = false);

private:
  vpUDPClient m_control_client; // UDP Client to command and receive response
  vpUDPServer *m_state_server, *m_video_server; // UDP servers to receive both state and video stream
  std::string m_ipAddress; ///< Ip address of the drone to discover on the network
  int m_control_discoveryPort;     ///< Port of the drone to discover on the network
  int m_state_discoveryPort;     ///< Port of the drone to discover on the network
  int m_video_discoveryPort;     ///< Port of the drone to discover on the network

  static bool m_running; ///< Used for checking if the drone is running ie if successfully connected and ready to
                         ///< receive commands

  bool m_exposureSet;        ///< Used to know if exposure compensation has been set
  bool m_flatTrimFinished;   ///< Used to know when the drone has finished a flat trim
  bool m_relativeMoveEnded;  ///< Used to know when the drone has ended a relative move
  bool m_videoResolutionSet; ///< Used to know if video resolution has been set
  bool m_streamingStarted;   ///< Used to know if the streaming has been started
  bool m_streamingModeSet;   ///< Used to know if the streaming mode has been set
  bool m_settingsReset;      ///< Used to know when the drone a finished the settings reset

  bool m_update_codec_params;               ///< Used to know if codec parameters need to be updated
  std::vector<uint8_t> m_codec_params_data; ///< Used to store codec parameters when they need to be updated

  unsigned int m_batteryLevel; ///< Percentage of battery remaining
  double m_maxTilt;            ///< Max pitch and roll value of the drone

  double m_cameraHorizontalFOV; ///< Camera horizontal FOV

  double m_currentCameraTilt; ///< Current tilt of the camera
  double m_minCameraTilt;     ///< Minimum possible tilt of the camera
  double m_maxCameraTilt;     ///< Maximum possible tilt of the camera

  double m_currentCameraPan; ///< Current pan of the camera
  double m_minCameraPan;     ///< Minimum possible tilt of the camera
  double m_maxCameraPan;     ///< Maximum possible tilt of the camera

  void cleanUp();
  void setupCallbacks();
  void startController();

};

#endif //#ifndef _vpRobotTello_h_
