
/* stocks */
stock get_speed_vector_to_entity(ent1, ent2, Float:speed, Float:new_velocity[3])
{
	if(!pev_valid(ent1) || !pev_valid(ent2))
	return 0;
	
	new Float:origin1[3]
	pev(ent1,pev_origin,origin1)
	new Float:origin2[3]
	pev(ent2,pev_origin,origin2)
	
	new_velocity[0] = origin2[0] - origin1[0];
	new_velocity[1] = origin2[1] - origin1[1];
	new_velocity[2] = origin2[2] - origin1[2];
	
	new Float:num
	num = speed / vector_length(new_velocity);
	
	new_velocity[0] *= num;
	new_velocity[1] *= num;
	new_velocity[2] *= num;
	
	if(new_velocity[2] > 1500.0)
	new_velocity[2] = 1500.0
	else if(new_velocity[2] < -200.0)
	new_velocity[2] = -200.0
	
	return 1;
}

stock can_see(ent, Float:origin[3], Float:origin2[3])
{
	engfunc(EngFunc_TraceLine, origin, origin2, DONT_IGNORE_MONSTERS, ent, 0)
	new Float:fraction
	get_tr2(0, TR_flFraction, fraction)
	if(fraction != 1.0)
		return 0
	
	return 1
}

stock can_see_ign_entity(ent, Float:origin[3], Float:origin2[3])
{
	engfunc(EngFunc_TraceLine, origin, origin2, DONT_IGNORE_MONSTERS, ent, 0)
	new Float:fraction
	get_tr2(0, TR_flFraction, fraction)
	new hit = get_tr2(0, TR_pHit)
	if(fraction != 1.0 && (!hit || !pev_valid(hit) || pev(hit, pev_solid) != SOLID_BSP))
		return 0
	
	return 1
}

stock can_see_entity(ent, ent2)
{
	new Float:origin1[3], Float:origin2[3], Float:mins[3]
	pev(ent, pev_mins, mins)
	pev(ent, pev_origin, origin1)
	origin1[2] += mins[2] + 5.0
	pev(ent2, pev_origin, origin2)
	origin2[2] += 20.0

	engfunc(EngFunc_TraceLine, origin1, origin2, DONT_IGNORE_MONSTERS, ent, 0)

	new Float:flFraction
	get_tr2(0, TraceResult:TR_flFraction, flFraction)
	
	if (flFraction == 1.0)
	return true
	
	new iHit = get_tr2(0, TR_pHit)

	return (iHit > 0 && iHit == ent2)
}

stock set_angles_smooth(ent, Float:angles[3], Float:fDegree)
{
	new Float:oldang[3], Float:diff, Float:shift_degree, Float:absdiff
	pev(ent, pev_angles, oldang)
	shift_degree = fDegree
	if(angles[1] < 0.0) angles[1] += 360.0

	diff = angles[1]-oldang[1]
	absdiff = floatabs(diff)

	if(!absdiff)
	{
		set_pev(ent, pev_angles, angles)
		set_pev(ent, pev_v_angle, angles)
	}
	else if(absdiff < shift_degree)
	{
		set_pev(ent, pev_angles, angles)
		set_pev(ent, pev_v_angle, angles)
	}
	else{
		// y=(x/60)^0.5
		new Float:decay = (floatpower(absdiff / 60.0, 0.5)  / 1.7333) * shift_degree 

		if(absdiff == 180.0)
		{
			oldang[1] += decay
		}
		else if((diff > 0.0 && diff <= 180.0) || (diff < -180.0))
		{
			oldang[1] += decay
		}
		else
		{
			oldang[1] -= decay
		}

		if(oldang[1]>=360.0) oldang[1]-=360.0
		else if(oldang[1]<0.0) oldang[1]+=360.0

		set_pev(ent, pev_angles, oldang)
		set_pev(ent, pev_v_angle, oldang)
	}

	return absdiff > 75.0
}

