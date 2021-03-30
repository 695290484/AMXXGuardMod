
/* 规则相关的事件 */

public fw_StartFrame_Post(){
	if(!gIsGameStarted)
		return

	static Float:fCurTime
	global_get(glb_time, fCurTime)
	if(gRoundStart && fCurTime-CheckScore >= 3.0){
		CheckScore = fCurTime
		updateLevel()

		if(fCurTime - gPrincessTime < 4.0){
			set_hudmessage(120,54,54, 0.02, 0.62, 1, 0.0, 6.5,0.1, 0.1, HUD_GAMEMSG)
			show_hudmessage(0, "警告: 20秒后魔王的军队将抵达战场!")
			CheckMonster = fCurTime + 20.0

			remove_task(TASK8)
			remove_task(TASK9)
			set_task(9.5, "task_showfirstlevelname", TASK8)
		}
	}

	if(CheckMonster && fCurTime-CheckMonster >= 0.1){
		// 随着亮度改变 产生的速度变化
		switch(gCurrentDarkLevel){
			case 0: CheckMonster = fCurTime + random_float(2.0, 3.0)
			case 1: CheckMonster = fCurTime + random_float(1.5, 2.0)
			case 2: CheckMonster = fCurTime + random_float(1.0, 1.5)
			case 3: CheckMonster = fCurTime + random_float(0.5, 1.0)
			case 4: CheckMonster = fCurTime
		}
		ExecuteForward(g_fwRefresh, g_fwDummyResult, gCurLevel, gMaxMonster-gMonsterEntCounter)
	}

	if(fCurTime-checkSeconds >= 1.0){
		checkSeconds = fCurTime

		static tar, body, monstername[32],Float:hp,Float:maxhp
		static team, Float:respawn
		for(new id = 1; id<=gMaxPlayers; ++id){
			if(!is_user_connected(id)) continue
			team = get_pdata_int(id, 114, 5)
			
			// 指向目标显示信息
			if(is_user_alive(id)){
				get_user_aiming(id,tar,body)
				if((IsMonster(tar) || IsNonPlayer(tar)) && (gUserTargetMonster[id] != tar || fCurTime - gUserLastAttack[id] > 5.0)){
					pev(tar, pev_targetname, monstername, 31)
					pev(tar,pev_health,hp)
					pev(tar,pev_max_health,maxhp)

					client_center(id, "%s  Lv%d HP:%.1f％", monstername, pev(tar, PEV_BOSSLEVEL), hp/maxhp*100.0)
					gLastCenterMsg[id] = 1
					/*
					set_hudmessage(255,99,71, -1.0, 0.75, 0, 0.0, 1.1, 0.0, 0.0, HUD_SHOWINFO)
					show_hudmessage(id, "%s  Lv:%d HP:%.1f％", monstername, pev(tar, PEV_BOSSLEVEL), hp/maxhp*100.0)

					for(new ob = 1; ob <= gMaxPlayers; ++ob){
						if(ob != id && is_user_connected(ob) && !is_user_alive(ob) && pev(ob, pev_iuser2) == id){
							set_hudmessage(255,99,71, -1.0, 0.75, 0, 0.0, 1.1, 0.0, 0.0, HUD_SHOWINFO)
							show_hudmessage(ob, "%s  Lv:%d HP:%.1f％", monstername, pev(tar, PEV_BOSSLEVEL), hp/maxhp*100.0)
						}
					}
					*/
				}else if(gLastCenterMsg[id]){
					gLastCenterMsg[id] = 0
					client_center(id, "")
				}
			}


			if(is_user_alive(id)){
				set_hudmessage(0, 200, 200, 0.01, 0.94, 0, 0.0, 2.0, 0.0, 0.0, HUD_SHOWINFO)
				show_hudmessage(id, "生命值:%d/%d", pev(id,pev_health), pev(id,pev_max_health))

				for(new ob = 1; ob <= gMaxPlayers; ++ob){
					if(ob != id && is_user_connected(ob) && !is_user_alive(ob) && pev(ob, pev_iuser2) == id){
						set_hudmessage(0, 200, 200, 0.01, 0.94, 0, 0.0, 2.0, 0.0, 0.0, HUD_SHOWINFO)
						show_hudmessage(ob, "生命值:%d/%d", pev(id,pev_health), pev(id,pev_max_health))
					}
				}

			}

			if(team>0 && team<3  && !is_user_alive(id) && (gDoNotCreatePrincess || IsNonPlayer(gPrincess))){
				respawn = gUserRespawnCD[id] - fCurTime + gUserLastDeath[id] + 1.0
				if(respawn >= 1.0){
					client_print(id, print_center, "即将复活: %1.f", respawn)
				}else{
					client_print(id, print_center, " ")
					ExecuteHamB(Ham_CS_RoundRespawn, id)

					if(!gDoNotCreatePrincess){
						// 设置在公主附近的路点复活
						new Float:princessOri[3], Float:respawnOri[3]
						pev(gPrincess, pev_origin, princessOri)
						navmesh_getRandomAreaPos(princessOri, 0.0, 800.0, respawnOri)
						set_pev(id, pev_origin, respawnOri)
						ClientCommand_UnStick (id)
					}
				}
			}
		}

		// b c d e f g h i j k l m
		// 12个亮度拆分成5段
		gCurrentDayTime += get_pcvar_num(cvar_gameseconds)
		new dark = gCurrentDayTime / 14400 // 3600 * 4
		if(dark >= 5){
			gCurrentDayTime = 0
		}
		if(dark != gCurrentDarkLevel){
			switch(dark){
				case 0: engfunc(EngFunc_LightStyle, 0, "m")
				case 1: engfunc(EngFunc_LightStyle, 0, "h")
				case 2: engfunc(EngFunc_LightStyle, 0, "f")
				case 3: engfunc(EngFunc_LightStyle, 0, "d")
				case 4: engfunc(EngFunc_LightStyle, 0, "b")
			}

			gCurrentDarkLevel = dark
			ExecuteForward(g_fwDarkLevelChange, g_fwDummyResult, gCurrentDarkLevel)
		}

	}
}

