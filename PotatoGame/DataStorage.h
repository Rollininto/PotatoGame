#pragma once
#include <filesystem>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <vector>
#include <Windows.h>
#include <WinBase.h>
#include <tinyxml2.h>

struct Character {
	int id;
	std::string path;
};
struct StoreCharacter {
	int id;
	std::string path;
	int price;
	bool is_bought;
};
struct sUserData {
	std::string login;
	int coins;
	int persId;
	int levelsPassed;
	bool UpToDate = true;
};
struct sLevelData {
	int id;
	std::string title;
	int reqCoins;
	int bonusCoins;
	int bestTime;
	std::string bestUserLogin;
};
struct sUsersLevelResult {	
	int userBestTime;
	int userBestBonusCoins;
};


class  DataStorage
{
public:
	DataStorage();
	~ DataStorage();
	static int Err;

	static void writeUserData(sUserData ud);
	static void updateUserData(sUserData ud);
	static sUserData getUserData(std::string login);
	static void updateBestResult(sLevelData ld);
	static int getOverLvlCnt();
	static sLevelData getLevelData(int id);
	static std::vector<sLevelData> getAvailableLevelsData();
	static bool userExsists(std::string login);
	static sUsersLevelResult getUsersLevelResult(sUserData ud, sLevelData ld);
	static void updateUsersLevelResult(sUserData ud, sLevelData ld, sUsersLevelResult res);
	static Character getCharById(int id);
	static std::vector<Character> getChars();
	static std::vector<StoreCharacter> getShopChars();
	static void AddBoughtChar(std::string login, int charId);
	static bool IsBoughtChar(std::string login, int charId);
private:
};