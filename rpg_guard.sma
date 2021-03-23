/*
	注：所有源文件请务必用UTF-8无bom编码保存
 */

#include <amxmodx>
#include <amxmisc>
#include <fakemeta>
#include <hamsandwich>
#include <engine>
#include <xs>
#include "navmesh.inc"

#define EXCLUDE_SELF
#include "rpg_guard.inc"

#include "inlines/globals.inl"
#include "inlines/natives.inl"
#include "inlines/utils.inl"
#include "inlines/tasks.inl"
#include "inlines/stocks.inl"
#include "inlines/pathfinding.inl"
#include "inlines/gamerules.inl"
#include "inlines/events.inl"

// 尸体消失时间
#define BODY_DELAY 5.0

public plugin_init()
{
	register_plugin("RPG-CS Guard Mod","1.0","zhiJiaN")

	register_forward(FM_StartFrame, "fw_StartFrame_Post", 1)
	RegisterHam(Ham_Killed, "player", "fw_PlayerKilled")
	RegisterHam(Ham_TraceAttack, "player", "fw_PlayerTraceAttack")
	RegisterHam(Ham_TakeDamage, "player", "fw_PlayerTakeDamage")

	// 和原版不同
	g_fwPostKilled = CreateMultiForward("rpg_fw_npckilled_post", ET_IGNORE, FP_CELL, FP_CELL, FP_CELL)
	g_fwPreTakeDamage = CreateMultiForward("rpg_fw_npctakedamage_pre", ET_CONTINUE, FP_CELL, FP_CELL, FP_FLOAT, FP_CELL, FP_CELL)
	g_fwPostTakeDamage = CreateMultiForward("rpg_fw_npctakedamage_post", ET_IGNORE, FP_CELL, FP_CELL, FP_FLOAT, FP_CELL, FP_CELL)
	
	g_fwPreThink = CreateMultiForward("rpg_fw_npcthink_pre", ET_CONTINUE, FP_CELL)
	g_fwPostThink  = CreateMultiForward("rpg_fw_npcthink_post", ET_CONTINUE, FP_CELL)
	g_fwPreTraceAttack = CreateMultiForward("rpg_fw_npctraceattack_pre", ET_CONTINUE, FP_CELL, FP_CELL, FP_FLOAT, FP_CELL, FP_CELL)
	g_fwPostTraceAttack = CreateMultiForward("rpg_fw_npctraceattack_post", ET_IGNORE, FP_CELL, FP_CELL, FP_FLOAT, FP_CELL, FP_CELL)
	g_fwPostCreate = CreateMultiForward("rpg_fw_npccreate_post", ET_IGNORE, FP_CELL)
	
	g_fwJump = CreateMultiForward("rpg_fw_npc_jump", ET_IGNORE, FP_CELL)
	g_fwMove = CreateMultiForward("rpg_fw_npc_move", ET_IGNORE, FP_CELL)
	g_fwAttack =  CreateMultiForward("rpg_fw_npc_attack", ET_CONTINUE, FP_CELL, FP_CELL)

	g_fwRefresh =  CreateMultiForward("rpg_fw_npc_refresh", ET_CONTINUE, FP_CELL, FP_CELL)
	g_fwMissionTrigger =  CreateMultiForward("rpg_fw_mission_tigger", ET_IGNORE, FP_CELL, FP_CELL)

	g_AllocString = engfunc(EngFunc_AllocString, "info_target")
	g_msgScoreInfo = get_user_msgid("ScoreInfo")
	g_msgStatusText = get_user_msgid("StatusText")

	register_message(get_user_msgid("TeamInfo"), "msgTeamInfo")

	register_forward(FM_Touch, "FM_NpcTouch")
	unregister_forward(FM_Spawn, gFwdSpawn)

	RegisterHam(Ham_Killed, "info_target", "HAM_NpcKilled")
	RegisterHam(Ham_Think, "info_target", "HAM_NpcThink")
	RegisterHam(Ham_TraceAttack, "info_target", "HAM_NpcTraceAttack")
	RegisterHam(Ham_TraceAttack, "info_target", "HAM_NpcTraceAttack_post", 1)
	RegisterHam(Ham_TakeDamage, "info_target", "HAM_NpcTakeDamage")
	RegisterHam(Ham_TakeDamage, "info_target", "HAM_NpcTakeDamage_post", 1)

	gMaxPlayers = get_maxplayers()

	// 是否用csdm提供的复活点
	// 0: 用地图出生点 1: 地图出生点+附近nav路点 2: 用csdm复活点 3:csdm+附近nav路点
	spawn_use_csdm = register_cvar("rg_spawn_use_csdm", "1")
	load_spawns()

	server_cmd("mp_round_infinite 1;mp_maxmoney 999999999;mp_respawn_immunitytime 3")
	server_cmd("mp_infinite_ammo 2;mp_give_player_c4 0;mp_buy_anywhere 0")
	server_cmd("mp_autoteambalance 0")

	// 初始化游戏规则
	init_gamerules()

	if(!task_exists(TASK6))
		set_task(3.0, "task_checkplayercount", TASK6, _, _, "b")

	register_logevent("EventRoundEnd", 2, "1=Round_End")
	register_logevent("EventStartRound", 2, "0=World triggered", "1=Round_Start")
	register_event("HLTV", "EventHLTV", "a", "1=0", "2=0")

	register_forward(FM_ChangeLevel, "fw_FMChangeLevel")
	register_forward(FM_AddToFullPack, "fw_AddToFullPack_post", 1)
	RegisterHam(Ham_TakeDamage, "hostage_entity", "HAM_HostageKilled_post", 1)

	server_cmd("endround 0")

	register_clcmd("chooseteam", "clcmd_changeteam")
	register_clcmd ("unstuck" , "ClientCommand_UnStick")
	register_clcmd ("say unstuck" , "ClientCommand_UnStick")
	register_clcmd ("say /unstuck" , "ClientCommand_UnStick")
}