public fw_PlayerKilled(victim, iAttacker, shouldgib){
	// No more points reduction 不减分就不会掉难度 
	//gUserScore[victim] = max(gUserScore[victim] - gCurLevel*5, 0)
	//UpdateFrags(victim, gUserScore[victim], -1, 1)

	gUserLastDeath[victim] = get_gametime()
	gUserRespawnCD[victim] = floatmin(10.0, gUserRespawnCD[victim] + 3.0)

	CancleBuilding(victim)
}

public client_putinserver(id){
	//gUserScore[id] = 0
	gUserRespawnCD[id] = 0.0
	gUserLastDeath[id] = 0.0
	gUserTargetMonster[id] = 0
	gUserLastAttack[id] = 0.0
	gMenuType[id] = 0
	gIsBuilding[id] = 0
	gRotation[id] = 0

	remove_task(TASK6)
	set_task(3.0, "task_checkplayercount", TASK6, _, _, "b")
}

public client_disconnect(id){
	// 最后一个玩家退出 结束回合
	if(get_csteam_num(1, 0) + get_csteam_num(2, 0) == 1){
		remove_task(TASK7)
		set_task(3.0, "task_resetround", TASK7)
		gIsGameStarted = 0
	}
	//gUserScore[id] = 0

	CancleBuilding(id)
}

public plugin_end(){
	remove_task(TASK6)
	remove_task(TASK7)
}

public fw_FMChangeLevel(){
	remove_task(TASK6)
	remove_task(TASK7)
}

/* 只有平局才会触发(mp修改了设定) */
public EventRoundEnd(){
	gRoundStart = 0
}

public EventStartRound(){
	if(get_csteam_num(1, 0) + get_csteam_num(2, 0) == 0){
		return
	}

	CheckMonster = get_gametime() + 20.0

	// remove_all_enemy() // 看上去不需要它了
	gIsGameStarted = 1
	gRoundStart = 1
	if(!gDoNotCreatePrincess){
		new Float:origin[3]
		while(gRoundStart){
			if( (!gPrincessCenter[0]&&!gPrincessCenter[1]&&!gPrincessCenter[2]&&navmesh_randomPosition(origin))
			|| ((gPrincessCenter[0]||gPrincessCenter[1]||gPrincessCenter[2])&& navmesh_getRandomAreaPos(gPrincessCenter, 0.0, 1200.0, origin)) ){
				origin[2] += 41.0
				if(is_hull_vacant(origin)){
					resetPrincess(gPrincess, origin)
					client_color(0, "/y任务:/g找到/ctr公主/y,并/g保护/y她!")
					gPrincessTime = get_gametime()
					ExecuteForward(g_fwMissionTrigger, g_fwDummyResult, mt_PrincessShowUp, gPrincess)
					break
				}
			}
			else{
				log_amx("公主初始化失败,可能因为没有路点文件")
				remove_task(TASK6)
				gIsGameStarted = 0
				break
			}
		}
	}
}

public EventHLTV(){
	for(new id=1;id<=gMaxPlayers;++id){
		client_putinserver(id)
		gUserScore[id] = 0	// 得分只在换回合时重置
	}

	remove_all_npc()

	engfunc(EngFunc_LightStyle, 0, "m")
	gCurrentDayTime = 0
	gCurrentDarkLevel = 0

	gCurLevel = 1
}

public msgTeamInfo(iMsgID, iDest, iReceiver){
	// 最后一个玩家换队伍至观察者
	if(gIsGameStarted && get_csteam_num(1, 0) + get_csteam_num(2, 0) == 0){
		remove_task(TASK7)
		set_task(3.0, "task_resetround", TASK7)
		gIsGameStarted = 0
	}
}


