#include "systempreferences.h"

bool SystemPreferences::_startMaximized = false;
bool SystemPreferences::_autoLoadPlugins = true;
unsigned short int SystemPreferences::_modelAtStart = 1;
std::string SystemPreferences::_modelfilename = "../../../../../../../models/Smart_Delay.gen";  //"";
// "../../../../../models/Smart_Delay.gen"

bool SystemPreferences::load()
{
    return false;
}

bool SystemPreferences::save()
{
    return false;
}

bool SystemPreferences::startMaximized()
{
    return _startMaximized;
}

void SystemPreferences::setStartMaximized(bool newStartMaximized)
{
    _startMaximized = newStartMaximized;
}

bool SystemPreferences::autoLoadPlugins()
{
    return _autoLoadPlugins;
}

void SystemPreferences::setAutoLoadPlugins(bool newAutoLoadPlugins)
{
    _autoLoadPlugins = newAutoLoadPlugins;
}

unsigned short SystemPreferences::modelAtStart()
{
    return _modelAtStart;
}

void SystemPreferences::setModelAtStart(unsigned short newModelAtStart)
{
    if (newModelAtStart>=0 && newModelAtStart<=2)
        _modelAtStart = newModelAtStart;
}

std::string SystemPreferences::modelfilename()
{
    return _modelfilename;
}

void SystemPreferences::setModelfilename(const std::string &newModelfilename)
{
    _modelfilename = newModelfilename;
}