public plugin_precache(){
	#if defined DEBUG
	laser = precache_model("sprites/laserbeam.spr")
	#endif
	spr_blood_spray = engfunc(EngFunc_PrecacheModel, "sprites/bloodspray.spr")
	spr_blood_drop = engfunc(EngFunc_PrecacheModel, "sprites/blood.spr")
	gFwdSpawn = register_forward(FM_Spawn, "fw_Spawn")

	engfunc(EngFunc_PrecacheModel, "sprites/rpg/healthbar.spr")
	if(!gDoNotCreatePrincess){
		gPrincess = engfunc(EngFunc_CreateNamedEntity, engfunc(EngFunc_AllocString, "hostage_entity"))
		if(pev_valid(gPrincess)){
			DispatchSpawn(gPrincess)
			SetMD_int(gPrincess, md_healthbar, 1)
			CheckHealthBar(gPrincess)
			new hb = GetMD_int(gPrincess, md_healthbar)
			if(hb && pev_valid(hb))
				set_pev(hb, pev_effects, pev(hb, pev_effects) | EF_NODRAW)
		}
	}
}

public plugin_natives(){
	register_native("rpg_create_g", "_native_rpg_create")
	register_native("rpg_precache_g", "_native_rpg_precache")
	register_native("rpg_animation_g", "_native_rpg_animation")
	register_native("rpg_set_animinterrupt_g", "_native_rpg_set_animinterrupt")
	register_native("rpg_delaydmg_g", "_native_rpg_delaydmg")
	register_native("rpg_set_user_normaldamage", "_native_set_user_normaldamage")
	register_native("rpg_get_user_normaldamage", "_native_get_user_normaldamage", 1)
	register_native("rpg_set_user_extradamage", "_native_set_user_extradamage")
	register_native("rpg_get_user_extradamage", "_native_get_user_extradamage", 1)
	register_native("rpg_get_princess", "_native_get_princess", 1)

	register_native("rpg_set_princessArea", "_native_set_princessArea")
	register_native("rpg_set_monsterArea", "_native_set_monsterArea")
}

