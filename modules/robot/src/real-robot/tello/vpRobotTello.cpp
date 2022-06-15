#include <visp3/robot/vpRobotTello.h>
#include <visp3/core/vpUDPClient.h>
#include <visp3/core/vpUDPServer.h>
#include <iostream>

vpRobotTello::vpRobotTello()
{
    // Setting up default parameters.
    m_ipAddress = "192.168.10.1";
    m_control_discoveryPort = 8889;
    m_state_discoveryPort = 8890;
    m_video_discoveryPort = 11111;

    // Initialising UDP servers.
    
    // m_video_server = vpUDPServer(m_ipAddress, m_video_discoveryPort);

    // The following is the procedure to connect to Tello drone as described in User manual of SDK 2.0
    // Step 1: Initialising UDP Client.
    m_control_client = vpUDPClient(m_ipAddress, m_control_discoveryPort);

    // Step 2: Send "command" to intiate SDK mode.
    m_control_client.send("command");
    // if(! m_control_client.send("command") == strlen("command"))
    //     std::cout << "Couldn't initiate SDK mode" << std::endl;

    // Step3: Receive Tello state. 
    // m_state_server = vpUDPServer(m_ipAddress, m_state_discoveryPort);
    // m_state_server.receive()

}

// vpRobotTello::vpRobotTello(bool verbose, bool setDefaultSettings, std::string ipAddress,
//                 int discoveryPort)
// {
//     vpUDPClient client;
//     client.init(ipAddress, discoveryPort);
//     if(client.send("command") != strlen("command"))
//         std::cout << "error" << std::endl;

//     else
//         std::cout << "command" << std::endl;
// }

vpRobotTello::~vpRobotTello()
{
}