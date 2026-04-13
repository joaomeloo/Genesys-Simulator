/*
 * Smart_ExtendedFSM.h
 *
 * Created on: 2025.2
 * Authors: André Rocco, Samantha Costa
 */

#ifndef SMART_EXTENDEDFSM_H
#define SMART_EXTENDEDFSM_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_Old_ExtendedFSM : public BaseGenesysTerminalApplication {
public:
	Smart_Old_ExtendedFSM();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* SMART_EXTENDEDFSM_H */