// 创建
create_monster(Float:origin[3], Float:angles[3], iMonsterlevel, szClass[], szName[], Float:fHealth,  Float:fMinsize[3], Float:fMaxsize[3], iModelID){
	gMonsterEntCounter = 0
	for(new ent = 1;ent < 512; ++ent){
		if(!pev_valid(ent) || pev(ent, PEV_MONSTER)  != 112)
			continue

		gMonsterEntCounter ++
		if(gMonsterEntCounter + gMapEntCounter > 511){
			gMaxMonster = 511 -gMapEntCounter
			break
		}
	}
	if(gMonsterEntCounter > gMaxMonster)
		return 0

	new iEntity = engfunc(EngFunc_CreateNamedEntity, g_AllocString)
	if(!pev_valid(iEntity))
		return 0

	if (iEntity>511)
	{
		set_pev(iEntity, pev_flags, FL_KILLME)
		return 0
	}
	
	set_pev(iEntity, pev_classname, szClass)
	set_pev(iEntity, pev_targetname, szName)
	set_pev(iEntity, pev_solid, SOLID_BBOX)
	set_pev(iEntity, pev_movetype, MOVETYPE_PUSHSTEP)
	set_pev(iEntity, pev_takedamage, DAMAGE_YES)
	set_pev(iEntity, pev_deadflag, DEAD_NO)
	set_pev(iEntity, pev_max_health, fHealth)
	set_pev(iEntity, pev_health, fHealth)
	set_pev(iEntity, pev_gravity, 1.0)
	set_pev(iEntity, pev_gamestate, 1)
	set_pev(iEntity, pev_angles, angles)
	set_pev(iEntity, PEV_BOSSLEVEL, iMonsterlevel)
	set_pev(iEntity, PEV_MONSTER, MONSTER)
	set_pev(iEntity, pev_flags, pev(iEntity, pev_flags) | FL_MONSTER)
	set_pev(iEntity, pev_nextthink, get_gametime()+2.0)
	set_pev(iEntity, pev_sequence, 0)
	set_pev(iEntity, pev_maxspeed, 150.0)
	set_pev(iEntity, pev_modelindex, iModelID)
	set_pev(iEntity, pev_controller_0, 125)
	set_pev(iEntity, pev_controller_1, 125)
	set_pev(iEntity, pev_controller_2, 125)
	set_pev(iEntity, pev_controller_3, 125)
	engfunc(EngFunc_SetSize, iEntity, fMinsize, fMaxsize)
	set_pev(iEntity, pev_velocity, Float:{0.0,0.0,0.1})
	remove_task(iEntity + TASK2)
	set_task(1.0, "task_find", iEntity + TASK2)
	set_task(5.0, "task_fixtarget", iEntity + TASK5)

	set_pdata_float(iEntity, m_flVelocityModifier, 1.0)
	
	if(origin[0] || origin[1] || origin[2]){
		origin[2] += (floatabs(fMinsize[2]) + 5.0)
		engfunc(EngFunc_SetOrigin, iEntity, origin)
		gMonsterEntCounter++
		setMonsterRandomEnemy(iEntity)
		ExecuteForward(g_fwPostCreate, g_fwDummyResult, iEntity)
		CheckHealthBar(iEntity)
		return  iEntity
	}else{
		if(fw_afterCreate(iEntity, origin, (floatabs(fMinsize[2]) + 5.0))){
			setMonsterRandomEnemy(iEntity)
			ExecuteForward(g_fwPostCreate, g_fwDummyResult, iEntity)
			CheckHealthBar(iEntity)
			return iEntity
		}
	}

	return 0
}