stock SetEntityTurn(iEntity, Float:target[3], smooth=1)
{
	new Float:angle[3], Float:origin[3]
	pev(iEntity, pev_angles, angle)
	pev(iEntity, pev_origin, origin)
	
	new Float:x = origin[0] - target[0]
	new Float:z = origin[1] - target[1]
	new Float:radians = floatatan(z/x, radian)
	angle[1] = radians * 180.0/3.141592654
	if(target[0] < origin[0]) angle[1] -= 180.0
	angle[0] = angle[2] = 0.0

	if(smooth){
		new d = set_angles_smooth(iEntity, angle, ANGLES_SMOOTH)
		if(d) set_pdata_float(iEntity, m_flVelocityModifier, 0.2)
	}
	else{
		set_pev(iEntity, pev_angles, angle)
		set_pev(iEntity, pev_v_angle, angle)
	}
}

#if defined DEBUG
test_drawline(ent){
	for(new i = 2; i < g_pathLength[ent]; ++i)
		DrawLine(g_way[ent][i-1], g_way[ent][i], 1)
}

DrawLine(const Float:sorigin[3],const Float:eorigin[3], type=0)
{
	engfunc(EngFunc_MessageBegin, MSG_PVS, SVC_TEMPENTITY, sorigin, 0)
	write_byte(TE_BEAMPOINTS)	// temp entity event
	engfunc(EngFunc_WriteCoord, sorigin[0]) // x
	engfunc(EngFunc_WriteCoord, sorigin[1]) // y
	engfunc(EngFunc_WriteCoord, sorigin[2]) // z +15.0
	engfunc(EngFunc_WriteCoord, eorigin[0]) // x axis
	engfunc(EngFunc_WriteCoord, eorigin[1]) // y axis
	engfunc(EngFunc_WriteCoord, eorigin[2]) // z axis+15.0
	write_short(laser)	// sprite index
	write_byte(0)			// start frame
	write_byte(0)			// framerate
	write_byte(type?4:100)			// life in 0.1's
	write_byte(20)			// line width in 0.1's
	write_byte(0)			// noise amplitude in 0.01's
	write_byte(type?255:120)		        // color: red
	write_byte(type?75:220)		        // color: green
	write_byte(type?0:120)		        // color: blue
	write_byte(120)			// brightness
	write_byte(0)			// scroll speed in 0.1's
	message_end() 
}

DrawLine2(const Float:sorigin[3],const Float:eorigin[3])
{
       engfunc(EngFunc_MessageBegin, MSG_PVS, SVC_TEMPENTITY, sorigin, 0)
       write_byte(TE_BEAMPOINTS)	// temp entity event
       engfunc(EngFunc_WriteCoord, sorigin[0]) // x
       engfunc(EngFunc_WriteCoord, sorigin[1]) // y
       engfunc(EngFunc_WriteCoord, sorigin[2]) // z +15.0
       engfunc(EngFunc_WriteCoord, eorigin[0]) // x axis
       engfunc(EngFunc_WriteCoord, eorigin[1]) // y axis
       engfunc(EngFunc_WriteCoord, eorigin[2]) // z axis+15.0
       write_short(laser)	// sprite index
       write_byte(0)			// start frame
       write_byte(0)			// framerate
       write_byte(1)			// life in 0.1's
       write_byte(20)			// line width in 0.1's
       write_byte(0)			// noise amplitude in 0.01's
       write_byte(255)		        // color: red
       write_byte(220)		        // color: green
       write_byte(120)		        // color: blue
       write_byte(120)			// brightness
       write_byte(0)			// scroll speed in 0.1's
       message_end() 
}

#endif

stock SetAnimationGaptime(iEntity, iAnim, Float:framerate=1.0, again=0, Float:gaptime=0.0)
{
	set_pev(iEntity, pev_framerate, framerate)

	new lastAnim = pev(iEntity, pev_sequence)
	if(lastAnim== iAnim && !again)
		return

	static Float:gtime, Float:gametime
	pev(iEntity, pev_animgap, gtime)
	gametime = get_gametime()
	if(again && gametime - gtime < gaptime)
		return

	if(lastAnim != iAnim && gametime < gAnimInterrupt[iEntity])
		return

	set_pev(iEntity, pev_animgap, gametime)
	set_pev(iEntity, pev_sequence, iAnim)
	set_pev(iEntity, pev_animtime, gametime)
	set_pev(iEntity, pev_frame, 0.0)

	//server_print("%d,%d,%f,%d,%f",iEntity, iAnim, framerate, again, gaptime)
}


