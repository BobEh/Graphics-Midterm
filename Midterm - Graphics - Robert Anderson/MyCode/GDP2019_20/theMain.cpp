#include "GLCommon.h"
#include <Windows.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <stdlib.h>		// c libs
#include <stdio.h>		// c libs

#include <iostream>		// C++ IO standard stuff
#include <map>			// Map aka "dictonary" 

#include "cModelLoader.h"			
#include "cVAOManager.h"		// NEW
#include "cGameObject.h"

#include "cShaderManager.h"

#include <sstream>
#include <fstream>

#include <limits.h>
#include <float.h>

// The Physics function
#include "PhysicsStuff.h"
#include "cPhysics.h"

#include "DebugRenderer/cDebugRenderer.h"
#include <pugixml/pugixml.hpp>
#include <pugixml/pugixml.cpp>
#include "cLight.h"
#include "cWorld.h"

// Used to visualize the attenuation of the lights...
#include "LightManager/cLightHelper.h"

using namespace pugi;

xml_document document;
std::string gameDataLocation = "gameData.xml";
xml_parse_result result = document.load_file(gameDataLocation.c_str());
std::ofstream file;
xml_node root_node = document.child("GameData");
xml_node lightData = root_node.child("LightData");
xml_node objectData = root_node.child("ObjectData");

std::fstream meshNameFile("meshNames.txt");


bool fileChanged = false;

void DrawObject(glm::mat4 m, cGameObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager);

//glm::vec3 borderLight3 = glm::vec3(0.0f, -149.0f, 0.0f);
//glm::vec3 borderLight4 = glm::vec3(0.0f, 200.0f, 0.0f);
//glm::vec3 borderLight5 = glm::vec3(0.0f, 0.0f, -199.0f);
//glm::vec3 borderLight6 = glm::vec3(0.0f, 0.0f, 199.0f);

cWorld* pWorld = new cWorld();

unsigned int currentObject = 0;

cLight* pMainLight = new cLight();
unsigned int currentLight = 0;
cLight* EyeLight1 = new cLight();
cLight* EyeLight2 = new cLight();
cLight* CaveLight = new cLight();
cLight* FlashLight = new cLight();

std::vector<cLight*> m_vec_pSpotLights;

float cameraLeftRight = 0.0f;

glm::vec3 cameraEyeMain = glm::vec3(-26.0f, 491.0f, -610.0f);

glm::vec3 cameraTarget = glm::vec3(pMainLight->getPositionX(), pMainLight->getPositionY(), pMainLight->getPositionZ());
glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

float SpotInnerAngle1 = 5.0f;
float cornerLightSpotOuterAngle1 = 7.5f;

//mainLight
// This is a "normalized" direction
// (i.e. the length is 1.0f)

bool bLightDebugSheresOn = false;


bool onGround = false;
bool onPlatform = false;

glm::mat4 calculateWorldMatrix(cGameObject* pCurrentObject);


// Load up my "scene"  (now global)
std::vector<cGameObject*> g_vec_pGameObjects;
std::vector<cGameObject*> g_vec_pWallObjects;
std::map<std::string /*FriendlyName*/, cGameObject*> g_map_GameObjectsByFriendlyName;
glm::vec3 currentObjectPosition = glm::vec3(-20.0f,0.0f,0.0f);


// returns NULL (0) if we didn't find it.
cGameObject* pFindObjectByFriendlyName( std::string name );
cGameObject* pFindObjectByFriendlyNameMap( std::string name );

//bool g_BallCollided = false;

bool isShiftKeyDownByAlone(int mods)
{
	if (mods == GLFW_MOD_SHIFT)			
	{
		// Shift key is down all by itself
		return true;
	}
	return false;
}