// 死亡
public HAM_NpcKilled(iEntity, iAttacker, gib){
	if(!IsMonster(iEntity))
		return HAM_IGNORED

	set_pev(iEntity, pev_deadflag, DEAD_DYING)
	set_pev(iEntity, pev_takedamage, DAMAGE_NO)
	set_pev(iEntity, pev_solid, SOLID_NOT)
	set_pev(iEntity, pev_nextthink, get_gametime()+ 0.02)
	task_checkdeadbody(iEntity + TASK1)

	remove_task(iEntity + TASK2)
	remove_task(iEntity + TASK5)

	if(is_user_connected(iAttacker)){
		new lv = pev(iEntity, PEV_BOSSLEVEL)
		if(lv){
			gUserScore[iAttacker] += lv
			UpdateFrags(iAttacker, gUserScore[iAttacker], -1, 1)
		}
	}

	new hb = GetMD_int(iEntity, md_healthbar)
	if(pev_valid(hb)){
		set_pev(hb, pev_flags, FL_KILLME)
		SetMD_int(iEntity, md_healthbar, 0)
	}

	ExecuteForward(g_fwPostKilled, g_fwDummyResult, iEntity, iAttacker, gLastHeadshot[iEntity])
	return HAM_SUPERCEDE
}

// 思考 (die - > pre(可断) -> 寻路 -> post(可断) -> nextthink + 0.07)
public HAM_NpcThink(iEntity){
	if(!IsMonster(iEntity))
		return HAM_IGNORED

	static Float:fCurTime, flag
	global_get(glb_time, fCurTime)
	flag = pev(iEntity, pev_deadflag)

	if(flag == DEAD_DYING){
		set_pev(iEntity, pev_deadflag, DEAD_DEAD)
		set_pev(iEntity, pev_nextthink, fCurTime + BODY_DELAY)
		return HAM_IGNORED
	}

	if(flag == DEAD_DEAD){
		gMonsterEntCounter --
		engfunc(EngFunc_RemoveEntity, iEntity)
		return HAM_IGNORED
	}

	gLastFlags[iEntity] = pev(iEntity, pev_flags)

	ExecuteForward(g_fwPreThink, g_fwDummyResult, iEntity)
	if(g_fwDummyResult == 1) return HAM_SUPERCEDE

	static enemy, isPlayer, isNonPlayer
	enemy = pev(iEntity, pev_enemy)
	isPlayer = IsPlayerAlive(enemy)
	isNonPlayer = enemy > gMaxPlayers && pev_valid(enemy) && pev(enemy, pev_deadflag) == DEAD_NO
	// 目标是活着的玩家, 玩家外的实体目标
	if(isPlayer || isNonPlayer){
		static targetname[16]
		pev(enemy, pev_targetname, targetname, charsmax(targetname))
		if(!strcmp(targetname, "公主") && pev(enemy, pev_effects) & EF_NODRAW){
			set_pev(iEntity, pev_nextthink, fCurTime+ 0.06)
			return HAM_IGNORED
		}

		if(walk_in_path(iEntity))
			return HAM_IGNORED
	}

	ExecuteForward(g_fwPostThink, g_fwDummyResult, iEntity)
	if(g_fwDummyResult == 1) return HAM_IGNORED
	set_pev(iEntity, pev_nextthink, fCurTime+ 0.06)
	return HAM_IGNORED
}

// 攻击(不在这里修改伤害,除非和 攻击相关而且takedamage不好处理)
public HAM_NpcTraceAttack(iEntity, attacker, Float:damage, Float:direction[3], tracehandle, damagetype){
	if(!IsMonster(iEntity) || !IsMonsterAlive(iEntity))
		return HAM_IGNORED

	if(pev(iEntity, pev_takedamage) == DAMAGE_NO)
		return HAM_SUPERCEDE

	new HitGroup = get_tr2(tracehandle, TR_iHitgroup)
	gLastHeadshot[iEntity] = 0
	new Float:Ndamage = damage
	if(HitGroup == HIT_HEAD){
		gLastHeadshot[iEntity] = 1
		Ndamage *= random_float(1.5, 2.2)
	}
	else if(HIT_CHEST <= HitGroup <= HIT_RIGHTARM) Ndamage *= random_float(0.8, 1.0)
	else if(HitGroup != HIT_GENERIC) Ndamage *= random_float(0.6, 0.8)
	SetHamParamFloat(3, Ndamage)

	ExecuteForward(g_fwPreTraceAttack, g_fwDummyResult, iEntity, attacker, Ndamage, damagetype, HitGroup)
	if(g_fwDummyResult == 1)
		return HAM_SUPERCEDE

	if(!is_user_connected(attacker))
		return HAM_IGNORED

	new Float:origin_end[3]
	get_tr2(tracehandle, TR_vecEndPos, origin_end)
	SpawnBlood(origin_end, 247, floatround(Ndamage))

	return HAM_IGNORED
}