// 怪物实体,伤害,延迟,距离,是否需要正面(1正面 -1背面 0不考虑)
stock dmgDelay(ent, Float:damage, Float:delay, Float:distance, dmgType=DMG_FALL, f2f=1){
	new data[5]
	data[0] = ent
	data[1] = floatround(damage)
	data[2] = floatround(distance)
	data[3] = f2f
	data[4] = dmgType
	set_task(delay, "task_attackdelay", ent + TASK3, data, 5)
}

stock can_see_entity_f2f(ent, ent2)
{
	new Float:angles[3], Float:vecEntToEnt2[3], Float:vecAng[3]
	new Float:origin1[3], Float:origin2[3]
	pev(ent, pev_origin, origin1)
	pev(ent2, pev_origin, origin2)
	pev(ent, pev_angles, angles)
	
	xs_vec_sub(origin2, origin1, vecEntToEnt2)
	angle_vector(angles, 1, vecAng)
	if(xs_vec_dot(vecEntToEnt2, vecAng)<0) return 0

    	engfunc(EngFunc_TraceLine, origin1, origin2, DONT_IGNORE_MONSTERS, ent, 0)

	new iHit = get_tr2(0, TR_pHit)
	return (iHit > 0 && iHit == ent2)
}


stock findRandomEnemy(){
	new players[32], count
	for(new id=1;id<=gMaxPlayers;++id)
	{
		if(is_user_alive(id)){
			players[count] = id
			count++
		}
	}
	return count>0?players[random_num(0, count-1)]:0
}

stock Float:get_box_distance2(Float:mins1[3],Float:maxs1[3],Float:mins2[3],Float:maxs2[3])
{
	new Float:distance[3], i, Float:max, Float:min;

	for(i=0;i<3;i++)
	{
		min = mins1[i];
		max = maxs2[i];

		if(min > max)
		{
			distance[i] = min - max;
			continue;
		}

		min = mins2[i];
		max = maxs1[i];

		if(min > max) distance[i] = min - max;
	}

	return distance[0] * distance[0] + distance[1] * distance[1] + distance[2] * distance[2];
}


stock SpawnBlood(const Float:vecOrigin[3], iColor, iAmount)
{
	if(iAmount == 0)
	return
	//iAmount *= 2
	if(iAmount > 255) iAmount = 255
	engfunc(EngFunc_MessageBegin, MSG_PVS, SVC_TEMPENTITY, vecOrigin)
	write_byte(TE_BLOODSPRITE)
	engfunc(EngFunc_WriteCoord, vecOrigin[0])
	engfunc(EngFunc_WriteCoord, vecOrigin[1])
	engfunc(EngFunc_WriteCoord, vecOrigin[2]) 
	write_short(spr_blood_spray)
	write_short(spr_blood_drop)
	write_byte(iColor)
	write_byte(min(max(3, iAmount / 10), 19))
	message_end()
}


// 来自CSDM
// Place user at a random spawn
stock do_random_spawn(ent, csdmspawns, Float:out[3])
{
	switch(csdmspawns){
		case 0:{
			if(setNomalSpawnPoint2(ent, out)){
				return 1
			}
		}
		case 1:{
			if(setNomalSpawnPoint2(ent, out)){

				if(navmesh_getRandomAreaPos(out, 80.0, 280.0, out)){
					return 1
				}
			}
		}
		case 2:{
			if(setNomalSpawnPoint(ent, out)){
				return 1
			}
		}
		case 3:{
			if(setNomalSpawnPoint(ent, out)){
				if(navmesh_getRandomAreaPos(out, 80.0, 280.0, out)){
					return 1
				}
			}
		}
	}
	return 0
}

