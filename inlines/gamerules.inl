
/* 游戏规则 */

init_gamerules(){
	setupLevel(1, "先头部队", 300)
}

fw_afterCreate(ent, Float:origin[3], Float:zOffset){
	if(!origin[0] && !origin[1] && !origin[2]){
		if(!changeMonstersPosition(ent, zOffset)){
			set_pev(ent, pev_flags, FL_KILLME)
			return 0
		}
	}
	return 1
}

setupLevel(level, levelName[], targetScore){
	formatex(gLevelName[level], charsmax(gLevelName[]), levelName)
	gLevelScore[level] = targetScore

	if(level > gMaxLevel)
		gMaxLevel = level
}

/* 每隔几秒计算一下 */
updateLevel(){
	new lastLevel = gCurLevel
	new allScore = calcAllScore()
	for(new i = 0; i < MAX_LEVEL; ++i){
		if(!gLevelScore[i])
			continue

		if(allScore < gLevelScore[i])
			break
		
		gCurLevel = i + 1
	}
	if(gCurLevel > gMaxLevel){
		set_hudmessage(120,54,54, 0.02, 0.62, 1, 0.0, 5.5, 0.2, 0.2, HUD_GAMEMSG)
		show_hudmessage(0, "任务完成,魔王的军队逃走了...")

		CheckMonster = get_gametime() + 9999.0
		server_cmd("endround CT")
		ExecuteForward(g_fwMissionTrigger, g_fwDummyResult, mt_MissionOver, 2)
	}else if(lastLevel != gCurLevel){
		ExecuteForward(g_fwMissionTrigger, g_fwDummyResult, mt_ChangeLevel, gCurLevel)
	}
}

calcAllScore(){
	new score
	for(new id=1;id<=gMaxPlayers;++id){
		if(is_user_connected(id)){
			score += gUserScore[id]
		}
	}
	return score
}

changeMonstersPosition(ent, Float:zOffset, retry=0){
	if(retry > 3)
		return 0

	new Float:origin[3]
	if(do_random_spawn(ent, get_pcvar_num(spawn_use_csdm), origin)){
		origin[2] += zOffset
		if(!is_hull_vacant(origin)){
			return 0 //changeMonstersPosition(ent, zOffset, retry++)
		}
		gMonsterEntCounter ++
		engfunc(EngFunc_SetOrigin, ent, origin)

		return 1
	}
	return 0
}

resetPrincess(hostage, Float:origin[3]){
	set_pev(hostage, pev_solid, SOLID_SLIDEBOX)
	set_pev(hostage, pev_deadflag, DEAD_NO)
	set_pev(hostage, pev_takedamage, DAMAGE_YES)
	set_pev(hostage, pev_health, 500.0)
	set_pev(hostage, pev_max_health, 500.0)
	set_pev(hostage, pev_flags, 0)
	set_pev(hostage, pev_rendermode, kRenderNormal)
	set_pev(hostage, pev_renderamt, 255.0)
	set_pev(hostage, pev_movetype, MOVETYPE_STEP)
	set_pev(hostage, pev_effects, EF_DIMLIGHT)
	set_pev(hostage, pev_targetname, "公主")
	set_pev(hostage, PEV_MONSTER, NONPLAYER)
	engfunc(EngFunc_SetOrigin, hostage, origin)

	new hb = GetMD_int(hostage, md_healthbar)
	if(hb && pev_valid(hb)){
		set_pev(hb, pev_effects, pev(hb, pev_effects) & ~EF_NODRAW)
		set_pev(hb, pev_frame, 99.0)
	}
}

hidePrincess(hostage){
	new hb = GetMD_int(hostage, md_healthbar)
	if(hb && pev_valid(hb))
		set_pev(hb, pev_effects, pev(hb, pev_effects) | EF_NODRAW)

	set_pev(hostage, pev_solid, SOLID_NOT)
	set_pev(hostage, pev_takedamage, DAMAGE_NO)
	set_pev(hostage, pev_effects, EF_NODRAW)
	engfunc(EngFunc_SetOrigin, hostage, Float:{-9000.0,-9000.0,-9000.0})
}