public HAM_NpcTraceAttack_post(iEntity, attacker, Float:damage, Float:direction[3], tracehandle, damagetype){
	if(!IsMonster(iEntity) || !IsMonsterAlive(iEntity))
		return HAM_IGNORED

	new HitGroup = get_tr2(tracehandle, TR_iHitgroup)
	ExecuteForward(g_fwPostTraceAttack, g_fwDummyResult, iEntity, attacker, damage, damagetype, HitGroup)
	return HAM_IGNORED
}

// 伤害(只负责修改伤害)
public HAM_NpcTakeDamage(iEntity, inflictor, attacker, Float:damage, damagetype){
	if(!is_user_connected(attacker))
		return HAM_IGNORED

	if(!IsMonster(iEntity) || !IsMonsterAlive(iEntity))
		return HAM_IGNORED

	if(!task_exists(attacker + TASK4)){
		dmgCount[attacker] = 0.0
		extDmgCount[attacker] = 0.0
		extDmgCount2[attacker] = 0.0
		set_task(0.1, "task_showDamageCount", attacker + TASK4)
	}

	ExecuteForward(g_fwPreTakeDamage, g_fwDummyResult, iEntity, attacker, damage, damagetype, gLastHeadshot[iEntity])
	if(g_fwDummyResult == 1)
		return HAM_SUPERCEDE

	static Float:totalDmg
	totalDmg = damage+extDmgCount[attacker]+extDmgCount2[attacker]
	if(totalDmg < 1.0) totalDmg = 1.0
	SetHamParamFloat(4, totalDmg)

	return HAM_IGNORED
}


public HAM_NpcTakeDamage_post(iEntity, inflictor, attacker, Float:damage, damagetype){
	if(!IsMonster(iEntity))
		return HAM_IGNORED

	static Float:gtime
	gtime = get_gametime()

	gUserTargetMonster[attacker] = iEntity
	gUserLastAttack[attacker] = gtime
	showMonstersHP(attacker, iEntity) // TODO 不清楚伤害是否已经造成 ai死后要立刻移除hud

	for(new id=1;id<=gMaxPlayers;++id){
		if(id == attacker || !gUserTargetMonster[id]) continue
		if(gUserTargetMonster[id] == iEntity){
			if(gtime - gUserLastAttack[id] < 5.0){
				showMonstersHP(id, iEntity)
			}else{
				gUserLastAttack[id] = 0.0
				gUserTargetMonster[id] = 0
			}
		}
	}

	dmgCount[attacker] += damage

	ExecuteForward(g_fwPostTakeDamage, g_fwDummyResult, iEntity, attacker, damage, damagetype, gLastHeadshot[iEntity])

	return HAM_IGNORED
}

public FM_NpcTouch(ent, id){
	if(IsMonster(ent)){
		if(IsMonster(id)){
			static Float:velocity2[3]
			static Float:vel[3]
			pev(ent, pev_velocity, vel)
			get_speed_vector_to_entity(id, ent, 110.0 , velocity2)
			xs_vec_add(velocity2, vel, velocity2)
			velocity2[2] = vel[2]
			set_pev(ent, pev_velocity, velocity2)
		}else if(IsNonPlayer(id)){
			set_pev(ent, pev_enemy, id)
		}
	}

	return FMRES_IGNORED
}


public fw_Spawn(iEntity)
{
	if(!pev_valid(iEntity)) 
		return 0
	
	gMapEntCounter ++
	
	return 0
}