// Collect random spawn points
stock load_spawns()
{
	// Check for CSDM spawns of the current map
	new cfgdir[32], mapname[32], filepath[100], linedata[64]
	get_configsdir(cfgdir, charsmax(cfgdir))
	get_mapname(mapname, charsmax(mapname))
	formatex(filepath, charsmax(filepath), "%s/csdm/%s.spawns.cfg", cfgdir, mapname)
	
	// Load CSDM spawns if present
	if (file_exists(filepath))
	{
		new csdmdata[10][6], file = fopen(filepath,"rt")
		
		while (file && !feof(file))
		{
			fgets(file, linedata, charsmax(linedata))
			
			// invalid spawn
			if(!linedata[0] || str_count(linedata,' ') < 2) continue;
			
			// get spawn point data
			parse(linedata,csdmdata[0],5,csdmdata[1],5,csdmdata[2],5,csdmdata[3],5,csdmdata[4],5,csdmdata[5],5,csdmdata[6],5,csdmdata[7],5,csdmdata[8],5,csdmdata[9],5)
			
			// origin
			g_spawns[g_spawnCount][0] = floatstr(csdmdata[0])
			g_spawns[g_spawnCount][1] = floatstr(csdmdata[1])
			g_spawns[g_spawnCount][2] = floatstr(csdmdata[2])
			
			// increase spawn count
			g_spawnCount++
			if (g_spawnCount >= sizeof g_spawns) break;
		}
		if (file) fclose(file)
	}
	else
	{
		// Collect regular spawns
		collect_spawns_ent("info_player_start")
		collect_spawns_ent("info_player_deathmatch")
	}
	
	// Collect regular spawns for non-random spawning unstuck
	collect_spawns_ent2("info_player_start")
	collect_spawns_ent2("info_player_deathmatch")
}

// Collect spawn points from entity origins
stock collect_spawns_ent(const classname[])
{
	new ent = -1
	while ((ent = engfunc(EngFunc_FindEntityByString, ent, "classname", classname)) != 0)
	{
		// get origin
		new Float:originF[3]
		pev(ent, pev_origin, originF)
		g_spawns[g_spawnCount][0] = originF[0]
		g_spawns[g_spawnCount][1] = originF[1]
		g_spawns[g_spawnCount][2] = originF[2]
		
		// increase spawn count
		g_spawnCount++
		if (g_spawnCount >= sizeof g_spawns) break;
	}
}

stock collect_spawns_ent2(const classname[])
{
	new ent = -1
	while ((ent = engfunc(EngFunc_FindEntityByString, ent, "classname", classname)) != 0)
	{
		// get origin
		new Float:originF[3]
		pev(ent, pev_origin, originF)
		g_spawns2[g_spawnCount2][0] = originF[0]
		g_spawns2[g_spawnCount2][1] = originF[1]
		g_spawns2[g_spawnCount2][2] = originF[2]
		
		// increase spawn count
		g_spawnCount2++
		if (g_spawnCount2 >= sizeof g_spawns2) break;
	}
}

stock is_hull_vacant(Float:origin[3], hull=HULL_HUMAN)
{
	engfunc(EngFunc_TraceHull, origin, origin, DONT_IGNORE_MONSTERS, hull, 0, 0)
	
	if (!get_tr2(0, TR_StartSolid) && !get_tr2(0, TR_AllSolid) && get_tr2(0, TR_InOpen))
		return true;
	
	return false;
}

stock str_count(const str[], searchchar)
{
	new count, i, len = strlen(str)
	
	for (i = 0; i <= len; i++)
	{
		if(str[i] == searchchar)
			count++
	}
	
	return count;
}

stock setNomalSpawnPoint(ent, Float:out[3]){
	// No spawns?
	if (!g_spawnCount)
		return 0; 
	
	static hull, i, sp_index
	
	// Get whether the player is crouching
	hull = (pev(ent, pev_flags) & FL_DUCKING) ? HULL_HEAD : HULL_HUMAN
	
	// Choose random spawn to start looping at
	sp_index = random_num(0, g_spawnCount - 1)
	
	// Try to find a clear spawn
	for (i = sp_index + 1; /*no condition*/; i++)
	{
		// Start over when we reach the end
		if (i >= g_spawnCount) i = 0
		
		// Free spawn space?
		if (is_hull_vacant(g_spawns[i], hull))
		{
			out = g_spawns[i];
			return 1;
		}
		
		// Loop completed, no free space found
		if (i == sp_index) break;
	}

	return 0
}

stock setNomalSpawnPoint2(ent, Float:out[3]){
	// No spawns?
	if (!g_spawnCount2)
		return 0; 
	
	static hull, i, sp_index
	
	// Get whether the player is crouching
	hull = (pev(ent, pev_flags) & FL_DUCKING) ? HULL_HEAD : HULL_HUMAN

	// Choose random spawn to start looping at
	sp_index = random_num(0, g_spawnCount2 - 1)
	
	// Try to find a clear spawn
	for (i = sp_index + 1; /*no condition*/; i++)
	{
		// Start over when we reach the end
		if (i >= g_spawnCount2) i = 0
		
		// Free spawn space?
		if (is_hull_vacant(g_spawns2[i], hull))
		{
			out = g_spawns2[i];
			return 1;
		}
		
		// Loop completed, no free space found
		if (i == sp_index) break;
	}

	return 0
}