bool isCtrlKeyDownByAlone(int mods)
{
	if (mods == GLFW_MOD_CONTROL)			
	{
		return true;
	}
	return false;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	const float CAMERASPEED = 1.0f;
	const float MOVESPEED = 5.0f;

	if ( !isShiftKeyDownByAlone(mods) && !isCtrlKeyDownByAlone(mods) )
	{
		//Reset positions
		//if (key == GLFW_KEY_R)
		//{
		//	pMainLight->setPositionX(0.0f);
		//	pMainLight->setPositionY(10.0f);
		//	pMainLight->setPositionZ(0.0f);
		//	cameraEye = glm::vec3(0.0, 30.0, -100.0);
		//}

		// Move the camera (A & D for left and right, along the x axis)
		if (key == GLFW_KEY_A)
		{
			cameraEyeMain.x -= CAMERASPEED;		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_D)
		{
			cameraEyeMain.x += CAMERASPEED;		// Move the camera +0.01f units
		}

		// Move the camera (Q & E for up and down, along the y axis)
		if (key == GLFW_KEY_Q)
		{
			cameraEyeMain.y -= CAMERASPEED;		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_E)
		{
			cameraEyeMain.y += CAMERASPEED;		// Move the camera +0.01f units
		}

		// Move the camera (W & S for towards and away, along the z axis)
		if (key == GLFW_KEY_W)
		{
			cameraEyeMain.z -= CAMERASPEED;		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_S)
		{
			cameraEyeMain.z += CAMERASPEED;		// Move the camera +0.01f units
		}

		if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		{
			cameraEyeMain = glm::vec3(-26.0f, 491.0f, -610.0f);
			currentObjectPosition = glm::vec3(-20.0f, 0.0f, 0.0f);
		}
		if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		{
			cameraEyeMain = glm::vec3(53.0f, 174.0f, 69.0f);
			currentObjectPosition = glm::vec3(g_vec_pGameObjects.at(20)->positionXYZ);
		}
		if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		{
			cameraEyeMain = glm::vec3(175.0f, 6.0f, -149.0f);
			currentObjectPosition = glm::vec3(g_vec_pGameObjects.at(4)->positionXYZ);
		}

//		if ( key == GLFW_KEY_B )
//		{ 
//			// Shoot a bullet from the pirate ship
//			// Find the pirate ship...
//			// returns NULL (0) if we didn't find it.
////			cGameObject* pShip = pFindObjectByFriendlyName("PirateShip");
//			cGameObject* pShip = pFindObjectByFriendlyNameMap("PirateShip");
//			// Maybe check to see if it returned something... 
//
//			// Find the sphere#2
////			cGameObject* pBall = pFindObjectByFriendlyName("Sphere#2");
//			cGameObject* pBall = pFindObjectByFriendlyNameMap("Sphere#2");
//
//			// Set the location velocity for sphere#2
//			pBall->positionXYZ = pShip->positionXYZ;
//			pBall->inverseMass = 1.0f;		// So it's updated
//			// 20.0 units "to the right"
//			// 30.0 units "up"
//			pBall->velocity = glm::vec3( 15.0f, 20.0f, 0.0f );
//			pBall->accel = glm::vec3(0.0f,0.0f,0.0f);
//			pBall->diffuseColour = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
//		}//if ( key == GLFW_KEY_B )

	}

	if (isShiftKeyDownByAlone(mods))
	{
		// switch object to control
		if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
		{
			if (currentObject == g_vec_pGameObjects.size() - 1)
			{
				currentObject = 0;
			}
			else
			{
				currentObject++;
			}
		}
		if (key == GLFW_KEY_BACKSLASH && action == GLFW_PRESS)
		{
			if (currentObject == 0)
			{
				currentObject = g_vec_pGameObjects.size() - 1;
			}
			else
			{
				currentObject--;
			}
		}
		// move the ramp
		if (key == GLFW_KEY_A)
		{
			g_vec_pGameObjects.at(currentObject)->positionXYZ.x += CAMERASPEED;
		}
		if (key == GLFW_KEY_D)
		{
			g_vec_pGameObjects.at(currentObject)->positionXYZ.x -= CAMERASPEED;
		}

		// Move the camera (Q & E for up and down, along the y axis)
		if (key == GLFW_KEY_Q)
		{
			g_vec_pGameObjects.at(currentObject)->positionXYZ.y += CAMERASPEED;
		}
		if (key == GLFW_KEY_E)
		{
			g_vec_pGameObjects.at(currentObject)->positionXYZ.y -= CAMERASPEED;
		}

		// Move the camera (W & S for towards and away, along the z axis)
		if (key == GLFW_KEY_W)
		{
			g_vec_pGameObjects.at(currentObject)->positionXYZ.z += CAMERASPEED;
		}
		if (key == GLFW_KEY_S)
		{
			g_vec_pGameObjects.at(currentObject)->positionXYZ.z -= CAMERASPEED;
		}

		//Rotate the ramp
		if (key == GLFW_KEY_F)
		{
			g_vec_pGameObjects.at(currentObject)->rotationXYZ.x += CAMERASPEED / 10.0f;
		}
		if (key == GLFW_KEY_H)
		{
			g_vec_pGameObjects.at(currentObject)->rotationXYZ.x -= CAMERASPEED / 10.0f;
		}

		// Move the camera (Q & E for up and down, along the y axis)
		if (key == GLFW_KEY_R)
		{
			g_vec_pGameObjects.at(currentObject)->rotationXYZ.y += CAMERASPEED / 10.0f;
		}
		if (key == GLFW_KEY_Y)
		{
			g_vec_pGameObjects.at(currentObject)->rotationXYZ.y -= CAMERASPEED / 10.0f;
		}

		// Move the camera (W & S for towards and away, along the z axis)
		if (key == GLFW_KEY_T)
		{
			g_vec_pGameObjects.at(currentObject)->rotationXYZ.z += CAMERASPEED / 10.0f;
		}
		if (key == GLFW_KEY_G)
		{
			g_vec_pGameObjects.at(currentObject)->rotationXYZ.z -= CAMERASPEED / 10.0f;
		}
		if (key == GLFW_KEY_P && action == GLFW_PRESS)
		{
			cGameObject* pObject = new cGameObject();			// HEAP
			pObject->meshName = g_vec_pGameObjects.at(currentObject)->meshName;
			pObject->friendlyName = g_vec_pGameObjects.at(currentObject)->meshName;
			pObject->positionXYZ = g_vec_pGameObjects.at(currentObject)->positionXYZ * glm::vec3(2.0f,0.0f,0.0f);
			pObject->rotationXYZ = g_vec_pGameObjects.at(currentObject)->rotationXYZ;
			pObject->scale = 1.0f;
			pObject->objectColourRGBA = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
			pObject->physicsShapeType = MESH;
			pObject->inverseMass = 0.0f;

			g_vec_pGameObjects.push_back(pObject);

			xml_node addition = objectData.append_child("Object");
			xml_node positionX = addition.append_child("PositionX");
			positionX.append_child(node_pcdata).set_value(std::to_string(pObject->positionXYZ.x).c_str());
			xml_node positionY = addition.append_child("PositionY");
			positionY.append_child(node_pcdata).set_value(std::to_string(pObject->positionXYZ.y).c_str());
			xml_node positionZ = addition.append_child("PositionZ");
			positionZ.append_child(node_pcdata).set_value(std::to_string(pObject->positionXYZ.z).c_str());
			xml_node rotationX = addition.append_child("RotationX");
			rotationX.append_child(node_pcdata).set_value(std::to_string(pObject->rotationXYZ.x).c_str());
			xml_node rotationY = addition.append_child("RotationY");
			rotationY.append_child(node_pcdata).set_value(std::to_string(pObject->rotationXYZ.y).c_str());
			xml_node rotationZ = addition.append_child("RotationZ");
			rotationZ.append_child(node_pcdata).set_value(std::to_string(pObject->rotationXYZ.z).c_str());
		}

		if (key == GLFW_KEY_K)
		{
			int index = 0;
			for (xml_node object = objectData.first_child(); object; object = object.next_sibling())
			{
				xml_node changeData = object.first_child();
				changeData.last_child().set_value(std::to_string(g_vec_pGameObjects.at(index)->positionXYZ.x).c_str());
				changeData = changeData.next_sibling();
				changeData.last_child().set_value(std::to_string(g_vec_pGameObjects.at(index)->positionXYZ.y).c_str());
				changeData = changeData.next_sibling();
				changeData.last_child().set_value(std::to_string(g_vec_pGameObjects.at(index)->positionXYZ.z).c_str());
				changeData = changeData.next_sibling();
				changeData.last_child().set_value(std::to_string(g_vec_pGameObjects.at(index)->rotationXYZ.x).c_str());
				changeData = changeData.next_sibling();
				changeData.last_child().set_value(std::to_string(g_vec_pGameObjects.at(index)->rotationXYZ.y).c_str());
				changeData = changeData.next_sibling();
				changeData.last_child().set_value(std::to_string(g_vec_pGameObjects.at(index)->rotationXYZ.z).c_str());
				index++;
			}
			fileChanged = true;
		}

		//if (key == GLFW_KEY_V)
		//{
		//	if (pCurrentLight == "mainLight")
		//		mainLightPosition -= 0.1f;
		//}
		//if (key == GLFW_KEY_B)
		//{
		//	if (pCurrentLight == "mainLight")
		//		mainLightPosition += 0.1f;
		//}
		//if (key == GLFW_KEY_N)
		//{
		//	if (pCurrentLight == "mainLight")
		//		mainLightPosition -= 0.1f;
		//}
		//if (key == GLFW_KEY_M)
		//{
		//	if (pCurrentLight == "mainLight")
		//		mainLightPosition += 0.1f;
		//}


		//if (key == GLFW_KEY_9)
		//{
		//	bLightDebugSheresOn = false;
		//}
		//if (key == GLFW_KEY_0)
		//{
		//	bLightDebugSheresOn = true;
		//}
		//// switch lights to control
		//if (key == GLFW_KEY_M)
		//{
		//	currentLight = 0;		// Move the camera -0.01f units
		//}
		//if (key == GLFW_KEY_1)
		//{
		//	currentLight = 1;
		//}
		//if (key == GLFW_KEY_2)
		//{
		//	currentLight = 2;
		//}
		//if (key == GLFW_KEY_3)
		//{
		//	currentLight = 3;
		//}
		//// move the light
		//if (key == GLFW_KEY_A)
		//{
		//	sNVPair message;
		//	message.name = "Position X Down";
		//	message.fValue = CAMERASPEED;
		//	pWorld->m_vec_pLights.at(currentLight)->RecieveMessage(message);		// Move the camera -0.01f units
		//}
		//if (key == GLFW_KEY_D)
		//{
		//	sNVPair message;
		//	message.name = "Position X Up";
		//	message.fValue = CAMERASPEED;
		//	pWorld->m_vec_pLights.at(currentLight)->RecieveMessage(message);
		//}

		//// Move the camera (Q & E for up and down, along the y axis)
		//if (key == GLFW_KEY_Q)
		//{
		//	sNVPair message;
		//	message.name = "Position Y Down";
		//	message.fValue = CAMERASPEED;
		//	pWorld->m_vec_pLights.at(currentLight)->RecieveMessage(message);
		//}
		//if (key == GLFW_KEY_E)
		//{
		//	sNVPair message;
		//	message.name = "Position Y Up";
		//	message.fValue = CAMERASPEED;
		//	pWorld->m_vec_pLights.at(currentLight)->RecieveMessage(message);
		//}

		//// Move the camera (W & S for towards and away, along the z axis)
		//if (key == GLFW_KEY_W)
		//{
		//	sNVPair message;
		//	message.name = "Position Z Down";
		//	message.fValue = CAMERASPEED;
		//	pWorld->m_vec_pLights.at(currentLight)->RecieveMessage(message);
		//}
		//if (key == GLFW_KEY_S)
		//{
		//	sNVPair message;
		//	message.name = "Position Z Up";
		//	message.fValue = CAMERASPEED;
		//	pWorld->m_vec_pLights.at(currentLight)->RecieveMessage(message);
		//}
		//if (key == GLFW_KEY_I && action == GLFW_PRESS)
		//{
		//	for (int i = 0; i < pWorld->m_vec_pLights.size(); i++)
		//	{
		//		pWorld->m_vec_pLights.at(i)->setLinearAtten(1.0f);
		//	}
		//}
		////if (key == GLFW_KEY_N && action == GLFW_PRESS)
		////{
		////	for (int i = 0; i < pWorld->m_vec_pLights.size(); i++)
		////	{
		////		pWorld->m_vec_pLights.at(i)->setLinearAtten(0.003517f);
		////	}
		////}

		//if (key == GLFW_KEY_K)
		//{
		//	for (int i = 0; i < pWorld->m_vec_pLights.size(); i++)
		//	{
		//		std::string currentNodeName = pWorld->m_vec_pLights.at(i)->getNodeName();
		//		xml_node LightToChange = lightData.child(currentNodeName.c_str());
		//		std::vector<std::string> changeData = pWorld->m_vec_pLights.at(i)->getAllDataStrings();
		//		//int index = 0;
		//		//changeData.push_back(std::to_string(pLightsVec.at(i)->getPositionX()));
		//		//changeData.push_back(std::to_string(pLightsVec.at(i)->getPositionY()));
		//		//changeData.push_back(std::to_string(pLightsVec.at(i)->getPositionZ()));
		//		//changeData.push_back(std::to_string(pLightsVec.at(i)->getConstAtten()));
		//		//changeData.push_back(std::to_string(pLightsVec.at(i)->getLinearAtten()));
		//		//changeData.push_back(std::to_string(pLightsVec.at(i)->getQuadraticAtten()));
		//		//changeData.push_back(std::to_string(pLightsVec.at(i)->getInnerSpot()));
		//		//changeData.push_back(std::to_string(pLightsVec.at(i)->getOuterSpot()));
		//		//Set data to xml to set positions

		//		int index = 0;
		//		for (xml_node dataNode = LightToChange.child("PositionX"); dataNode; dataNode = dataNode.next_sibling())
		//		{
		//			//LightToChange->first_node("PositionX")->value(changeData.at(i).c_str());
		//			//LightToChange->first_node("PositionY")->value(changeData.at(i).c_str());
		//			//LightToChange->first_node("PositionZ")->value(changeData.at(i).c_str());
		//			//LightToChange->first_node("ConstAtten")->value(changeData.at(i).c_str());
		//			//LightToChange->first_node("LinearAtten")->value(changeData.at(i).c_str());
		//			//LightToChange->first_node("QuadraticAtten")->value(changeData.at(i).c_str());
		//			//LightToChange->first_node("SpotInnerAngle")->value(changeData.at(i).c_str());
		//			//LightToChange->first_node("SpotOuterAngle")->value(changeData.at(i).c_str());

		//			//std::string changeString = changeData.at(index);
		//			//std::cout << changeString << std::endl;
		//			dataNode.last_child().set_value(changeData.at(index).c_str());
		//			//std::cout << dataNode->value() << std::endl;
		//			//dataNode = dataNode->next_sibling();
		//			index++;
		//		}
		//		//for (xml_node<>* dataNode = LightToChange->first_node(); dataNode; dataNode = dataNode->next_sibling())
		//		//{
		//		//	//assert(index < changeData.size());
		//		//	const char * stringToChange = changeData.at(index).c_str();
		//		//	dataNode->value(stringToChange);
		//		//	file.open(gameDataLocation);
		//		//	file << "<?xml version='1.0' encoding='utf-8'?>\n";
		//		//	file << document;
		//		//	file.close();
		//		//	index++;
		//		//}
		//	}
		//	fileChanged = true;
		//}

		if (key == GLFW_KEY_9)
		{
			bLightDebugSheresOn = false;			
		}
		if (key == GLFW_KEY_0)
		{
			bLightDebugSheresOn = true; 
		}

	}//if (isShiftKeyDownByAlone(mods))

	if (isCtrlKeyDownByAlone(mods))
	{
		// move the shpere
		cGameObject* pSphere = pFindObjectByFriendlyName("Sphere#1");
		if (key == GLFW_KEY_D)
		{
			//pSphere->rotationXYZ -= glm::vec3(CAMERASPEED, 0.0f, 0.0f);
			pSphere->velocity -= glm::vec3(MOVESPEED, 0.0f, 0.0f);		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_A)
		{
			//pSphere->rotationXYZ += glm::vec3(CAMERASPEED, 0.0f, 0.0f);
			pSphere->velocity += glm::vec3(MOVESPEED, 0.0f, 0.0f);		// Move the camera +0.01f units
		}

		if (key == GLFW_KEY_M && action == GLFW_PRESS)
		{
			if (pSphere->isWireframe)
			{
				pSphere->isWireframe = false;
			}
			else
			{
				pSphere->isWireframe = true;
			}
		}

		if (key == GLFW_KEY_1)
		{
			pWorld->m_vec_pLights.at(currentLight)->setConstAtten(pWorld->m_vec_pLights.at(currentLight)->getConstAtten() * 0.99f);			// 99% of what it was
		}
		if (key == GLFW_KEY_2)
		{
			pWorld->m_vec_pLights.at(currentLight)->setConstAtten(pWorld->m_vec_pLights.at(currentLight)->getConstAtten() * 1.01f);
		}
		if (key == GLFW_KEY_3)
		{
			pWorld->m_vec_pLights.at(currentLight)->setLinearAtten(pWorld->m_vec_pLights.at(currentLight)->getLinearAtten() * 0.99f);			// 99% of what it was
		}
		if (key == GLFW_KEY_4)
		{
			pWorld->m_vec_pLights.at(currentLight)->setLinearAtten(pWorld->m_vec_pLights.at(currentLight)->getLinearAtten() * 1.01f);			// 1% more of what it was
		}
		if (key == GLFW_KEY_5)
		{
			pWorld->m_vec_pLights.at(currentLight)->setQuadraticAtten(pWorld->m_vec_pLights.at(currentLight)->getQuadraticAtten() * 0.99f);
		}
		if (key == GLFW_KEY_6)
		{
			pWorld->m_vec_pLights.at(currentLight)->setQuadraticAtten(pWorld->m_vec_pLights.at(currentLight)->getQuadraticAtten() * 1.01f);
		}
		if (key == GLFW_KEY_N)
		{
			pWorld->m_vec_pLights.at(0)->setConstAtten(2.5f);
			pWorld->m_vec_pLights.at(1)->setConstAtten(0.000072f);
			pWorld->m_vec_pLights.at(2)->setConstAtten(0.000072f);
			pWorld->m_vec_pLights.at(3)->setConstAtten(0.000072f);
			m_vec_pSpotLights.at(0)->setConstAtten(0.000002f);
		}
		if (key == GLFW_KEY_B)
		{
			pWorld->m_vec_pLights.at(0)->setConstAtten(1.05f);
			pWorld->m_vec_pLights.at(1)->setConstAtten(3.0f);
			pWorld->m_vec_pLights.at(2)->setConstAtten(3.0f);
			pWorld->m_vec_pLights.at(3)->setConstAtten(3.0f);
			m_vec_pSpotLights.at(0)->setConstAtten(3.0f);
		}
	}

	if (isCtrlKeyDownByAlone(mods) && isShiftKeyDownByAlone(mods))
	{
		
	}


	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

template <class T>
T randInRange(T min, T max)
{
	double value =
		min + static_cast <double> (rand())
		/ (static_cast <double> (RAND_MAX / (static_cast<double>(max - min))));
	return static_cast<T>(value);
};


int main(void)
{

	cModelLoader* pTheModelLoader = new cModelLoader();	// Heap

	std::string objectPath;
	std::ifstream objectLocation("objectPaths.txt");
	std::vector<cMesh> meshVec;
	if (objectLocation.is_open())
	{
		while (getline(objectLocation, objectPath))
		{
			cMesh theMesh;
			pTheModelLoader->LoadPlyModel(objectPath, theMesh);
			meshVec.push_back(theMesh);
		}
		objectLocation.close();
	}

	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);



	cDebugRenderer* pDebugRenderer = new cDebugRenderer();
	pDebugRenderer->initialize();

	cShaderManager* pTheShaderManager = new cShaderManager();

	cShaderManager::cShader vertexShad;
	vertexShad.fileName = "assets/shaders/vertexShader01.glsl";

	cShaderManager::cShader fragShader;
	fragShader.fileName = "assets/shaders/fragmentShader01.glsl";

	if (!pTheShaderManager->createProgramFromFile("SimpleShader", vertexShad, fragShader))
	{
		std::cout << "Error: didn't compile the shader" << std::endl;
		std::cout << pTheShaderManager->getLastError();
		return -1;
	}

	GLuint shaderProgID = pTheShaderManager->getIDFromFriendlyName("SimpleShader");


	// Create a VAO Manager...
	// #include "cVAOManager.h"  (at the top of your file)
	cVAOManager* pTheVAOManager = new cVAOManager();

	// Note, the "filename" here is really the "model name" 
	//  that we can look up later (i.e. it doesn't have to be the file name)
	std::string meshName;
	std::vector<std::string> nameVec;
	if (meshNameFile.is_open())
	{
		while (getline(meshNameFile, meshName))
		{
			nameVec.push_back(meshName);
		}
	}
	for (int i = 0; i < nameVec.size(); ++i)
	{
		sModelDrawInfo drawInfo;
		pTheVAOManager->LoadModelIntoVAO(nameVec.at(i), meshVec.at(i), drawInfo, shaderProgID);
	}


	// At this point, the model is loaded into the GPU

	cGameObject* pSphere = new cGameObject();
	pSphere->meshName = "sphere";
	pSphere->friendlyName = "Sphere#1";	// We use to search 
	pSphere->positionXYZ = glm::vec3(-25.0f, 10.0f, 1.0f);
	pSphere->rotationXYZ = glm::vec3(0.0f,0.0f,0.0f);
	pSphere->scale = 1.0f;
	pSphere->objectColourRGBA = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	// Set the sphere's initial velocity, etc.
	pSphere->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	pSphere->accel = glm::vec3(0.0f,0.0f,0.0f);
	pSphere->physicsShapeType = SPHERE;
	pSphere->SPHERE_radius = 1.0f;
	pSphere->inverseMass = 1.0f;
	pSphere->isWireframe = false;
	pSphere->isVisible = false;
//	pSphere->inverseMass = 0.0f;			// Sphere won't move

		// Sphere and cube
	cGameObject* pSphere2 = new cGameObject();
	pSphere2->meshName = "sphere";
	pSphere2->friendlyName = "Sphere#2";
//	pSphere2->positionXYZ = glm::vec3(25.0f, 20.0f, 1.0f);
	pSphere2->rotationXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	pSphere2->scale = 1.0f;
	pSphere2->objectColourRGBA = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	// Set the sphere's initial velocity, etc.
//	pSphere2->velocity = glm::vec3(6.0f, -15.0f, 0.0f);
	pSphere2->accel = glm::vec3(0.0f, 0.0f, 0.0f);
	pSphere2->physicsShapeType = SPHERE;
	pSphere2->SPHERE_radius = 1.0f;
	pSphere2->inverseMass = 0.0f;
	pSphere2->isVisible = false;
	//	pSphere->inverseMass = 0.0f;			// Sphere won't move

	cGameObject* pIsland = new cGameObject();			// HEAP
	pIsland->meshName = "island";
	pIsland->friendlyName = "island";
	pIsland->positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	pIsland->rotationXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	pIsland->scale = 1.0f;
	pIsland->objectColourRGBA = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	pIsland->physicsShapeType = MESH;
	pIsland->inverseMass = 0.0f;

	int nameCount = 8;
	for (xml_node object = objectData.first_child(); object; object = object.next_sibling(), nameCount++)
	{
		cGameObject* pObject = new cGameObject();			// HEAP
		pObject->meshName = nameVec.at(nameCount);
		pObject->friendlyName = nameVec.at(nameCount);
		std::string positionXString = object.child("PositionX").child_value();
		std::string positionYString = object.child("PositionY").child_value();
		std::string positionZString = object.child("PositionZ").child_value();
		pObject->positionXYZ = glm::vec3(std::stof(positionXString), std::stof(positionYString), std::stof(positionZString));
		std::string rotationXString = object.child("RotationX").child_value();
		std::string rotationYString = object.child("RotationY").child_value();
		std::string rotationZString = object.child("RotationZ").child_value();
		pObject->rotationXYZ = glm::vec3(std::stof(rotationXString), std::stof(rotationYString), std::stof(rotationZString));
		pObject->scale = 1.0f;
		pObject->objectColourRGBA = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
		pObject->physicsShapeType = MESH;
		pObject->inverseMass = 0.0f;	// Ignored during update

		::g_vec_pGameObjects.push_back(pObject);
	}

	::g_vec_pGameObjects.push_back(pSphere);
	::g_vec_pWallObjects.push_back(pIsland);


	// Will be moved placed around the scene
	cGameObject* pDebugSphere = new cGameObject();
	pDebugSphere->meshName = "sphere";
	pDebugSphere->friendlyName = "debug_sphere";
	pDebugSphere->positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	pDebugSphere->rotationXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	pDebugSphere->scale = 0.1f;
//	pDebugSphere->objectColourRGBA = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	pDebugSphere->debugColour = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	pDebugSphere->isWireframe = true;
	pDebugSphere->inverseMass = 0.0f;			// Sphere won't move
	
	glEnable(GL_DEPTH);			// Write to the depth buffer
	glEnable(GL_DEPTH_TEST);	// Test with buffer when drawing
	
	cPhysics* pPhsyics = new cPhysics();

	cLightHelper* pLightHelper = new cLightHelper();

	//Get data from xml to set positions of main light
	pMainLight->setNodeName("MainLight");
	xml_node mainLightNode = lightData.child("MainLight");
	pMainLight->setPositionX(std::stof(mainLightNode.child("PositionX").child_value()));
	pMainLight->setPositionY(std::stof(mainLightNode.child("PositionY").child_value()));
	pMainLight->setPositionZ(std::stof(mainLightNode.child("PositionZ").child_value()));
	pMainLight->setPositionXYZ(glm::vec3(std::stof(mainLightNode.child("PositionX").child_value()), std::stof(mainLightNode.child("PositionY").child_value()), std::stof(mainLightNode.child("PositionZ").child_value())));
	pMainLight->_ColourR = std::stof(mainLightNode.child("ColourR").child_value());
	pMainLight->_ColourG = std::stof(mainLightNode.child("ColourG").child_value());
	pMainLight->_ColourB = std::stof(mainLightNode.child("ColourB").child_value());
	pMainLight->setConstAtten(std::stof(mainLightNode.child("ConstAtten").child_value()));
	pMainLight->setLinearAtten(std::stof(mainLightNode.child("LinearAtten").child_value()));
	pMainLight->setQuadraticAtten(std::stof(mainLightNode.child("QuadraticAtten").child_value()));
	pMainLight->setInnerSpot(std::stof(mainLightNode.child("SpotInnerAngle").child_value()));
	pMainLight->setOuterSpot(std::stof(mainLightNode.child("SpotOuterAngle").child_value()));

	pWorld->m_vec_pLights.push_back(pMainLight);

	EyeLight1->setNodeName("EyeLight1");
	xml_node EyeLight1Node = lightData.child("EyeLight1");
	EyeLight1->setPositionX(std::stof(EyeLight1Node.child("PositionX").child_value()));
	EyeLight1->setPositionY(std::stof(EyeLight1Node.child("PositionY").child_value()));
	EyeLight1->setPositionZ(std::stof(EyeLight1Node.child("PositionZ").child_value()));
	EyeLight1->setPositionXYZ(glm::vec3(std::stof(EyeLight1Node.child("PositionX").child_value()), std::stof(EyeLight1Node.child("PositionY").child_value()), std::stof(EyeLight1Node.child("PositionZ").child_value())));
	EyeLight1->_ColourR = std::stof(EyeLight1Node.child("ColourR").child_value());
	EyeLight1->_ColourG = std::stof(EyeLight1Node.child("ColourG").child_value());
	EyeLight1->_ColourB = std::stof(EyeLight1Node.child("ColourB").child_value());
	EyeLight1->setConstAtten(std::stof(EyeLight1Node.child("ConstAtten").child_value()));
	EyeLight1->setLinearAtten(std::stof(EyeLight1Node.child("LinearAtten").child_value()));
	EyeLight1->setQuadraticAtten(std::stof(EyeLight1Node.child("QuadraticAtten").child_value()));
	EyeLight1->setInnerSpot(std::stof(EyeLight1Node.child("SpotInnerAngle").child_value()));
	EyeLight1->setOuterSpot(std::stof(EyeLight1Node.child("SpotOuterAngle").child_value()));

	pWorld->m_vec_pLights.push_back(EyeLight1);

	EyeLight2->setNodeName("EyeLight2");
	xml_node EyeLight2Node = lightData.child("EyeLight2");
	EyeLight2->setPositionX(std::stof(EyeLight2Node.child("PositionX").child_value()));
	EyeLight2->setPositionY(std::stof(EyeLight2Node.child("PositionY").child_value()));
	EyeLight2->setPositionZ(std::stof(EyeLight2Node.child("PositionZ").child_value()));
	EyeLight2->setPositionXYZ(glm::vec3(std::stof(EyeLight2Node.child("PositionX").child_value()), std::stof(EyeLight2Node.child("PositionY").child_value()), std::stof(EyeLight2Node.child("PositionZ").child_value())));
	EyeLight2->_ColourR = std::stof(EyeLight2Node.child("ColourR").child_value());
	EyeLight2->_ColourG = std::stof(EyeLight2Node.child("ColourG").child_value());
	EyeLight2->_ColourB = std::stof(EyeLight2Node.child("ColourB").child_value());
	EyeLight2->setConstAtten(std::stof(EyeLight2Node.child("ConstAtten").child_value()));
	EyeLight2->setLinearAtten(std::stof(EyeLight2Node.child("LinearAtten").child_value()));
	EyeLight2->setQuadraticAtten(std::stof(EyeLight2Node.child("QuadraticAtten").child_value()));
	EyeLight2->setInnerSpot(std::stof(EyeLight2Node.child("SpotInnerAngle").child_value()));
	EyeLight2->setOuterSpot(std::stof(EyeLight2Node.child("SpotOuterAngle").child_value()));

	pWorld->m_vec_pLights.push_back(EyeLight2);

	CaveLight->setNodeName("CaveLight");
	xml_node CaveLightNode = lightData.child("CaveLight");
	CaveLight->setPositionX(std::stof(CaveLightNode.child("PositionX").child_value()));
	CaveLight->setPositionY(std::stof(CaveLightNode.child("PositionY").child_value()));
	CaveLight->setPositionZ(std::stof(CaveLightNode.child("PositionZ").child_value()));
	CaveLight->setPositionXYZ(glm::vec3(std::stof(CaveLightNode.child("PositionX").child_value()), std::stof(CaveLightNode.child("PositionY").child_value()), std::stof(CaveLightNode.child("PositionZ").child_value())));
	CaveLight->_ColourR = std::stof(CaveLightNode.child("ColourR").child_value());
	CaveLight->_ColourG = std::stof(CaveLightNode.child("ColourG").child_value());
	CaveLight->_ColourB = std::stof(CaveLightNode.child("ColourB").child_value());
	CaveLight->setConstAtten(std::stof(CaveLightNode.child("ConstAtten").child_value()));
	CaveLight->setLinearAtten(std::stof(CaveLightNode.child("LinearAtten").child_value()));
	CaveLight->setQuadraticAtten(std::stof(CaveLightNode.child("QuadraticAtten").child_value()));
	CaveLight->setInnerSpot(std::stof(CaveLightNode.child("SpotInnerAngle").child_value()));
	CaveLight->setOuterSpot(std::stof(CaveLightNode.child("SpotOuterAngle").child_value()));

	pWorld->m_vec_pLights.push_back(CaveLight);

	FlashLight->setNodeName("FlashLight");
	xml_node FlashLightNode = lightData.child("FlashLight");
	FlashLight->setPositionX(std::stof(FlashLightNode.child("PositionX").child_value()));
	FlashLight->setPositionY(std::stof(FlashLightNode.child("PositionY").child_value()));
	FlashLight->setPositionZ(std::stof(FlashLightNode.child("PositionZ").child_value()));
	FlashLight->setPositionXYZ(glm::vec3(std::stof(FlashLightNode.child("PositionX").child_value()), std::stof(FlashLightNode.child("PositionY").child_value()), std::stof(FlashLightNode.child("PositionZ").child_value())));
	FlashLight->_ColourR = std::stof(FlashLightNode.child("ColourR").child_value());
	FlashLight->_ColourG = std::stof(FlashLightNode.child("ColourG").child_value());
	FlashLight->_ColourB = std::stof(FlashLightNode.child("ColourB").child_value());
	FlashLight->setConstAtten(std::stof(FlashLightNode.child("ConstAtten").child_value()));
	FlashLight->setLinearAtten(std::stof(FlashLightNode.child("LinearAtten").child_value()));
	FlashLight->setQuadraticAtten(std::stof(FlashLightNode.child("QuadraticAtten").child_value()));
	FlashLight->setInnerSpot(std::stof(FlashLightNode.child("SpotInnerAngle").child_value()));
	FlashLight->setOuterSpot(std::stof(FlashLightNode.child("SpotOuterAngle").child_value()));

	m_vec_pSpotLights.push_back(FlashLight);

	int setCount = 0;
	float properYPosition = 0.0f;
	float properPlatformYPosition = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		if (fileChanged)
		{
			
		}
		float ratio;
		int width, height;
		glm::mat4 p, v;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		// Projection matrix
		p = glm::perspective(0.6f,		// FOV
							 ratio,			// Aspect ratio
							 0.1f,			// Near clipping plane
							 1000.0f);		// Far clipping plane

		// View matrix
		v = glm::mat4(1.0f);

		glm::vec3 mainLightPosition = glm::vec3(pMainLight->getPositionX(), pMainLight->getPositionY(), pMainLight->getPositionZ());
		glm::vec3 EyeLight1Position = glm::vec3(EyeLight1->getPositionX(), EyeLight1->getPositionY(), EyeLight1->getPositionZ());
		glm::vec3 EyeLight2Position = glm::vec3(EyeLight2->getPositionX(), EyeLight2->getPositionY(), EyeLight2->getPositionZ());
		glm::vec3 CaveLightPosition = glm::vec3(CaveLight->getPositionX(), CaveLight->getPositionY(), CaveLight->getPositionZ());
		glm::vec3 FlashLightPosition = glm::vec3(FlashLight->getPositionX(), FlashLight->getPositionY(), FlashLight->getPositionZ());

		//Look at lights

		//if (pWorld->m_vec_pLights.at(currentLight)->_nodeName == "MainLight")
		//{
		//	v = glm::lookAt(cameraEyeMain,
		//		mainLightPosition,
		//		upVector);
		//}
		//if (pWorld->m_vec_pLights.at(currentLight)->_nodeName == "EyeLight1")
		//{
		//	v = glm::lookAt(cameraEyeMain,
		//		EyeLight1Position,
		//		upVector);
		//}
		//if (pWorld->m_vec_pLights.at(currentLight)->_nodeName == "EyeLight2")
		//{
		//	v = glm::lookAt(cameraEyeMain,
		//		EyeLight2Position,
		//		upVector);
		//}
		//if (pWorld->m_vec_pLights.at(currentLight)->_nodeName == "CaveLight")
		//{
		//	v = glm::lookAt(cameraEyeMain,
		//		CaveLightPosition,
		//		upVector);
		//}
		//if (pWorld->m_vec_pLights.at(currentLight)->_nodeName == "FlashLight")
		//{
		//	v = glm::lookAt(cameraEyeMain,
		//		CaveLightPosition,
		//		upVector);
		//}
		
		//Look at current object
		v = glm::lookAt(cameraEyeMain,	currentObjectPosition,	upVector);

		//Look at sphere

		//glm::vec3 cameraEye = glm::vec3(g_vec_pGameObjects.at(0)->positionXYZ.x + cameraLeftRight, g_vec_pGameObjects.at(0)->positionXYZ.y + 20.0f, g_vec_pGameObjects.at(0)->positionXYZ.z - 50.0f);
		//v = glm::lookAt(cameraEyeMain,
		//	pSphere->positionXYZ,
		//	upVector);

		glViewport(0, 0, width, height);

		// Clear both the colour buffer (what we see) and the 
		//  depth (or z) buffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		int lightIndex = 0;
		for (lightIndex; lightIndex < pWorld->m_vec_pLights.size(); ++lightIndex)
		{
			std::string positionString = "theLights[" + std::to_string(lightIndex) + "].position";
			std::string diffuseString = "theLights[" + std::to_string(lightIndex) + "].diffuse";
			std::string specularString = "theLights[" + std::to_string(lightIndex) + "].specular";
			std::string attenString = "theLights[" + std::to_string(lightIndex) + "].atten";
			std::string directionString = "theLights[" + std::to_string(lightIndex) + "].direction";
			std::string param1String = "theLights[" + std::to_string(lightIndex) + "].param1";
			std::string param2String = "theLights[" + std::to_string(lightIndex) + "].param2";

			GLint position = glGetUniformLocation(shaderProgID, positionString.c_str());
			GLint diffuse = glGetUniformLocation(shaderProgID, diffuseString.c_str());
			GLint specular = glGetUniformLocation(shaderProgID, specularString.c_str());
			GLint atten = glGetUniformLocation(shaderProgID, attenString.c_str());
			GLint direction = glGetUniformLocation(shaderProgID, directionString.c_str());
			GLint param1 = glGetUniformLocation(shaderProgID, param1String.c_str());
			GLint param2 = glGetUniformLocation(shaderProgID, param2String.c_str());

			glUniform4f(position, pWorld->m_vec_pLights.at(lightIndex)->getPositionX(), pWorld->m_vec_pLights.at(lightIndex)->getPositionY(), pWorld->m_vec_pLights.at(lightIndex)->getPositionZ(), 1.0f);
			glUniform4f(diffuse, pWorld->m_vec_pLights.at(lightIndex)->_ColourR, pWorld->m_vec_pLights.at(lightIndex)->_ColourG, pWorld->m_vec_pLights.at(lightIndex)->_ColourB, 1.0f);	// White
			glUniform4f(specular, 1.0f, 1.0f, 1.0f, 1.0f);	// White
			glUniform4f(atten, pWorld->m_vec_pLights.at(lightIndex)->getConstAtten(),  /* constant attenuation */	pWorld->m_vec_pLights.at(lightIndex)->getLinearAtten(),  /* Linear */ pWorld->m_vec_pLights.at(lightIndex)->getQuadraticAtten(),	/* Quadratic */  1000000.0f);	// Distance cut off

			glUniform4f(param1, 0.0f /*POINT light*/, 0.0f, 0.0f, 1.0f);
			glUniform4f(param2, 1.0f /*Light is on*/, 0.0f, 0.0f, 1.0f);
		}
		for (int i = 0; i < m_vec_pSpotLights.size(); ++i)
		{
			std::string positionString = "theLights[" + std::to_string(lightIndex) + "].position";
			std::string diffuseString = "theLights[" + std::to_string(lightIndex) + "].diffuse";
			std::string specularString = "theLights[" + std::to_string(lightIndex) + "].specular";
			std::string attenString = "theLights[" + std::to_string(lightIndex) + "].atten";
			std::string directionString = "theLights[" + std::to_string(lightIndex) + "].direction";
			std::string param1String = "theLights[" + std::to_string(lightIndex) + "].param1";
			std::string param2String = "theLights[" + std::to_string(lightIndex) + "].param2";

			GLint position = glGetUniformLocation(shaderProgID, positionString.c_str());
			GLint diffuse = glGetUniformLocation(shaderProgID, diffuseString.c_str());
			GLint specular = glGetUniformLocation(shaderProgID, specularString.c_str());
			GLint atten = glGetUniformLocation(shaderProgID, attenString.c_str());
			GLint direction = glGetUniformLocation(shaderProgID, directionString.c_str());
			GLint param1 = glGetUniformLocation(shaderProgID, param1String.c_str());
			GLint param2 = glGetUniformLocation(shaderProgID, param2String.c_str());

			glm::vec3 rockToCave = g_vec_pGameObjects.at(4)->positionXYZ - g_vec_pGameObjects.at(9)->positionXYZ;
			glm::vec3 spotLightDirection = glm::normalize(rockToCave);

			glUniform4f(position, m_vec_pSpotLights.at(i)->getPositionX(), m_vec_pSpotLights.at(i)->getPositionY(), m_vec_pSpotLights.at(i)->getPositionZ(), 1.0f);
			glUniform4f(diffuse, m_vec_pSpotLights.at(i)->_ColourR, m_vec_pSpotLights.at(i)->_ColourG, m_vec_pSpotLights.at(i)->_ColourB, 1.0f);	// White
			glUniform4f(specular, 1.0f, 1.0f, 1.0f, 1.0f);	// White
			glUniform4f(atten, m_vec_pSpotLights.at(i)->getConstAtten(),  /* constant attenuation */	m_vec_pSpotLights.at(i)->getLinearAtten(),  /* Linear */ m_vec_pSpotLights.at(i)->getQuadraticAtten(),	/* Quadratic */  1000000.0f);	// Distance cut off
			glUniform4f(direction, spotLightDirection.x, spotLightDirection.y, spotLightDirection.z, 1.0f );
			glUniform4f(param1, 1.0f /*POINT light*/, 15.0f, 15.0f, 1.0f);
			glUniform4f(param2, 1.0f /*Light is on*/, 0.0f, 0.0f, 1.0f);
		}

		// Also set the position of my "eye" (the camera)
		//uniform vec4 eyeLocation;
		GLint eyeLocation_UL = glGetUniformLocation( shaderProgID, "eyeLocation");

		glUniform4f( eyeLocation_UL, 
					 cameraEyeMain.x, cameraEyeMain.y, cameraEyeMain.z, 1.0f );

		float flicker = randInRange(0.000072f, 1.0f);
		pWorld->m_vec_pLights.at(1)->setConstAtten(flicker);
		pWorld->m_vec_pLights.at(2)->setConstAtten(flicker);

		std::stringstream ssTitle;
		ssTitle
			<< "X: "
			<< cameraEyeMain.x << ", "
			<< "Y: "
			<< cameraEyeMain.y << ", "
			<< "Z: "
			<< cameraEyeMain.z;
			//<< "Const Atten: "
			//<< pLightsVec.at(currentLight)->getConstAtten() << " , "
			//<< "Linear Atten: "
			//<< pLightsVec.at(currentLight)->getLinearAtten() << " , "
			//<< "Quadratic Atten: "
			//<< pLightsVec.at(currentLight)->getQuadraticAtten();
		glfwSetWindowTitle( window, ssTitle.str().c_str() );


		GLint matView_UL = glGetUniformLocation(shaderProgID, "matView");
		GLint matProj_UL = glGetUniformLocation(shaderProgID, "matProj");

		glUniformMatrix4fv(matView_UL, 1, GL_FALSE, glm::value_ptr(v));
		glUniformMatrix4fv(matProj_UL, 1, GL_FALSE, glm::value_ptr(p));


		// **************************************************
		// **************************************************
		// Loop to draw everything in the scene


		for (int index = 0; index != ::g_vec_pGameObjects.size(); index++)
		{
			glm::mat4 matModel = glm::mat4(1.0f);

			cGameObject* pCurrentObject = ::g_vec_pGameObjects[index];

			DrawObject( matModel, pCurrentObject, 
					   shaderProgID, pTheVAOManager);

		}//for (int index...
		for (int index = 0; index != ::g_vec_pWallObjects.size(); index++)
		{
			glm::mat4 matModel = glm::mat4(1.0f);

			cGameObject* pCurrentObject = ::g_vec_pWallObjects[index];

			DrawObject(matModel, pCurrentObject,
				shaderProgID, pTheVAOManager);

		}

		if (bLightDebugSheresOn) 
		{
			{// Draw where the light is at
				for (int i = 0; i < pWorld->m_vec_pLights.size(); ++i)
				{
					glm::mat4 matModel = glm::mat4(1.0f);
					pDebugSphere->positionXYZ = pWorld->m_vec_pLights.at(i)->_positionXYZ;
					pDebugSphere->scale = 0.5f;
					pDebugSphere->debugColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
					pDebugSphere->isWireframe = true;
					DrawObject(matModel, pDebugSphere,
						shaderProgID, pTheVAOManager);
				}
			}

			// Draw spheres to represent the attenuation...
			{   // Draw a sphere at 1% brightness
				glm::mat4 matModel = glm::mat4(1.0f);
				pDebugSphere->positionXYZ.x = pWorld->m_vec_pLights.at(currentLight)->getPositionX();
				pDebugSphere->positionXYZ.y = pWorld->m_vec_pLights.at(currentLight)->getPositionY();
				pDebugSphere->positionXYZ.z = pWorld->m_vec_pLights.at(currentLight)->getPositionZ();
				float sphereSize = pLightHelper->calcApproxDistFromAtten( 
													   0.01f,		// 1% brightness (essentially black)
													   0.001f,		// Within 0.1%  
													   100000.0f,	// Will quit when it's at this distance
					pWorld->m_vec_pLights.at(currentLight)->getConstAtten(),
					pWorld->m_vec_pLights.at(currentLight)->getLinearAtten(),
					pWorld->m_vec_pLights.at(currentLight)->getQuadraticAtten() );
				pDebugSphere->scale = sphereSize;
				pDebugSphere->debugColour = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
				pDebugSphere->isWireframe = true;
				DrawObject(matModel, pDebugSphere,
						   shaderProgID, pTheVAOManager);
			}
			{   // Draw a sphere at 25% brightness
				glm::mat4 matModel = glm::mat4(1.0f);
				pDebugSphere->positionXYZ.x = pWorld->m_vec_pLights.at(currentLight)->getPositionX();
				pDebugSphere->positionXYZ.y = pWorld->m_vec_pLights.at(currentLight)->getPositionY();
				pDebugSphere->positionXYZ.z = pWorld->m_vec_pLights.at(currentLight)->getPositionZ();
				float sphereSize = pLightHelper->calcApproxDistFromAtten( 
													   0.25f,		// 1% brightness (essentially black)
													   0.001f,		// Within 0.1%  
													   100000.0f,	// Will quit when it's at this distance
					pWorld->m_vec_pLights.at(currentLight)->getConstAtten(),
					pWorld->m_vec_pLights.at(currentLight)->getLinearAtten(),
					pWorld->m_vec_pLights.at(currentLight)->getQuadraticAtten());
				pDebugSphere->scale = sphereSize;
				pDebugSphere->debugColour = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
				pDebugSphere->isWireframe = true;
				DrawObject(matModel, pDebugSphere,
						   shaderProgID, pTheVAOManager);
			}
			{   // Draw a sphere at 50% brightness
				glm::mat4 matModel = glm::mat4(1.0f);
				pDebugSphere->positionXYZ.x = pWorld->m_vec_pLights.at(currentLight)->getPositionX();
				pDebugSphere->positionXYZ.y = pWorld->m_vec_pLights.at(currentLight)->getPositionY();
				pDebugSphere->positionXYZ.z = pWorld->m_vec_pLights.at(currentLight)->getPositionZ();
				float sphereSize = pLightHelper->calcApproxDistFromAtten( 
													   0.50f,		// 1% brightness (essentially black)
													   0.001f,		// Within 0.1%  
													   100000.0f,	// Will quit when it's at this distance
					pWorld->m_vec_pLights.at(currentLight)->getConstAtten(),
					pWorld->m_vec_pLights.at(currentLight)->getLinearAtten(),
					pWorld->m_vec_pLights.at(currentLight)->getQuadraticAtten());
				pDebugSphere->scale = sphereSize;
				pDebugSphere->debugColour = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
				pDebugSphere->isWireframe = true;
				DrawObject(matModel, pDebugSphere,
						   shaderProgID, pTheVAOManager);
			}
			{   // Draw a sphere at 75% brightness
				glm::mat4 matModel = glm::mat4(1.0f);
				pDebugSphere->positionXYZ.x = pWorld->m_vec_pLights.at(currentLight)->getPositionX();
				pDebugSphere->positionXYZ.y = pWorld->m_vec_pLights.at(currentLight)->getPositionY();
				pDebugSphere->positionXYZ.z = pWorld->m_vec_pLights.at(currentLight)->getPositionZ();
				float sphereSize = pLightHelper->calcApproxDistFromAtten( 
													   0.75f,		// 1% brightness (essentially black)
													   0.001f,		// Within 0.1%  
													   100000.0f,	// Will quit when it's at this distance
					pWorld->m_vec_pLights.at(currentLight)->getConstAtten(),
					pWorld->m_vec_pLights.at(currentLight)->getLinearAtten(),
					pWorld->m_vec_pLights.at(currentLight)->getQuadraticAtten());
				pDebugSphere->scale = sphereSize;
				pDebugSphere->debugColour = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
				pDebugSphere->isWireframe = true;
				DrawObject(matModel, pDebugSphere,
						   shaderProgID, pTheVAOManager);
			}
			{   // Draw a sphere at 95% brightness
				glm::mat4 matModel = glm::mat4(1.0f);
				pDebugSphere->positionXYZ.x = pWorld->m_vec_pLights.at(currentLight)->getPositionX();
				pDebugSphere->positionXYZ.y = pWorld->m_vec_pLights.at(currentLight)->getPositionY();
				pDebugSphere->positionXYZ.z = pWorld->m_vec_pLights.at(currentLight)->getPositionZ();
				float sphereSize = pLightHelper->calcApproxDistFromAtten( 
													   0.95f,		// 1% brightness (essentially black)
													   0.001f,		// Within 0.1%  
													   100000.0f,	// Will quit when it's at this distance
					pWorld->m_vec_pLights.at(currentLight)->getConstAtten(),
					pWorld->m_vec_pLights.at(currentLight)->getLinearAtten(),
					pWorld->m_vec_pLights.at(currentLight)->getQuadraticAtten() );
				pDebugSphere->scale = sphereSize;
				pDebugSphere->debugColour = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
				pDebugSphere->isWireframe = true;
				DrawObject(matModel, pDebugSphere,
						   shaderProgID, pTheVAOManager);
			}
		}// if (bLightDebugSheresOn) 

		 // **************************************************
		// *************************************************
		if (fileChanged)
		{
			//file.open(gameDataLocation);
			file << "<?xml version='1.0' encoding='utf-8'?>\n";
			document.save_file(gameDataLocation.c_str());
			//file.close();
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}// main loop

	glfwDestroyWindow(window);
	glfwTerminate();

	// Delete everything
	delete pTheModelLoader;
//	delete pTheVAOManager;

	for (int i = 0; i < nameVec.size(); i++)
	{
		meshNameFile << nameVec.at(i);
	}

	// Watch out!!
	// sVertex* pVertices = new sVertex[numberOfVertsOnGPU];
//	delete [] pVertices;		// If it's an array, also use [] bracket

	exit(EXIT_SUCCESS);
}


