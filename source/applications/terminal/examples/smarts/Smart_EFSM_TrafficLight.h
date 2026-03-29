/*
 * Smart_EFSM_TrafficLight.h
 *
 * Created on: 2025.2
 * Authors: André Rocco, Samantha Costa
 */

#ifndef SMART_EFSM_TRAFFICLIGHT_H
#define SMART_EFSM_TRAFFICLIGHT_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_EFSM_TrafficLight : public BaseGenesysTerminalApplication {
public:
    Smart_EFSM_TrafficLight();
public:
    virtual int main(int argc, char** argv);
};

#endif /* SMART_EFSM_TRAFFICLIGHT_H */
