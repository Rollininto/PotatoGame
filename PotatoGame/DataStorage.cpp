#include "DataStorage.h"

DataStorage::DataStorage()
{
}

DataStorage::~DataStorage()
{
}

void DataStorage::writeUserData(sUserData ud)
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Users.xml");

	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	tinyxml2::XMLElement * pUser = doc.NewElement(ud.login.c_str());
	pUser->SetAttribute("Coins", ud.coins);
	pUser->SetAttribute("PersId", ud.persId);
	pUser->SetAttribute("LevelsPassed", ud.levelsPassed);
	pRoot->InsertEndChild(pUser);
	tinyxml2::XMLError eResult = doc.SaveFile("Resources/Data/Users.xml");
	if (eResult != tinyxml2::XML_SUCCESS) {
		std::ostringstream es("");
		es << "Failed to save xml! Error: " << eResult << std::endl;
		OutputDebugString(es.str().c_str());
	}
}

void DataStorage::updateUserData(sUserData ud)
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Users.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	tinyxml2::XMLElement * pUser = pRoot->FirstChildElement(ud.login.c_str());
	pUser->SetAttribute("Coins", ud.coins);
	pUser->SetAttribute("PersId", ud.persId);
	pUser->SetAttribute("LevelsPassed", ud.levelsPassed);
	pRoot->InsertEndChild(pUser);
	tinyxml2::XMLError eResult = doc.SaveFile("Resources/Data/Users.xml");
	if (eResult != tinyxml2::XML_SUCCESS) {
		std::ostringstream es("");
		es << "Failed to save xml! Error: " << eResult << std::endl;
		OutputDebugString(es.str().c_str());
	}
}

sUserData DataStorage::getUserData(std::string login) {
	sUserData udret;
	udret.login = login;
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Users.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	tinyxml2::XMLElement * pUser = pRoot->FirstChildElement(login.c_str());
	if (pUser == nullptr) {
		udret.coins = 0;
		udret.persId = 1;
		udret.levelsPassed = 0;
		writeUserData(udret);
	}
	else {
		tinyxml2::XMLError eResult;
		eResult = pUser->QueryIntAttribute("Coins", &udret.coins);
		eResult = pUser->QueryIntAttribute("PersId", &udret.persId);
		eResult = pUser->QueryIntAttribute("LevelsPassed", &udret.levelsPassed);
	}
	udret.UpToDate = true;
	return udret;
}

bool DataStorage::userExsists(std::string login) {
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Users.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	tinyxml2::XMLElement * pUser = pRoot->FirstChildElement(login.c_str());
	if (pUser == nullptr)
		return false;
	return true;
}

sUsersLevelResult DataStorage::getUsersLevelResult(sUserData ud, sLevelData ld)
{
	sUsersLevelResult res;
	if (ud.login == "Guest") {
		res.userBestBonusCoins = 0;
		res.userBestTime = 0;
		return res;
	}
	int lId = -1;
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Users.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	tinyxml2::XMLElement * pUser = pRoot->FirstChildElement(ud.login.c_str());
	if (pUser != nullptr) {
		tinyxml2::XMLElement * pLevelRes = pUser->FirstChildElement("LevelRes");
		while (pLevelRes != nullptr)
		{
			pLevelRes->QueryIntAttribute("Id", &lId);
			if (lId == ld.id) {
				break;
			}
			pLevelRes = pLevelRes->NextSiblingElement("LevelRes");
		}
		if (pLevelRes != nullptr) {
			pLevelRes->QueryIntAttribute("BestBonusCoins", &res.userBestBonusCoins);
			pLevelRes->QueryIntAttribute("BestTime", &res.userBestTime);
		}
		else {
			res.userBestBonusCoins = 0;
			res.userBestTime = 0;
		}
	}
	return res;
}