void DrawObject(glm::mat4 m, cGameObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager)
{
	// 
				//         mat4x4_identity(m);
	m = glm::mat4(1.0f);



	// ******* TRANSLATION TRANSFORM *********
	glm::mat4 matTrans
	= glm::translate(glm::mat4(1.0f),
					 glm::vec3(pCurrentObject->positionXYZ.x,
							   pCurrentObject->positionXYZ.y,
							   pCurrentObject->positionXYZ.z));
	m = m * matTrans;
	// ******* TRANSLATION TRANSFORM *********



	// ******* ROTATION TRANSFORM *********
	//mat4x4_rotate_Z(m, m, (float) glfwGetTime());
	glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f),
									pCurrentObject->rotationXYZ.z,					// Angle 
									glm::vec3(0.0f, 0.0f, 1.0f));
	m = m * rotateZ;

	glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f),
									pCurrentObject->rotationXYZ.y,	//(float)glfwGetTime(),					// Angle 
									glm::vec3(0.0f, 1.0f, 0.0f));
	m = m * rotateY;

	glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f),
									pCurrentObject->rotationXYZ.x,	// (float)glfwGetTime(),					// Angle 
									glm::vec3(1.0f, 0.0f, 0.0f));
	m = m * rotateX;
	// ******* ROTATION TRANSFORM *********



	// ******* SCALE TRANSFORM *********
	glm::mat4 scale = glm::scale(glm::mat4(1.0f),
								 glm::vec3(pCurrentObject->scale,
										   pCurrentObject->scale,
										   pCurrentObject->scale));
	m = m * scale;
	// ******* SCALE TRANSFORM *********



	//mat4x4_mul(mvp, p, m);
	//mvp = p * v * m;

	// Choose which shader to use
	//glUseProgram(program);
	glUseProgram(shaderProgID);


	//glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
	//glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));

	//uniform mat4 matModel;		// Model or World 
	//uniform mat4 matView; 		// View or camera
	//uniform mat4 matProj;
	GLint matModel_UL = glGetUniformLocation(shaderProgID, "matModel");

	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(m));
	//glUniformMatrix4fv(matView_UL, 1, GL_FALSE, glm::value_ptr(v));
	//glUniformMatrix4fv(matProj_UL, 1, GL_FALSE, glm::value_ptr(p));



	// Find the location of the uniform variable newColour
	GLint newColour_location = glGetUniformLocation(shaderProgID, "newColour");

	glUniform3f(newColour_location,
				pCurrentObject->objectColourRGBA.r,
				pCurrentObject->objectColourRGBA.g,
				pCurrentObject->objectColourRGBA.b);

	GLint diffuseColour_UL = glGetUniformLocation(shaderProgID, "diffuseColour");
	glUniform4f(diffuseColour_UL, 
				pCurrentObject->objectColourRGBA.r,
				pCurrentObject->objectColourRGBA.g,
				pCurrentObject->objectColourRGBA.b,
				pCurrentObject->objectColourRGBA.a);	// 

	GLint specularColour_UL = glGetUniformLocation(shaderProgID, "specularColour");
	glUniform4f(specularColour_UL,
				1.0f,	// R
				1.0f,	// G
				1.0f,	// B
				1000.0f);	// Specular "power" (how shinny the object is)
	                        // 1.0 to really big (10000.0f)


	//uniform vec4 debugColour;
	//uniform bool bDoNotLight;
	GLint debugColour_UL = glGetUniformLocation(shaderProgID, "debugColour");
	GLint bDoNotLight_UL = glGetUniformLocation(shaderProgID, "bDoNotLight");

	if ( pCurrentObject->isWireframe )
	{ 
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		// LINES
		glUniform4f( debugColour_UL, 
					pCurrentObject->debugColour.r,
					pCurrentObject->debugColour.g,
					pCurrentObject->debugColour.b,
					pCurrentObject->debugColour.a);
		glUniform1f(bDoNotLight_UL, (float)GL_TRUE);
	}
	else
	{	// Regular object (lit and not wireframe)
		glUniform1f(bDoNotLight_UL, (float)GL_FALSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);		// SOLID
	}
	//glPointSize(15.0f);

	if (pCurrentObject->disableDepthBufferTest)
	{
		glDisable(GL_DEPTH_TEST);					// DEPTH Test OFF
	}
	else
	{
		glEnable(GL_DEPTH_TEST);						// Turn ON depth test
	}

	if (pCurrentObject->disableDepthBufferWrite)
	{
		glDisable(GL_DEPTH);						// DON'T Write to depth buffer
	}
	else
	{
		glEnable(GL_DEPTH);								// Write to depth buffer
	}


	//		glDrawArrays(GL_TRIANGLES, 0, 2844);
	//		glDrawArrays(GL_TRIANGLES, 0, numberOfVertsOnGPU);

	sModelDrawInfo drawInfo;
	//if (pTheVAOManager->FindDrawInfoByModelName("bunny", drawInfo))
	if (pVAOManager->FindDrawInfoByModelName(pCurrentObject->meshName, drawInfo))
	{
		glBindVertexArray(drawInfo.VAO_ID);
		glDrawElements(GL_TRIANGLES,
					   drawInfo.numberOfIndices,
					   GL_UNSIGNED_INT,
					   0);
		glBindVertexArray(0);
	}
	
	return;
} // DrawObject;
// 

