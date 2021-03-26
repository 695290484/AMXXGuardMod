
/* 定时器 */
public task_checkdeadbody(iEntity){
	iEntity -= TASK1
	if(!IsMonster(iEntity) || pev(iEntity, pev_deadflag) != DEAD_DYING) return

	if(pev(iEntity, pev_flags) & FL_ONGROUND){
		set_pev(iEntity, pev_movetype, MOVETYPE_NONE)
	}

	set_task(0.1, "task_checkdeadbody", iEntity + TASK1)
}


public task_find(tid){
	new ent = tid - TASK2
	
	if(!IsMonster(ent) || !IsMonsterAlive(ent)) return
	new enemy = pev(ent, pev_enemy)

	// 无敌人保持寻路状态
	if(!enemy || !pev_valid(enemy)
	|| (enemy <= gMaxPlayers && !is_user_alive(enemy))
	|| (enemy > gMaxPlayers && IsNonPlayer(enemy) && !IsNonPlayerAlive(enemy))){
		setMonsterRandomEnemy(ent)
		set_task(2.0, "task_find", tid)
		return
	}

	// 在路径偏移时暂停自动寻路
	if(g_stopPathFinding[ent] >= get_gametime()){
		set_task(1.0, "task_find", tid)
		return
	}

	reset_pathfindvars(ent)

	new Float:fO[3], Float:angles[3]
	new Float:mins1[3], Float:mins2[3]
	pev(ent, pev_origin, fO)
	pev(ent, pev_angles, angles)

	new Float:pos[3]
	pev(enemy, pev_origin, pos)

	pev(ent, pev_mins, mins1)
	pev(enemy, pev_mins, mins2)
	if(enemy <= gMaxPlayers && pev(enemy, pev_flags) & FL_DUCKING) pos[2] += 18.0

	// 添加了size对坐标的影响
	fO[2] += mins1[2] + 5.0
	pos[2] += mins2[2] + 5.0

	new len = navmesh_find(fO, pos, FASTEST_ROUTE)

	new Float:closet[3]

	if(len){
		navmesh_closetpoint(fO, 1, len, closet)

		g_way[ent][0] = fO
		g_way[ent][1] = fO // 从这个之后开始走好了
		for(new i = 1 ; i < min(len,7) ; ++ i){
			navmesh_get(i, g_way[ent][i+1])
		}
		g_pathLength[ent] = len>6?8:len+1
		g_nextPoint[ent] = 2
	}

	#if defined DEBUG
	test_drawline(ent)
	#endif
	set_task(2.0, "task_find", tid)
}

public task_attackdelay(data[]){
	new ent = data[0]
	if(!pev_valid(ent) || !IsMonster(ent) || !IsMonsterAlive(ent)) return

	new Float:damage = float(data[1])
	new distance = data[2]
	new f2f = data[3]
	new dmgType = data[4]

	new Float:o1[3], Float:o2[3]
	pev(ent, pev_origin, o1)

	for(new id=1;id<=gMaxPlayers;++id){
		if(!is_user_alive(id) || (f2f==1 && !can_see_entity_f2f(ent, id)) || (f2f==-1 && can_see_entity_f2f(ent, id))) continue
		pev(id, pev_origin, o2)
		if(length2D(o1, o2) > distance || floatabs(o1[2]-o1[2]) > distance) continue
		ExecuteHamB(Ham_TakeDamage, id, ent, ent, damage, dmgType)
	}

	for(new other=gMaxPlayers+1;other<512;++other){
		if(!IsNonPlayer(other) || !IsNonPlayerAlive(other) || (f2f==1 && !can_see_entity_f2f(ent, other)) || (f2f==-1 && can_see_entity_f2f(ent, other))) continue
		pev(other, pev_origin, o2)
		if(length2D(o1, o2) > distance || floatabs(o1[2]-o1[2]) > distance) continue
		ExecuteHamB(Ham_TakeDamage, other, ent, ent, damage, dmgType)
	}
}

