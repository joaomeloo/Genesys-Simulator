#ifndef SYSTEMPREFERENCES_H
#define SYSTEMPREFERENCES_H

#include <string>
//#include <iostream>

class SystemPreferences
{
public:
    static bool load();
    static bool save();
    static bool startMaximized();
    static void setStartMaximized(bool newStartMaximized);

    static bool autoLoadPlugins();
    static void setAutoLoadPlugins(bool newAutoLoadPlugins);

    static unsigned short modelAtStart();
    static void setModelAtStart(unsigned short newModelAtStart);

    static std::string modelfilename();
    static void setModelfilename(const std::string &newModelfilename);

    static bool darkMode();
    static void setDarkMode(bool newDarkMode);

private:
    SystemPreferences(){};
private:
    static bool _startMaximized;
    static bool _autoLoadPlugins;
    static unsigned short int _modelAtStart;
    static std::string _modelfilename;
	static bool _darkMode;
};

#endif // SYSTEMPREFERENCES_H