public HAM_HostageKilled_post(iEntity, inflictor, attacker, Float:damage, damagetype){
	if(!pev_valid(iEntity)) return HAM_IGNORED

	if(pev(iEntity, pev_deadflag) != DEAD_NO){
		set_hudmessage(120,54,54, 0.02, 0.62, 1, 0.0, 5.5, 0.2, 0.2, HUD_GAMEMSG)
		show_hudmessage(0, "任务失败: 公主已经被杀害...")

		server_cmd("endround T")
		gRoundStart = 0
		ExecuteForward(g_fwMissionTrigger, g_fwDummyResult, mt_MissionOver, 1)
	}else{
		new hb = GetMD_int(iEntity, md_healthbar)
		if(pev_valid(hb)){
			static Float:hp,Float:maxhp
			pev(iEntity, pev_health, hp)
			pev(iEntity, pev_max_health, maxhp)
			if(maxhp < hp) set_pev(hb, pev_frame, 99.0)
			else set_pev(hb, pev_frame, 0.0 + (((hp - 1.0) * 100.0) / maxhp))
		}
	}

	return HAM_IGNORED
}

public clcmd_changeteam(id)
{
	if(!is_user_connected(id))
		return PLUGIN_CONTINUE;

	new team = get_pdata_int(id, 114, 5)
	
	if (team == 3 || !team)
		return PLUGIN_CONTINUE;

	new Float:gtime = get_gametime()
	if(gtime - mTime[id] <0.4){
		if(mCount[id] <3){
			mCount[id] ++
			if(mCount[id] >=3){
				mCount[id] = 0
				if(gMenuType[id]){
					gMenuType[id] = 0
					client_print(id, print_chat, "* 已切换至玩家菜单")
				}else{
					gMenuType[id] = 1
					client_print(id, print_chat, "* 已切换至队伍菜单")
				}
			}
		}
	}else{
		mCount[id] = 0
	}
	mTime[id] = gtime

	if(gMenuType[id])
		return PLUGIN_CONTINUE;
	
	// M菜单
	rpg_mainmenu(id)
	return PLUGIN_HANDLED;
}

rpg_mainmenu(id){
	set_pdata_int(id, 205, 0)

	new menuid, menu[256]
	formatex(menu, charsmax(menu), "\r#. \dRPG-Guard模式^n\dQ群:\r1080724568")
	menuid = menu_create(menu, "mainmenuhandle")
	menu_additem(menuid, "\w 武器")
	menu_additem(menuid, "\w 建造")

	menu_setprop(menuid, MPROP_EXITNAME, "退出")
	menu_setprop(menuid, MPROP_BACKNAME, "后退")
	menu_setprop(menuid, MPROP_NEXTNAME, "下一页")
	menu_display(id, menuid)
}


public mainmenuhandle(id, menuid, key)
{
	menu_destroy(menuid)
	if(key<0)
		return

	switch(key){
		case 0: client_cmd(id, "say /wp")
		case 1: buildMenu(id)
	}
}

public fw_AddToFullPack_post(es, e, user, host, host_flags, player, p_set){
	if(!pev_valid(user))
		return FMRES_IGNORED

	static monster
	monster = pev(user, PEV_MONSTER)
	if(monster != MONSTER && monster != NONPLAYER)
		return FMRES_IGNORED

	static hb
	hb = GetMD_int(user, md_healthbar)
	if(!hb || !pev_valid(hb))
		return FMRES_IGNORED

	if(pev(hb, pev_effects) & EF_NODRAW)
		return FMRES_IGNORED

	static Float:Origin[3]
	pev(user, pev_origin, Origin)				
	Origin[2] += pev(hb, pev_iuser4)	
	engfunc(EngFunc_SetOrigin, hb, Origin)

	return FMRES_HANDLED
}

public fw_PlayerTraceAttack(victim, attacker, Float:damage, Float:direction[3], tracehandle, damagetype){
	if(attacker>0 && attacker<gMaxPlayers && attacker != victim)
		return HAM_SUPERCEDE

	return HAM_IGNORED
}

public fw_PlayerTakeDamage(victim, inflictor, attacker, Float:damage, damagetype){
	if(attacker>0 && attacker<gMaxPlayers && attacker != victim)
		return HAM_SUPERCEDE

	return HAM_IGNORED
}	

public fw_SetModel(entity, const model[])
{
	if (strlen(model) < 8)
		return;

	static classname[10]
	pev(entity, pev_classname, classname, charsmax(classname))
		
	if (equal(classname, "weaponbox")){
		set_pev(entity, pev_nextthink, get_gametime() + get_pcvar_float(cvar_removeweapon))
	}
}

public touch_weapon(weaponbox,worldspawn)
{
	if(pev_valid(weaponbox))
	{
		float_weapon(weaponbox)
		set_task(0.1, "task_spinweapon", weaponbox)
	}
}

public fw_CmdStart(iPlayer, uc_handle, seed)
{
	if(!is_user_alive(iPlayer)) return

	moveBuilding(iPlayer, uc_handle)
}

public fw_GameDesc()
{
	forward_return(FMV_STRING, "RPG-Guard")
	return FMRES_SUPERCEDE
}
