
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
		}
	}

	if(CheckMonster && fCurTime-CheckMonster >= 0.1){ 
		CheckMonster = fCurTime
		ExecuteForward(g_fwRefresh, g_fwDummyResult, gCurLevel, gMaxMonster-gMonsterEntCounter)
	}

	if(fCurTime-checkSeconds >= 1.0){
		checkSeconds = fCurTime

		//static tar, body, monstername[32],Float:hp,Float:maxhp
		static team, Float:respawn
		for(new id = 1; id<=gMaxPlayers; ++id){
			team = get_user_team(id)
/*			
			// 指向目标显示信息
			if(is_user_alive(id)){
				get_user_aiming(id,tar,body)
				if((IsMonster(tar) || IsNonPlayer(tar)) && (gUserTargetMonster[id] != tar || fCurTime - gUserLastAttack[id] > 5.0)){
					pev(tar, pev_targetname, monstername, 31)
					pev(tar,pev_health,hp)
					pev(tar,pev_max_health,maxhp)
					set_hudmessage(255,99,71, -1.0, 0.75, 0, 0.0, 1.1, 0.0, 0.0, HUD_SHOWINFO)
					show_hudmessage(id, "%s  Lv:%d HP:%.1f％", monstername, pev(tar, PEV_BOSSLEVEL), hp/maxhp*100.0)

					for(new ob = 1; ob <= gMaxPlayers; ++ob){
						if(ob != id && is_user_connected(ob) && !is_user_alive(ob) && pev(ob, pev_iuser2) == id){
							set_hudmessage(255,99,71, -1.0, 0.75, 0, 0.0, 1.1, 0.0, 0.0, HUD_SHOWINFO)
							show_hudmessage(ob, "%s  Lv:%d HP:%.1f％", monstername, pev(tar, PEV_BOSSLEVEL), hp/maxhp*100.0)
						}
					}
				}
			}
*/

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

			if(is_user_connected(id) && team>0 && team<3  && !is_user_alive(id) && (gDoNotCreatePrincess || IsNonPlayer(gPrincess))){
				respawn = gUserRespawnCD[id] - fCurTime + gUserLastDeath[id]
				if(respawn >= 0.0){
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


	}
}

public fw_PlayerKilled(victim, iAttacker, shouldgib){
	gUserScore[victim] = max(gUserScore[victim] - gCurLevel*5, 0)
	UpdateFrags(victim, gUserScore[victim], -1, 1)

	gUserLastDeath[victim] = get_gametime()
	gUserRespawnCD[victim] = floatmin(10.0, gUserRespawnCD[victim] + 3.0)
}

public client_putinserver(id){
	gUserScore[id] = 0
	gUserRespawnCD[id] = 0.0
	gUserLastDeath[id] = 0.0
	gUserTargetMonster[id] = 0
	gUserLastAttack[id] = 0.0
	gMenuType[id] = 0

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
}

public plugin_end(){
	remove_task(TASK6)
	remove_task(TASK7)
}

public fw_FMChangeLevel(){
	remove_task(TASK6)
	remove_task(TASK7)
}

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
					client_color(0, "/ctr公主/y出现在了地图上,/g找到/y并/g保护/y她!")
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
	for(new id=1;id<=gMaxPlayers;++id)
		client_putinserver(id)

	remove_all_npc()
}

public msgTeamInfo(iMsgID, iDest, iReceiver){
	// 最后一个玩家换队伍至观察者
	if(get_csteam_num(1, 0) + get_csteam_num(2, 0) == 0){
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
		ExecuteForward(g_fwMissionTrigger, g_fwDummyResult, mt_MissionOver, 1)
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
	formatex(menu, charsmax(menu), "\w 武器菜单")
	menu_additem(menuid, menu)

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
	}
}