stock UpdateFrags(id, frags, deaths, scoreboard)
{
	if(frags < 0)
		frags = pev(id, pev_frags)
	else
		set_pev(id, pev_frags, float(frags))

	if(deaths < 0)
		deaths = get_pdata_int(id, 444)
	else
		set_pdata_int(id, 444, deaths, 5)

	if (scoreboard)
	{
		message_begin(MSG_BROADCAST, g_msgScoreInfo)
		write_byte(id)
		write_short(frags)
		write_short(deaths)
		write_short(0)
		write_short(get_pdata_int(id, 114))
		message_end()
	}
}


// 解卡 from arkshine

new const START_DISTANCE  = 32   // --| The first search distance for finding a free location in the map.
new const MAX_ATTEMPTS  =  128  // --| How many times to search in an area for a free space.

new Float:gf_LastCmdTime[ 33 ];
enum Coord_e { Float:x, Float:y, Float:z }
public ClientCommand_UnStick ( const id )
{
	if(!is_user_stucked(id)){
		return PLUGIN_HANDLED
	}

	new Float:f_MinFrequency = 3.0;
	new Float:f_ElapsedCmdTime = get_gametime () - gf_LastCmdTime[ id ];


	if ( f_ElapsedCmdTime < f_MinFrequency ) 
	{
		//client_print ( id, print_chat, "[AMXX] You must wait %.1f seconds before trying to free yourself.", f_MinFrequency - f_ElapsedCmdTime );
		return PLUGIN_HANDLED;
	}

	gf_LastCmdTime[ id ] = get_gametime ();

	UTIL_UnstickPlayer ( id, START_DISTANCE, MAX_ATTEMPTS )

	return PLUGIN_HANDLED;
}


UTIL_UnstickPlayer ( const id, const i_StartDistance, const i_MaxAttempts )
{
	// --| Not alive, ignore.
	if ( !is_user_alive ( id ) )  return -1

	static Float:vf_OriginalOrigin[ Coord_e ], Float:vf_NewOrigin[ Coord_e ];
	static i_Attempts, i_Distance;

	// --| Get the current player's origin.
	pev ( id, pev_origin, vf_OriginalOrigin );

	i_Distance = i_StartDistance;

	while ( i_Distance < 1000 )
	{
		i_Attempts = i_MaxAttempts;

		while ( i_Attempts-- )
		{
			vf_NewOrigin[ x ] = random_float ( vf_OriginalOrigin[ x ] - i_Distance, vf_OriginalOrigin[ x ] + i_Distance );
			vf_NewOrigin[ y ] = random_float ( vf_OriginalOrigin[ y ] - i_Distance, vf_OriginalOrigin[ y ] + i_Distance );
			vf_NewOrigin[ z ] = random_float ( vf_OriginalOrigin[ z ] - i_Distance, vf_OriginalOrigin[ z ] + i_Distance );

			engfunc ( EngFunc_TraceHull, vf_NewOrigin, vf_NewOrigin, DONT_IGNORE_MONSTERS, GetPlayerHullSize ( id ), id, 0 );

			// --| Free space found.
			if ( get_tr2 ( 0, TR_InOpen ) && !get_tr2 ( 0, TR_AllSolid ) && !get_tr2 ( 0, TR_StartSolid ) )
			{
				// --| Set the new origin .
				engfunc ( EngFunc_SetOrigin, id, vf_NewOrigin );
				return 1;
			}
		}

		i_Distance += i_StartDistance;
	}

	// --| Could not be found.
	return 0;
}


stock bool:is_user_stucked(iPlayer)
{
	static Float:origin[3]
	pev(iPlayer, pev_origin, origin)
	engfunc(EngFunc_TraceHull, origin, origin, DONT_IGNORE_MONSTERS,(pev(iPlayer, pev_flags) & FL_DUCKING) ? HULL_HEAD : HULL_HUMAN, iPlayer, 0)
	if(get_tr2(0, TR_StartSolid) || get_tr2(0, TR_AllSolid) || !get_tr2(0, TR_InOpen))
		return true
	return false
}

