#pragma once


#include "iObject.h"
#include <string>
#include <vector>

class cObjectFactory
{
public:
	iObject* CreateObject(std::string objectType);
};