public task_showDamageCount(attacker){
	attacker -= TASK4
	if(!is_user_connected(attacker))
		return

	static Float:totalDmg
	totalDmg = dmgCount[attacker]

	static text[64]
	if(totalDmg < 1.0)
		formatex(text, charsmax(text), "MISS")
	else if(extDmgCount[attacker] > 0.01)
		formatex(text, charsmax(text), "%.1f(+%.1f)", dmgCount[attacker], extDmgCount2[attacker])
	else
		formatex(text, charsmax(text), "%.1f", dmgCount[attacker])

	set_hudmessage(255, 80, 155, -1.0, 0.55, 0, 0.0, 1.3, 0.1, 0.1, HUD_DMG)
	show_hudmessage(attacker, "%s", text)

	for(new id=1;id<=get_maxplayers();++id){
		if(is_user_alive(id) || !is_user_connected(id) || pev(id, pev_iuser2) != attacker) continue
		set_hudmessage(255, 80, 155, -1.0, 0.55, 0, 0.0, 1.3, 0.1, 0.1, HUD_DMG) 
		show_hudmessage(id, "%s", text)
	}
}

public task_fixtarget(tid){
	new ent = tid - TASK5
	
	if(!IsMonster(ent) || !IsMonsterAlive(ent)) return

	new enemy = pev(ent, pev_enemy)
	if (!enemy || !pev_valid(enemy))
		return

	new Float:origin[3], Float:enemyPos[3], o[3], Float:dist, Float:dist2
	pev(ent, pev_origin, origin)
	pev(enemy, pev_origin, enemyPos)
	dist = get_distance_f(origin, enemyPos)

	for(new others = 1; others<=gMaxPlayers; ++others){
		if(enemy != others && is_user_alive(others)){
			pev(others, pev_origin, o)
			dist2 = get_distance_f(origin, o)
			if(dist>dist2 && dist2<900.0 && f2f(ent, others) && !random_num(0, 2)){
				set_pev(ent, pev_enemy, others)
				set_task(10.0, "task_fixtarget", tid)
				return
			}
		}
	}


/*
	new others = -1
	while((others = engfunc(EngFunc_FindEntityInSphere, others, origin, 240.0)) > 0){
		if(others > gMaxPlayers && IsNonPlayer(others) && !IsNonPlayerAlive(others) && f2f(ent, others)){
			pev(others, pev_origin, o)
			if(get_distance_f(origin, enemyPos)>get_distance_f(origin, o)){
				set_pev(ent, pev_enemy, others)
				break
			}
		}	
	}
*/

	set_task(5.0, "task_fixtarget", tid)
	return


}

public task_checkplayercount(tid){
	if(gIsGameStarted)
		return

	if(get_csteam_num(1, 0) + get_csteam_num(2, 0)){
		gIsGameStarted = 1
		server_cmd("endround 0")
		remove_task(tid)
		gRoundStart = ExecuteForward(g_fwMissionTrigger, g_fwDummyResult, mt_MissionOver, 0)
	}
}

public task_resetround(){
	server_cmd("endround T")
	ExecuteForward(g_fwMissionTrigger, g_fwDummyResult, mt_MissionOver, 0)
}

public task_showfirstlevelname(){
	set_hudmessage(120,54,54, 0.02, 0.62, 1, 0.0, 6.5,0.1, 0.1, HUD_GAMEMSG)
	show_hudmessage(0, "%s", gLevelName[1])

	client_cmd(0, "spk %s", gSounds[1])

	remove_task(TASK9)
	set_task(random_float(5.0, 10.0), "task_playAmbienceSound", TASK9)
}

public task_playAmbienceSound(){
	client_cmd(0, "spk %s", gSounds[0])

	remove_task(TASK9)
	set_task(random_float(72.0, 90.0), "task_playAmbienceSound", TASK9)
}

public task_spinweapon(ent)
{
	if(pev_valid(ent))
	{
		static Float:floatvector[3]
		floatvector[1] = 60.0
		set_pev(ent,pev_avelocity,floatvector)
	}
}
