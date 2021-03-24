
/* 全局 */
new g_fwDummyResult, g_fwPostKilled, g_fwPreThink, g_fwPostThink, g_fwPreTraceAttack, g_fwPostTraceAttack, g_fwPostCreate, g_fwPreTakeDamage, g_fwPostTakeDamage
new g_fwJump, g_fwMove, g_fwAttack
new g_fwRefresh, g_fwMissionTrigger, g_fwDarkLevelChange

new  g_AllocString, g_msgScoreInfo, g_msgStatusText

new cvar_bodydelay, cvar_gameseconds

#define MONSTER 112
#define NONPLAYER 122

#define PEV_MONSTER pev_iuser2
#define PEV_BOSSLEVEL pev_iuser3

#define HUD_DMG 1
#define HUD_MONSTER 2
#define HUD_SHOWINFO 3
#define HUD_GAMEMSG 4

#define TASK1 1000
#define TASK2 2000
#define TASK3 3000
#define TASK4 4000
#define TASK5 5000
#define TASK6 6000
#define TASK7 7000
#define TASK8 8000

new gMaxPlayers

new Float:g_way[512][8][3] // 0是当前位置 1是路径上最近的点 2是第一个目标路点
new g_pathLength[512] // 路径长度
new g_nextPoint[512] // 下个路点
new Float:g_checkInterval[512] // 跳跃检测间隔
new Float:g_stopPathFinding[512] // 暂停寻路
new Float:g_stopHandsChecking[512] // 暂停双手检测

new gLastFlags[512]

#define ANGLES_SMOOTH 18.5

//const m_flVelocityModifier = 108 // 点实体没有喔

//#define DEBUG
#if defined DEBUG
new laser 
#endif

// 最多30种
#define MONSTER_TYPES 30
new gMTCounter
new gMTModelIndex[MONSTER_TYPES]
new gMTModel[MONSTER_TYPES][128]
//new gMTLabel[MONSTER_TYPES][33]
new gMTWalkSeq[MONSTER_TYPES], gMTWalkFrames[MONSTER_TYPES], Float:gMTWalkFPS[MONSTER_TYPES]
new gMTRunSeq[MONSTER_TYPES], gMTRunFrames[MONSTER_TYPES], Float:gMTRunFPS[MONSTER_TYPES]
new Float:gMTWalkSpeed[MONSTER_TYPES], Float:gMTRunSpeed[MONSTER_TYPES]

#define pev_animgap pev_fuser4

new gLastHeadshot[512]
new Float:gAnimInterrupt[512]

new spr_blood_spray, spr_blood_drop

new Float:dmgCount[33] // 总伤害=原伤害+普通额外伤害+特殊额外伤害
new Float:extDmgCount[33] // 普通额外伤害
new Float:extDmgCount2[33] // 特殊额外伤害

// 随机复活点
new spawn_use_csdm
const MAX_CSDM_SPAWNS = 128
new g_spawnCount, g_spawnCount2 // 2是队伍出生点,1是csdm提供的点
new Float:g_spawns[MAX_CSDM_SPAWNS][3], Float:g_spawns2[MAX_CSDM_SPAWNS][3]

new gFwdSpawn, gMapEntCounter

/* 游戏规则变量 */

new gDoNotCreatePrincess = 0

#define MAX_LEVEL 30
new gMonsterEntCounter
new gMaxMonster = 50
new gCurLevel = 0, gMaxLevel = 0
new gLevelScore[MAX_LEVEL]
new gLevelName[MAX_LEVEL][32]
new gUserScore[33], Float:gUserRespawnCD[33], Float:gUserLastDeath[33]

new gUserTargetMonster[33], Float:gUserLastAttack[33]

new gPrincess, Float:gPrincessTime
new gIsGameStarted, gRoundStart

new Float:gPrincessCenter[3], Float:gMonsterCenter[3]

new Float:CheckScore, Float:CheckMonster, Float:checkSeconds

new Float:mTime[33], mCount[33], gMenuType[33]

new gLastCenterMsg[33], gCurrentDayTime, gCurrentDarkLevel