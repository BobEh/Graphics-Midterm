#include "cWorld.h"
#include <iostream>
#include <sstream>

sNVPair cWorld::RecieveMessage(sNVPair message)
{

	sNVPair response;
	response.name = "OK";

	if (message.name == "Position X Down")
	{
		this->RecieveMessage(message);
		response.name = "OK";
		return response;
	}
	if (message.name == "Position X Up")
	{
		this->RecieveMessage(message);
		response.name = "OK";
		return response;
	}
	if (message.name == "Position Y Down")
	{
		this->RecieveMessage(message);
		response.name = "OK";
		return response;
	}
	if (message.name == "Position Y Up")
	{
		this->RecieveMessage(message);
		response.name = "OK";
		return response;
	}
	if (message.name == "Position Z Down")
	{
		this->RecieveMessage(message);
		response.name = "OK";
		return response;
	}
	if (message.name == "Position Z Up")
	{
		this->RecieveMessage(message);
		response.name = "OK";
		return response;
	}
	else
	{
		response.name = "UNKNOWN COMMAND";
	}
	return response;
}