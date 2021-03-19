
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

		static tar, body, monstername[32],Float:hp,Float:maxhp
		static team, Float:respawn
		for(new id = 1; id<=gMaxPlayers; ++id){
			team = get_user_team(id)
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

			if(is_user_connected(id) && team>0 && team<3  && !is_user_alive(id) && IsNonPlayer(gPrincess)){
				respawn = gUserRespawnCD[id] - fCurTime + gUserLastDeath[id]
				if(respawn >= 0.0){
					client_print(id, print_center, "即将复活: %1.f", respawn)
				}else{
					client_print(id, print_center, " ")
					ExecuteHamB(Ham_CS_RoundRespawn, id)

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

public fw_PlayerKilled(victim, iAttacker, shouldgib){
	gUserScore[iAttacker] = max(gUserScore[iAttacker] - gCurLevel*5, 0)
	UpdateFrags(iAttacker, gUserScore[iAttacker], -1, 1)

	gUserLastDeath[iAttacker] = get_gametime()
	gUserRespawnCD[iAttacker] = floatmin(10.0, gUserRespawnCD[iAttacker] + 3.0)
}

public client_putinserver(id){
	gUserScore[id] = 0
	gUserRespawnCD[id] = 0.0
	gUserLastDeath[id] = 0.0
	gUserTargetMonster[id] = 0
	gUserLastAttack[id] = 0.0

	remove_task(TASK6)
	set_task(3.0, "task_checkplayercount", TASK6, _, _, "b")
}

public client_disconnect(id){
	// 最后一个玩家退出 结束回合
	if(get_csteam_num(1, 0) + get_csteam_num(2, 0) == 1){
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
	//for(new id=1;id<=gMaxPlayers;++id)
	//	client_putinserver(id)
}

public EventStartRound(){
	if(get_csteam_num(1, 0) + get_csteam_num(2, 0) == 0){
		return
	}

	CheckMonster = get_gametime() + 20.0

	remove_all_enemy()
	gIsGameStarted = 1
	gRoundStart = 1

	new Float:origin[3]
	while(true){
		if( (!gPrincessCenter[0]&&!gPrincessCenter[1]&&!gPrincessCenter[2]&&navmesh_randomPosition(origin))
		|| ((gPrincessCenter[0]||gPrincessCenter[1]||gPrincessCenter[2])&& navmesh_getRandomAreaPos(gPrincessCenter, 0.0, 1200.0, origin)) ){
			origin[2] += 41.0
			if(is_hull_vacant(origin)){
				resetPrincess(gPrincess, origin)
				client_color(0, "/ctr公主/y出现在了地图上,/g找到/y并/g保护/y她!")
				gPrincessTime = get_gametime()
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

public EventHLTV(){
	for(new id=1;id<=gMaxPlayers;++id)
		client_putinserver(id)

	remove_all_npc()
}

public HAM_HostageKilled_post(ent, killer){
	set_hudmessage(120,54,54, 0.02, 0.62, 1, 0.0, 5.5, 0.2, 0.2, HUD_GAMEMSG)
	show_hudmessage(0, "任务失败: 公主已经被杀害...")

	server_cmd("endround T")
}
