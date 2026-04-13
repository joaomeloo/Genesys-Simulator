#include "systempreferences.h"
#include <QSettings>

bool SystemPreferences::_startMaximized = true;
bool SystemPreferences::_autoLoadPlugins = true;
bool SystemPreferences::_checkSystemPackagesAtStart = true;
unsigned short int SystemPreferences::_openModelAtStart = 1;
std::string SystemPreferences::_modelfilenameToOpen = "";
bool SystemPreferences::_darkMode = false;

bool SystemPreferences::load()
{
    QSettings settings("GenESyS", "Simulator");
    _startMaximized = settings.value("startMaximized", false).toBool();
    _autoLoadPlugins = settings.value("autoLoadPlugins", true).toBool();
    _modelAtStart = settings.value("modelAtStart", 2).toUInt();
    _modelfilename = settings.value("modelfilename", "../../../../../../../models/Smart_Delay.gen").toString().toStdString();
    _darkMode = settings.value("darkMode", false).toBool();
    return true;
}

bool SystemPreferences::save()
{
    QSettings settings("GenESyS", "Simulator");
    settings.setValue("startMaximized", _startMaximized);
    settings.setValue("autoLoadPlugins", _autoLoadPlugins);
    settings.setValue("modelAtStart", _modelAtStart);
    settings.setValue("modelfilename", QString::fromStdString(_modelfilename));
    settings.setValue("darkMode", _darkMode);
    settings.sync();
    return true;
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
    return _openModelAtStart;
}

void SystemPreferences::setModelAtStart(unsigned short newModelAtStart)
{
    if (newModelAtStart>=0 && newModelAtStart<=2)
        _openModelAtStart = newModelAtStart;
}

std::string SystemPreferences::modelfilename()
{
    return _modelfilenameToOpen;
}

void SystemPreferences::setModelfilename(const std::string &newModelfilename)
{
    _modelfilenameToOpen = newModelfilename;
}

bool SystemPreferences::checkSystemPackagesAtStart()
{
    return _checkSystemPackagesAtStart;
}

void SystemPreferences::setCheckSystemPackagesAtStart(bool newCheckSystemPackagesAtStart)
{
    _checkSystemPackagesAtStart = newCheckSystemPackagesAtStart;
}

bool SystemPreferences::darkMode()
{
    return _darkMode;
}

void SystemPreferences::setDarkMode(bool newDarkMode)
{
    _darkMode = newDarkMode;
}

bool SystemPreferences::darkMode()
{
    return _darkMode;
}

void SystemPreferences::setDarkMode(bool newDarkMode)
{
    _darkMode = newDarkMode;
}
