#include <visp3/robot/vpRobotTello.h>
#include <iostream>

int main(int argc, char** argv)
{
    int val = 0;
    std::cin >> val;
    vpRobotTello tello = vpRobotTello();

    vpTime::sleepMs(5000);

    return 0;
}