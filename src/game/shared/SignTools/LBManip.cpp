#include "LBManip.h"
#include "Keys.h"


std::string LBDataToHexString(CSteamID userID, SteamLeaderboard_t hSteamLeaderboard, int32 iLeaderboardScore, LeaderboardScoreDetails_v2_t LeaderboardScoreDetails)
{
	//Output identification string
	LBDataUnion u = {};
	u.data.userID = userID;
	u.data.score = iLeaderboardScore;
	u.data.details = LeaderboardScoreDetails;
	u.data.hLeaderboard = hSteamLeaderboard;

	//generating sha256 hex string. SteamID is hashed into the result, so that we can check the owner of the record later when uploading and displaying.
	auto vecUnion = std::vector<unsigned char>(sizeof(LBDataUnion) - picosha2::k_digest_size, 0);
	memcpy(vecUnion.data(), &u, vecUnion.size());
	std::vector<unsigned char> hashResult(picosha2::k_digest_size);
	picosha2::hash256(vecUnion, hashResult);
	memcpy(u.data.SHA256, hashResult.data(), picosha2::k_digest_size);

	//now we convert the union to hex string and append sha256 string to it.
	return CharArrayToHexStr(u.rawBytes + sizeof(CSteamID), sizeof(LBDataUnion) - sizeof(CSteamID));
}

bool SignHexStringToLBData(CSteamID userID, std::string signHexStr, LBDataUnion& outLBDetails)
{
	auto tmpSignByteArray = HexStrToCharArray(signHexStr);

	//now we need to verify the signature, currently the public key is hard coded into Keys.h. ther is no need to read it from file cause not much public keys needed.
	auto keyList = GetPublicKeyList();

	//the signature may come from different moderators, so we need to verify all possiblilities
	for each (auto key in keyList)
	{
		//ensure the string is long enough to store the message
		std::string tmpMsgHexStr = std::string();
		tmpMsgHexStr.resize(tmpSignByteArray.size());
		unsigned long long int mlen;

		if (crypto_sign_open((unsigned char*)(tmpMsgHexStr.data()), &mlen, tmpSignByteArray.data(), tmpSignByteArray.size(), key.data()) == 0)
		{//this is a valid message in hex string
			//resize message_hex_string to its real length and converts it back to raw bytes
			tmpMsgHexStr.resize(mlen);
			auto tempMsgByteArray = HexStrToCharArray(tmpMsgHexStr);

			//the message in raw bytes should have the length of score+details+leaderboardhandle+sha256
			if (tempMsgByteArray.size() != sizeof(int32) + sizeof(LeaderboardScoreDetails_v2_t) + sizeof(SteamLeaderboard_t) + picosha2::k_digest_size);
			{
				continue;
			}

			//reconstruct LBDataUnion
			outLBDetails.data.userID = userID;
			memcpy(&(outLBDetails.data.score), tempMsgByteArray.data(), tempMsgByteArray.size());

			//re-generating sha256 resultto check if this record belongs to the user.
			auto vecUnion = std::vector<unsigned char>(sizeof(LBDataUnion) - picosha2::k_digest_size, 0);
			memcpy(vecUnion.data(), &outLBDetails, vecUnion.size());
			std::vector<unsigned char> newHashResult(picosha2::k_digest_size);
			picosha2::hash256(vecUnion, newHashResult);			
			
			//now check the hash
			bool flg = true;
			for (int i = 0; i < picosha2::k_digest_size; i++)
			{
				if (newHashResult[i] != outLBDetails.data.SHA256[i])
				{
					flg = false;
					break;
				}
			}
			if (flg)//the record belongs to current user
			{
				return true;
			}
		}
	}

	return false;
}

std::vector<int32> PrepareVerifiedLBDetails(LBDataUnion lbData, std::string signStr)
{
	//byte len: LBDataUnion - userID - record + int(signlen) + signStr
	//int len: (byteLen + intLen-1)/4
	size_t signLen = signStr.length();
	auto verifiedRecord = std::vector<int32>((sizeof(LBDataUnion) - sizeof(CSteamID) - sizeof(int32) + sizeof(size_t) + signLen + sizeof(int32) - 1) / sizeof(int32), 0);

	//prepare data
	memcpy(verifiedRecord.data(), lbData.rawBytes + sizeof(CSteamID) + sizeof(int32), sizeof(LBDataUnion) - sizeof(CSteamID) - sizeof(int32));
	memcpy(verifiedRecord.data() + sizeof(LBDataUnion) - sizeof(CSteamID) - sizeof(int32), &signLen, sizeof(size_t));
	memcpy(verifiedRecord.data() + sizeof(LBDataUnion) - sizeof(CSteamID) - sizeof(int32) + sizeof(size_t), signStr.data(), signLen);
	return verifiedRecord;
}