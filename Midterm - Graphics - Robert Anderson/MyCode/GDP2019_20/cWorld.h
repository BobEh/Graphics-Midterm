#pragma once

#include <vector>
#include "cLight.h"
#include "iMessageInterface.h"

class cWorld
{
public:

	// from iMessageInterface
	virtual sNVPair RecieveMessage(sNVPair message);

	std::vector<cLight*> m_vec_pLights;

private:
	float m_worldSize;

};