stock f2f(ent, ent2)
{
	new Float:angles[3], Float:vecEntToEnt2[3], Float:vecAng[3]
	new Float:origin1[3], Float:origin2[3]
	pev(ent, pev_origin, origin1)
	pev(ent2, pev_origin, origin2)
	pev(ent, pev_angles, angles)
	
	xs_vec_sub(origin2, origin1, vecEntToEnt2)
	angle_vector(angles, 1, vecAng)
	if(xs_vec_dot(vecEntToEnt2, vecAng)<0)
		return 0

	return 1
}

stock showMonstersHP(attacker, ent){
	static TempText[81], Float:Hp, Float:MaxHp, pos
	pev(ent, pev_health, Hp)
	pev(ent, pev_max_health, MaxHp)

	static Float:fSize
	fSize = float(charsmax(TempText))

	pos = min(charsmax(TempText), max(0, floatround((fSize * Hp) / MaxHp)))

	formatex(TempText, charsmax(TempText), "________________________________________________________________________________")
	TempText[pos] = EOS

	static color[3]
	if(pos <= 20){
		color[0] = 255;color[1] = 0;color[2] = 0
	}else if(pos <= 40){
		color[0] = 255;color[1] = 140;color[2] = 0
	}else if(pos <= 60){
		color[0] = 173;color[1] = 255;color[2] = 47
	}else if(pos <= 80){
		color[0] = 125;color[1] = 255;color[2] = 0
	}else{
		color[0] = 0;color[1] = 128;color[2] = 0
	}

	set_hudmessage(color[0], color[1], color[2], 0.1, 0.10, 0, 0.0, 4.7, 0.0, 0.3, HUD_MONSTER)
	show_hudmessage(attacker, TempText)

	for(new id=1;id<=gMaxPlayers;++id){
		if(is_user_alive(id) || !is_user_connected(id) || pev(id, pev_iuser2) != attacker) continue
		set_hudmessage(color[0], color[1], color[2], 0.1, 0.10, 0, 0.0, 4.7, 0.0, 0.3, HUD_MONSTER)
		show_hudmessage(id, TempText)
	}

	new hb = GetMD_int(ent, md_healthbar)
	if(hb && pev_valid(hb)){
		if(MaxHp < Hp) set_pev(hb, pev_frame, 99.0)
		else set_pev(hb, pev_frame, 0.0 + (((Hp - 1.0) * 100.0) / MaxHp))
	}
}

stock client_color(const id, const input[], any:...)
{
	new msg[191], iLen = formatex(msg, 190, "^x03")
	vformat(msg[iLen], 190 - iLen, input, 3)
	replace_all(msg, 190, "/g", "^4")
	replace_all(msg, 190, "/y", "^1")
	replace_all(msg, 190, "/ctr", "^3")
	replace_all(msg, 190, "/w", "^0")
	message_begin(id ? MSG_ONE : MSG_BROADCAST, get_user_msgid( "SayText" ), _, id)
	write_byte(1)
	write_string(msg)
	message_end()
}

stock client_center(const id, const input[], any:...)
{
	new msg[129]
	vformat(msg, charsmax(msg), input, 3)
	message_begin(MSG_ONE, g_msgStatusText, {0, 0, 0}, id)
	write_byte(0)
	write_string(msg)
	message_end()
}

stock get_csteam_num(team, IsAlive){
	new count
	for(new i=1;i<=gMaxPlayers;++i){
		if(!is_user_connected(i)) continue
		if(IsAlive && !is_user_alive(i)) continue
		if(team && get_user_team(i) != team) continue
		count++
	}
	return count
}

stock remove_all_npc(){
	new target[32]
	for(new ent = gMaxPlayers+1; ent < 512; ++ent){
		if(IsMonster(ent))
			set_pev(ent, pev_flags, FL_KILLME)
		else if(IsNonPlayer(ent)){
			pev(ent, pev_targetname, target, charsmax(target))
			if(!strcmp(target, "公主")){
				hidePrincess(ent)
			}else
				set_pev(ent, pev_flags, FL_KILLME)
		}
	}
	gMonsterEntCounter = 0
}

stock remove_all_enemy(){
	for(new ent = gMaxPlayers+1; ent < 512; ++ent){
		if(IsMonster(ent))
			set_pev(ent, pev_enemy, 0)
	}
}

