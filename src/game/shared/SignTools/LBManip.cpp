#include "LBManip.h"
#include "Keys.h"


std::string LBRawToHexString(CSteamID userID, SteamLeaderboard_t m_hSteamLeaderboard, int32 m_iLeaderboardScore, LeaderboardScoreDetails_v2_t m_LeaderboardScoreDetails)
{
	//Output identification string
	union LBDataUnion u;
	u.rawData.m_iLeaderboardScore = m_iLeaderboardScore;
	u.rawData.m_LeaderboardScoreDetails = m_LeaderboardScoreDetails;
	u.rawData.m_hSteamLeaderboard = m_hSteamLeaderboard;
	//stores hash of userID to protect privacy
	auto vecID = std::vector<unsigned char>(sizeof(userID),0);
	memcpy(vecID.data(), &userID, sizeof(userID));
	std::vector<unsigned char> hash_result(picosha2::k_digest_size);
	picosha2::hash256(vecID, hash_result);
	memcpy(u.rawData.userIDHash, hash_result.data(), picosha2::k_digest_size);
	
	//generating sha256 hex string. SteamID's SHA256 result is hashed into the overal result, so that we can check the owner of the record later when uploading and displaying.
	auto vecUnion = std::vector<unsigned char>(sizeof(u) - picosha2::k_digest_size, 0);
	memcpy(vecUnion.data(), &u, vecUnion.size());
	picosha2::hash256(vecUnion, hash_result);
	memcpy(u.rawData.SHA256, hash_result.data(), picosha2::k_digest_size);

	//now we convert the union to hex string and append sha256 string to it.
	return char_array_to_hex_string(u.byteData, sizeof(u));

}