// returns NULL (0) if we didn't find it.
cGameObject* pFindObjectByFriendlyName(std::string name)
{
	// Do a linear search 
	for (unsigned int index = 0;
		 index != g_vec_pGameObjects.size(); index++)
	{
		if (::g_vec_pGameObjects[index]->friendlyName == name)
		{
			// Found it!!
			return ::g_vec_pGameObjects[index];
		}
	}
	for (unsigned int index = 0;
		index != g_vec_pWallObjects.size(); index++)
	{
		if (::g_vec_pWallObjects[index]->friendlyName == name)
		{
			// Found it!!
			return ::g_vec_pWallObjects[index];
		}
	}
	// Didn't find it
	return NULL;
}

// returns NULL (0) if we didn't find it.
cGameObject* pFindObjectByFriendlyNameMap(std::string name)
{
	//std::map<std::string, cGameObject*> g_map_GameObjectsByFriendlyName;
	return ::g_map_GameObjectsByFriendlyName[name];
}

glm::mat4 calculateWorldMatrix(cGameObject* pCurrentObject)
{

	glm::mat4 matWorld = glm::mat4(1.0f);


	// ******* TRANSLATION TRANSFORM *********
	glm::mat4 matTrans
		= glm::translate(glm::mat4(1.0f),
			glm::vec3(pCurrentObject->positionXYZ.x,
				pCurrentObject->positionXYZ.y,
				pCurrentObject->positionXYZ.z));
	matWorld = matWorld * matTrans;
	// ******* TRANSLATION TRANSFORM *********



	// ******* ROTATION TRANSFORM *********
	//mat4x4_rotate_Z(m, m, (float) glfwGetTime());
	glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f),
		pCurrentObject->rotationXYZ.z,					// Angle 
		glm::vec3(0.0f, 0.0f, 1.0f));
	matWorld = matWorld * rotateZ;

	glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f),
		pCurrentObject->rotationXYZ.y,	//(float)glfwGetTime(),					// Angle 
		glm::vec3(0.0f, 1.0f, 0.0f));
	matWorld = matWorld * rotateY;

	glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f),
		pCurrentObject->rotationXYZ.x,	// (float)glfwGetTime(),					// Angle 
		glm::vec3(1.0f, 0.0f, 0.0f));
	matWorld = matWorld * rotateX;
	// ******* ROTATION TRANSFORM *********



	// ******* SCALE TRANSFORM *********
	glm::mat4 scale = glm::scale(glm::mat4(1.0f),
		glm::vec3(pCurrentObject->scale,
			pCurrentObject->scale,
			pCurrentObject->scale));
	matWorld = matWorld * scale;
	// ******* SCALE TRANSFORM *********


	return matWorld;
}