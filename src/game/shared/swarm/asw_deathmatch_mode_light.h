/* This header should be included in source files that need 
ASWDeathmatchMode() function only. To improve compiling perfomance. 
*/
#ifndef asw_deathmatch_mode_light_h__
#define asw_deathmatch_mode_light_h__

#ifdef CLIENT_DLL
#define CASW_Deathmatch_Mode C_ASW_Deathmatch_Mode
#endif 

class CASW_Deathmatch_Mode;
// returns an object if asw_deathmatch_mode entity is present in map
CASW_Deathmatch_Mode* ASWDeathmatchMode();
#endif // asw_deathmatch_mode_light_h__