stock set_all_monster_death(){
	for(new ent = gMaxPlayers+1; ent < 512; ++ent){
		if(IsMonster(ent)){
			set_pev(ent, pev_deadflag, DEAD_DYING)
			set_pev(ent, pev_takedamage, DAMAGE_NO)
			set_pev(ent, pev_solid, SOLID_NOT)
			set_pev(ent, pev_nextthink, get_gametime()+ 0.02)

			if(pev(ent, pev_flags) & FL_ONGROUND){
				set_pev(ent, pev_movetype, MOVETYPE_NONE)
			}else{
				task_checkdeadbody(ent + TASK1)
			}

			remove_task(ent + TASK2)
			remove_task(ent + TASK5)
		}
	}
}


stock setMonsterRandomEnemy(ent){
	new rndPlayer = findRandomEnemy()
	if(!gDoNotCreatePrincess)
		set_pev(ent, pev_enemy, rndPlayer?(random_num(0,1)?rndPlayer:gPrincess):gPrincess)
	else
		set_pev(ent, pev_enemy, rndPlayer)
}

stock CheckHealthBar(ent){
	if(GetMD_int(ent, md_healthbar) == 1){
		new hb = engfunc(EngFunc_CreateNamedEntity, engfunc(EngFunc_AllocString, "env_sprite"))
		if(pev_valid(hb)){
			set_pev(hb, pev_scale, 0.15)
			engfunc(EngFunc_SetModel, hb, "sprites/rpg/healthbar.spr")
			SetMD_int(ent, md_healthbar, hb)
			set_pev(hb, pev_iuser4, 75)
			set_pev(hb, pev_frame, 99.0)
			new Float:Origin[3]
			pev(ent, pev_origin, Origin)
			Origin[2] += 75.0
			engfunc(EngFunc_SetOrigin, hb, Origin)
		}
	}
}

stock fm_get_user_money(id){
	return get_pdata_int(id, 115, 5)
}

stock fm_set_user_money(iPlayer, money, flag)
{
	set_pdata_int(iPlayer, 115, money, 5)
	message_begin(MSG_ONE, get_user_msgid("Money"), {0,0,0}, iPlayer)
	write_long(money)
	write_byte(flag)
	message_end()
}

stock Float:getVIPratio(id){
	new Float:racebnf = 0.0
	new flags = get_user_flags(id)
	if(flags & ADMIN_LEVEL_C) return 1.0 + racebnf
	else if(flags & ADMIN_LEVEL_B) return 0.7 + racebnf
	else if(flags & ADMIN_LEVEL_A) return 0.5 + racebnf
	return 0.0 + racebnf
}

stock e_register_cvar(name[], string[], flags = 0, Float:fvalue = 0.0)
{
	new cvar = register_cvar(name, string, flags, fvalue);

	static path[96];
	if(!path[0])
	{
		get_localinfo("amxx_configsdir", path, charsmax(path));
		format(path, charsmax(path), "%s/%s", path, "rpg-guard.cfg");
	}

	new file, print = true;
	static line[64];
	if(!file_exists(path))
	{
		file = fopen(path, "wt");
		if(!file)
			return cvar;
	}
	else
	{
		file = fopen(path, "rt");
		if(!file)
			return cvar;

		while(!feof(file))
		{
			fgets(file, line, charsmax(line));
			if(line[0] == ';' || !line[0])
				continue;

			if(contain(line, string) > 0)
			{
				print = false;
				break;
			}
		}
		fclose(file);
		file = fopen(path, "at");
	}

	if(print)
	{
		get_plugin(-1, line, charsmax(line));
		fprintf(file, "%-32s ^"%s^" // ^"%s^"^n", name, string, line);
	}

	fclose(file);
	return cvar;
}

stock float_weapon(ent)
{
	if(pev_valid(ent))
	{
		if(is_item_onground(ent) == 1.0){
			new Float:color[3]
			set_pev(ent,pev_renderfx,kRenderFxGlowShell)
			new Float:amt = 45.0
			set_pev(ent, pev_renderamt, amt)
			color[0] = 255.0
			color[1] = 255.0
			color[2] = 255.0
			set_pev(ent,pev_rendercolor,color)
			set_pev(ent,pev_velocity,Float:{0.0,0.0,0.0})
			set_pev(ent,pev_movetype,MOVETYPE_NOCLIP)
		}else{
			new Float:v[3]
			pev(ent, pev_velocity, v)
			xs_vec_mul_scalar(v, -0.2, v)
			set_pev(ent,pev_velocity,v)
			set_pev(ent, pev_nextthink, get_gametime()+1.0)
		}
	}
}

