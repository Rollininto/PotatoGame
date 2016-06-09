#include "DataStorage.h"

int DataStorage::Err = 0;

DataStorage::DataStorage()
{
	Err = 0;
}

DataStorage::~DataStorage()
{
}

void DataStorage::writeUserData(sUserData ud)
{
	Err = 0;
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Users.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	if (pRoot != NULL) {
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
	else {
		Err = 1;
	}
}

void DataStorage::updateUserData(sUserData ud)
{
	Err = 0;
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Users.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	if (pRoot != NULL) {
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
	else {
		Err = 1;
	}
}

sUserData DataStorage::getUserData(std::string login) {
	Err = 0;
	sUserData udret;
	udret.login = login;
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Users.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	if (pRoot != NULL) {
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
	else {
		Err = 1;
		return sUserData();
	}
}

bool DataStorage::userExsists(std::string login) {
	Err = 0;
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Users.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	if (pRoot != NULL) {
		tinyxml2::XMLElement * pUser = pRoot->FirstChildElement(login.c_str());
		if (pUser == nullptr)
			return false;
		return true;
	}
	return false;
}

sUsersLevelResult DataStorage::getUsersLevelResult(sUserData ud, sLevelData ld)
{
	Err = 0;
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
	if (pRoot != NULL) {
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
	else {
		Err = 1;
		return sUsersLevelResult();
	}
}

void DataStorage::updateUsersLevelResult(sUserData ud, sLevelData ld, sUsersLevelResult res)
{
	Err = 0;
	int lId = -1;
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Users.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	if (pRoot != NULL) {
		tinyxml2::XMLElement * pUser = pRoot->FirstChildElement(ud.login.c_str());
		if (pUser == nullptr) {
			Err = 1;
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
				pLevelRes->SetAttribute("BestBonusCoins", res.userBestBonusCoins);
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
	else {
		Err = 1;
	}
}

Character DataStorage::getCharById(int id)
{
	Err = 0;
	int chId = -1;
	Character retChar;
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Characters.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	if (pRoot != NULL) {
		tinyxml2::XMLElement * pChar = pRoot->FirstChildElement("Character");
		while (pChar != NULL) {
			pChar->QueryIntAttribute("Id", &chId);
			if (chId == id) {
				break;
			}
			pChar = pChar->NextSiblingElement("Character");
		}
		if (pChar == nullptr) {
			pChar = pRoot->FirstChildElement("ShopCharacter");
			while (pChar != NULL) {
				pChar->QueryIntAttribute("Id", &chId);
				if (chId == id) {
					break;
				}
				pChar = pChar->NextSiblingElement("ShopCharacter");
			}
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
	else {
		Err = 1;
		return Character();
	}
}

std::vector<Character> DataStorage::getChars()
{
	Err = 0;
	std::vector<Character> retChars;
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Characters.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	if (pRoot != NULL) {
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
		doc.Clear();
		return retChars;
	}
	else {
		Err = 1;
		return std::vector<Character>();
	}
}

std::vector<StoreCharacter> DataStorage::getShopChars()
{
	Err = 0;
	std::vector<StoreCharacter> retChars;
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Characters.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	if (pRoot != NULL) {
		tinyxml2::XMLElement * pChar = pRoot->FirstChildElement("ShopCharacter");
		while (pChar != NULL) {
			StoreCharacter ch;
			pChar->QueryIntAttribute("Id", &ch.id);
			const char * szAttributeText = nullptr;
			szAttributeText = pChar->Attribute("Path");
			if (szAttributeText != nullptr)
				ch.path = szAttributeText;
			pChar->QueryIntAttribute("Price", &ch.price);
			retChars.push_back(ch);
			pChar = pChar->NextSiblingElement("ShopCharacter");
		}
		return retChars;
	}
	else {
		Err = 1;
		return std::vector<StoreCharacter>();
	}
}

void DataStorage::AddBoughtChar(std::string login, int charId)
{
	Err = 0;
	int cId = -1;
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Users.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	if (pRoot != NULL) {
		tinyxml2::XMLElement * pUser = pRoot->FirstChildElement(login.c_str());
		if (pUser != nullptr) {
			tinyxml2::XMLElement * pChar = doc.NewElement("StoreCharacter");
			pChar->SetAttribute("Id", charId);
			pUser->InsertEndChild(pChar);
			doc.SaveFile("Resources/Data/Users.xml");
		}
	}
	else {
		Err = 1;		
	}
}

bool DataStorage::IsBoughtChar(std::string login, int charId)
{
	Err = 0;
	bool ret = false;
	int chId = -1;
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Users.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	if (pRoot != NULL) {
		tinyxml2::XMLElement * pUser = pRoot->FirstChildElement(login.c_str());
		if (pUser == nullptr) {
			Err = 1;
			return false;
		}
		else {
			tinyxml2::XMLElement * pChar = pUser->FirstChildElement("StoreCharacter");
			while (pChar != NULL) {
				pChar->QueryIntAttribute("Id", &chId);
				if (chId == charId) {
					ret = true;
					break;
				}
				pChar = pChar->NextSiblingElement("StoreCharacter");
			}
		}
		return ret;
	}
	else {
		Err = 1;
		return false;
	}
}

void DataStorage::updateBestResult(sLevelData ld)
{
	Err = 0;
	int id;
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Levels.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	if (pRoot != NULL) {
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
	else {
		Err = 1;
	}
}

int DataStorage::getOverLvlCnt() {
	Err = 0;
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Levels.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	if (pRoot != NULL) {
		tinyxml2::XMLElement * pLevel = pRoot->FirstChildElement("Level");
		int cnt = 0;
		while (pLevel != nullptr) {
			cnt++;
			pLevel = pLevel->NextSiblingElement("Level");
		}
		return cnt;
	}
	else {
		Err = 1;
		return 0;
	}
}

sLevelData DataStorage::getLevelData(int id) {
	Err = 0;
	sLevelData ldret;

	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Levels.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	if (pRoot != NULL) {
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
	else {
		Err = 1;
		return sLevelData();
	}
}

std::vector<sLevelData> DataStorage::getAvailableLevelsData() {
	Err = 0;
	std::vector<sLevelData> ldret;

	tinyxml2::XMLDocument doc;
	doc.LoadFile("Resources/Data/Levels.xml");
	tinyxml2::XMLNode * pRoot = doc.FirstChild();
	if (pRoot != NULL) {
		tinyxml2::XMLElement * pLevel = pRoot->FirstChildElement("Level");
		while (pLevel != nullptr)
		{
			sLevelData levret;

			pLevel->QueryIntAttribute("Id", &levret.id);

			tinyxml2::XMLError eResult;
			eResult = pLevel->QueryIntAttribute("ReqCoins", &levret.reqCoins);
			eResult = pLevel->QueryIntAttribute("BonusCoins", &levret.bonusCoins);

			const char * szAttributeText = nullptr;
			szAttributeText = pLevel->Attribute("Title");
			if (szAttributeText != nullptr)
				levret.title = szAttributeText;

			eResult = pLevel->QueryIntAttribute("BestTime", &levret.bestTime);

			szAttributeText = nullptr;
			szAttributeText = pLevel->Attribute("BestUser");
			if (szAttributeText != nullptr)
				levret.bestUserLogin = szAttributeText;
			pLevel = pLevel->NextSiblingElement("Level");
			ldret.push_back(levret);
		}
		return ldret;
	}
	else {
		Err = 1;
		return std::vector<sLevelData>();
	}
}