void DataStorage::updateUsersLevelResult(sUserData ud, sLevelData ld, sUsersLevelResult res)
{
	int lId = -1;
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Users.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	tinyxml2::XMLElement * pUser = pRoot->FirstChildElement(ud.login.c_str());
	if (pUser == nullptr) {
		//fatal error
	}
	else {
		tinyxml2::XMLElement * pLevelRes = pUser->FirstChildElement("LevelRes");
		while (pLevelRes != nullptr)
		{
			pLevelRes->QueryIntAttribute("Id", &lId);
			if (lId == ld.id) {
				break;
			}
			pLevelRes = pLevelRes->NextSiblingElement("LevelRes");
		}
		if (pLevelRes == nullptr) {
			pLevelRes = doc.NewElement("LevelRes");
			pLevelRes->SetAttribute("Id", ld.id);
			pLevelRes->SetAttribute("BestBonusCoins",res.userBestBonusCoins);
			pLevelRes->SetAttribute("BestTime", res.userBestTime);
			pUser->InsertEndChild(pLevelRes);
		}
		else {
			pLevelRes->SetAttribute("BestBonusCoins", res.userBestBonusCoins);
			pLevelRes->SetAttribute("BestTime", res.userBestTime);
		}
		doc.SaveFile("Resources/Data/Users.xml");
	}	
}

Character DataStorage::getCharById(int id)
{
	int chId = -1;
	Character retChar;
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Characters.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	tinyxml2::XMLElement * pChar = pRoot->FirstChildElement("Character");
	while (pChar != NULL) {
		pChar->QueryIntAttribute("Id", &chId);
		if (chId == id) {
			break;
		}
		pChar = pChar->NextSiblingElement("Character");
	}
	if (pChar != nullptr) {
		retChar.id = id;
		const char * szAttributeText = nullptr;
		szAttributeText = pChar->Attribute("Path");
		if (szAttributeText != nullptr)
			retChar.path = szAttributeText;
	}
	return retChar;
}

std::vector<Character> DataStorage::getChars()
{
	std::vector<Character> retChars;
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Characters.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	tinyxml2::XMLElement * pChar = pRoot->FirstChildElement("Character");
	while (pChar != NULL) {
		Character ch;
		pChar->QueryIntAttribute("Id", &ch.id);
		const char * szAttributeText = nullptr;
		szAttributeText = pChar->Attribute("Path");
		if (szAttributeText != nullptr)
			ch.path = szAttributeText;
		retChars.push_back(ch);
		pChar = pChar->NextSiblingElement("Character");
	}
	return retChars;
}

void DataStorage::updateBestResult(sLevelData ld)
{
	int id;
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Levels.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	tinyxml2::XMLElement * pLevel = pRoot->FirstChildElement("Level");
	while (pLevel != nullptr)
	{
		pLevel->QueryIntAttribute("Id", &id);
		if (ld.id == id) {
			break;
		}
		pLevel = pLevel->NextSiblingElement("Level");
	}
	if (pLevel == nullptr) {

	}
	else {
		pLevel->SetAttribute("BestTime", ld.bestTime);
		pLevel->SetAttribute("BestUser", ld.bestUserLogin.c_str());
	}
	doc.SaveFile("Resources/Data/Levels.xml");
}

int DataStorage::getOverLvlCnt() {
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Levels.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	return 0;
}

sLevelData DataStorage::getLevelData(int id) {
	sLevelData ldret;

	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Levels.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	tinyxml2::XMLElement * pLevel = pRoot->FirstChildElement("Level");
	while (pLevel != nullptr)
	{
		pLevel->QueryIntAttribute("Id", &ldret.id);
		if (ldret.id == id) {
			break;
		}
		pLevel = pLevel->NextSiblingElement("Level");
	}
	if (pLevel == nullptr) {

	}
	else {
		tinyxml2::XMLError eResult;
		eResult = pLevel->QueryIntAttribute("ReqCoins", &ldret.reqCoins);
		eResult = pLevel->QueryIntAttribute("BonusCoins", &ldret.bonusCoins);

		const char * szAttributeText = nullptr;
		szAttributeText = pLevel->Attribute("Title");
		if (szAttributeText != nullptr)
			ldret.title = szAttributeText;

		eResult = pLevel->QueryIntAttribute("BestTime", &ldret.bestTime);

		szAttributeText = nullptr;
		szAttributeText = pLevel->Attribute("BestUser");
		if (szAttributeText != nullptr)
			ldret.bestUserLogin = szAttributeText;
	}
	return ldret;
}