stock Float:is_item_onground(id)
{
	new Float:vOrigin[3], Float:fDist
	pev(id, pev_origin, vOrigin)
	fDist = vOrigin[2]
	
	while(engfunc(EngFunc_PointContents, vOrigin) == CONTENTS_EMPTY && vOrigin[2]>-9999.0)
		vOrigin[2] -= 1.0
	
	if(engfunc(EngFunc_PointContents, vOrigin) == CONTENTS_SOLID)
		return fDist - vOrigin[2]
	
	return 0.0
}

stock get_aim_origin_vector(iPlayer, Float:forw, Float:right, Float:up, Float:vStart[])
{
	new Float:vOrigin[3], Float:vAngle[3], Float:vForward[3], Float:vRight[3], Float:vUp[3]
	
	pev(iPlayer, pev_origin, vOrigin)
	pev(iPlayer, pev_view_ofs, vUp)
	xs_vec_add(vOrigin, vUp, vOrigin)
	pev(iPlayer, pev_v_angle, vAngle)
	
	angle_vector(vAngle, ANGLEVECTOR_FORWARD, vForward)
	angle_vector(vAngle, ANGLEVECTOR_RIGHT, vRight)
	angle_vector(vAngle, ANGLEVECTOR_UP, vUp)
	
	vStart[0] = vOrigin[0] + vForward[0] * forw + vRight[0] * right + vUp[0] * up
	vStart[1] = vOrigin[1] + vForward[1] * forw + vRight[1] * right + vUp[1] * up
	vStart[2] = vOrigin[2] + vForward[2] * forw + vRight[2] * right + vUp[2] * up
}

stock GetPlayerCurrentWeapon(iPlayer)
{
	new iWeapon = get_pdata_cbase(iPlayer, 373)
	if(iWeapon <= 0) return 0
	return get_pdata_int(iWeapon, 43, 4)
}


stock bool:fm_floor_entity(index)
{
	new Float:start[3], Float:dest[3];
	pev(index, pev_origin, start);
	dest[0] = start[0];
	dest[1] = start[1];
	dest[2] = -8191.0;

	engfunc(EngFunc_TraceLine, start, dest, DONT_IGNORE_MONSTERS, index, 0);
	new iEntity = get_tr2(0, TR_pHit);
	if(IsBuilding(iEntity) || IsTarget(iEntity))
		return true;
	
	return false;
}

stock Float:fm_distance_to_floor(index, ignoremonsters = 1)
{
	new Float:start[3], Float:dest[3], Float:end[3];
	pev(index, pev_origin, start);
	dest[0] = start[0];
	dest[1] = start[1];
	dest[2] = -8191.0;

	engfunc(EngFunc_TraceLine, start, dest, ignoremonsters, index, 0);
	get_tr2(0, TR_vecEndPos, end);

	pev(index, pev_absmin, start);
	new Float:ret = start[2] - end[2];

	return ret > 0 ? ret : 0.0;
}

stock CheckStuck(iEntity, Float:fMin[3], Float:fMax[3])
{
	new Float:testorigin[3]
	pev(iEntity, pev_origin, testorigin)

	new Float:Origin_F[3], Float:Origin_R[3], Float:Origin_L[3], Float:Origin_B[3]
	xs_vec_copy(testorigin, Origin_L) ; xs_vec_copy(testorigin, Origin_F)
	xs_vec_copy(testorigin, Origin_B) ; xs_vec_copy(testorigin, Origin_R)
	Origin_F[0] += fMax[0] ; Origin_B[0] += fMin[0]
	Origin_L[1] += fMax[1] ; Origin_R[1] += fMin[1]
	if(engfunc(EngFunc_PointContents, Origin_F) != CONTENTS_EMPTY || engfunc(EngFunc_PointContents, Origin_R) != CONTENTS_EMPTY ||
	engfunc(EngFunc_PointContents, Origin_L) != CONTENTS_EMPTY || engfunc(EngFunc_PointContents, Origin_B) != CONTENTS_EMPTY)
		return 0

	return 1
}
