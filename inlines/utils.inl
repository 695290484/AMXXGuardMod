
/* 工具 */
#define IsMonster(%1) (%1>0&&pev_valid(%1) && pev(%1, PEV_MONSTER) == MONSTER)
#define IsMonsterAlive(%1) pev(%1, pev_deadflag) == DEAD_NO
#define IsPlayerAlive(%1) (%1<=gMaxPlayers && is_user_alive(%1))
#define IsNonPlayer(%1) (%1>0&&pev_valid(%1) && pev(%1, PEV_MONSTER) == NONPLAYER)
#define IsNonPlayerAlive(%1) pev(%1, pev_deadflag) == DEAD_NO

#define GetPlayerHullSize(%1)  ( ( pev ( %1, pev_flags ) & FL_DUCKING ) ? HULL_HEAD : HULL_HUMAN )

