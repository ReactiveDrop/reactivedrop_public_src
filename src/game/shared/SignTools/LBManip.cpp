#include "LBManip.h"
#include "Keys.h"


std::string LBRawToHexString(CSteamID userID, SteamLeaderboard_t m_hSteamLeaderboard, int32 m_iLeaderboardScore, LeaderboardScoreDetails_v2_t m_LeaderboardScoreDetails)
{
	//Output identification string
	LBDataUnion u = {};
	u.rawData.userID = userID;
	u.rawData.m_iLeaderboardScore = m_iLeaderboardScore;
	u.rawData.m_LeaderboardScoreDetails = m_LeaderboardScoreDetails;
	u.rawData.m_hSteamLeaderboard = m_hSteamLeaderboard;
	//stores hash of userID to protect privacy
	
	//generating sha256 hex string. SteamID's SHA256 result is hashed into the overal result, so that we can check the owner of the record later when uploading and displaying.
	auto vecUnion = std::vector<unsigned char>(sizeof(LBDataUnion) - picosha2::k_digest_size, 0);
	memcpy(vecUnion.data(), &u, vecUnion.size());
	std::vector<unsigned char> hash_result(picosha2::k_digest_size);
	picosha2::hash256(vecUnion, hash_result);
	memcpy(u.rawData.SHA256, hash_result.data(), picosha2::k_digest_size);

	//now we discard the userID info and convert the rest of union to hex string.
	return CharArrayToHexString(u.byteData + sizeof(CSteamID), sizeof(LBDataUnion) - sizeof(CSteamID));
}

bool SignatureStringToLBRaw(CSteamID userID, std::string inSignHexStr, LBDataUnion& outLBData)
{
	//converts it back to raw bytes
	auto tmpSignByteArray = HexStringToCharArray(inSignHexStr);

	//now we need to verify the signature, currently the public key is hard coded into Keys.h. ther is no need to read it from file cause not much public keys needed.
	auto keyList = GetPublicKeyList();

	//the signature may come from different moderators, so we need to verify all possiblilities
	for each (auto key in keyList)
	{
		//ensure the string is long enough to store the message
		std::string msgHexStr = std::string();
		msgHexStr.resize(tmpSignByteArray.size());
		unsigned long long int msgLen;

		if (crypto_sign_open((unsigned char*)(msgHexStr.data()), &msgLen, tmpSignByteArray.data(), tmpSignByteArray.size(), key.data()) == 0)
		{
			//this is a valid message in hex string
			//resize message_hex_string to its real length and converts it back to raw bytes
			msgHexStr.resize(msgLen);
			auto tempByteArray = HexStringToCharArray(msgHexStr);

			//the message in raw bytes should have the length of score+details+leaderboardhandle+sha256
			if (tempByteArray.size() != sizeof(LBDataUnion) - sizeof(CSteamID))
			{
				continue;
			}

			//now we resotre union LBDataUnion
			outLBData.rawData.userID = userID;
			memcpy(outLBData.byteData + sizeof(CSteamID), tempByteArray.data(), sizeof(LBDataUnion) - sizeof(CSteamID));

			//now we need to check if this record belongs to the user.
			//construct temp vecUnion 
			auto vecUnion = std::vector<unsigned char>(sizeof(LBDataUnion) - picosha2::k_digest_size, 0);
			memcpy(vecUnion.data(), &outLBData, vecUnion.size());
			std::vector<unsigned char> hashResult(picosha2::k_digest_size);
			picosha2::hash256(vecUnion, hashResult);

			//now check the hash
			for (int i = 0; i < picosha2::k_digest_size; i++)
			{
				if (hashResult[i] != outLBData.rawData.SHA256[i])
				{
					return false;
				}
			}
			return true;
		}

	}
	return false;
}