
/* 寻路 */
reset_pathfindvars(iEntity)
{
	for(new i;i<8;++i) g_way[iEntity][i][0]=g_way[iEntity][i][1]=g_way[iEntity][i][2]=0.0
	g_pathLength[iEntity] = 0
	g_nextPoint[iEntity] = 2
	g_checkInterval[iEntity] = 0.0
	g_stopPathFinding[iEntity] = 0.0
	g_stopHandsChecking[iEntity] = 0.0
}

walk_in_path(ent){
	static Float:goal[3], Float:origin[3], enemy, Float:tmpVel[3]
	pev(ent, pev_origin, origin)

	enemy = pev(ent, pev_enemy)
	pev(enemy, pev_origin, goal)
	
	static Float:attackradius
	attackradius = GetMD_float(ent, md_attackradius)
	attackradius = attackradius?attackradius:80.0
	
	if(enemy <= gMaxPlayers && length2D(origin, goal) <= attackradius && floatabs(origin[2] - goal[2]) <= attackradius * 1.35){ // 到目标处了(攻击距离)
		ExecuteForward(g_fwAttack, g_fwDummyResult, ent, enemy)
		SetEntityTurn(ent, goal, 0)
		if(g_fwDummyResult > 0)
			return 1
		return 0
	}

	static Float:mins[3], Float:maxs[3]
	static Float:absmins1[3], Float:absmaxs1[3],Float:absmins2[3], Float:absmaxs2[3]
	pev(ent, pev_mins, mins)
	pev(ent, pev_maxs, maxs)

	if(enemy > gMaxPlayers){
		pev(ent, pev_absmin, absmins1)
		pev(ent, pev_absmax, absmaxs1)
		pev(enemy, pev_absmin, absmins2)
		pev(enemy, pev_absmax, absmaxs2)
		if(get_box_distance2(absmins1,absmaxs1,absmins2,absmaxs2)<(attackradius?attackradius:80.0)){
			ExecuteForward(g_fwAttack, g_fwDummyResult, ent, enemy)
			if(g_fwDummyResult > 0)
				return 1
			return 0
		}
	}

	if(g_nextPoint[ent] >= g_pathLength[ent])
		return 0

	static Float:angles[3], Float:velocity[3], Float:maxspeed
	pev(ent, pev_velocity, velocity)
	pev(ent, pev_maxspeed, maxspeed)
	
	static flag
	flag = pev(ent, pev_flags)

	static  Float:gtime
	gtime = get_gametime()
	origin[2] = origin[2] + mins[2] + 5.0

	if(g_stopPathFinding[ent] < gtime){
		// 直追
		if(g_pathLength[ent]<=4 && can_see_entity(ent, enemy)){
			pev(enemy, pev_origin, goal)

			if(flag&FL_ONGROUND && check_jumping(ent, velocity)){
				jump(ent, velocity, velocity)
			}
		}else{
			pev(ent, pev_angles, angles)


			new Float:oldFirstPos[3]
			if(!oldFirstPos[0] && !oldFirstPos[1] && !oldFirstPos[2]){
				oldFirstPos = g_way[ent][g_nextPoint[ent]]
			}
			SetEntityTurn(ent, oldFirstPos)

			//if(g_nextPoint[ent] == 3 || g_nextPoint[ent] == 4){
			//	new Float:outAdjust[3]
			//	navmesh_feeleradjustment(angles[1], origin, outAdjust)
			//	if(outAdjust[0] || outAdjust[1] || outAdjust[2]){
			//		g_way[ent][g_nextPoint[ent]] = outAdjust
			//	}
				//server_print("%f,%f,%f",g_way[ent][g_nextPoint[ent]][0],g_way[ent][g_nextPoint[ent]][1],g_way[ent][g_nextPoint[ent]][2])
			//}

			goal = g_way[ent][g_nextPoint[ent]]

			angle_vector(angles, 1, angles)
			static Float:v[3]
			xs_vec_sub(goal, origin, v)

			if(IsLengthLessThan(origin, goal, 35.0) || (
			xs_vec_dot(v, angles)<0 && IsLengthLessThan(origin, goal, 80.0) && can_see(ent, origin, goal))){

				if(flag&FL_ONGROUND){
					if(navmesh_attribute(g_nextPoint[ent]) == NAV_JUMP)
						jump(ent, velocity, velocity)
					if(check_jumping(ent, velocity))
						jump(ent, velocity, velocity)
				}
				g_nextPoint[ent] ++
			}else{
				if(flag&FL_ONGROUND && check_jumping(ent, velocity)){
					jump(ent, velocity, velocity)
				}
			}
		}
	}else{
		goal = g_way[ent][g_nextPoint[ent]]
	}

	SetEntityTurn(ent, goal)

	pev(ent, pev_angles, angles)

	angle_vector(angles, 1, tmpVel)
	xs_vec_mul_scalar(tmpVel, maxspeed, tmpVel)
	tmpVel[2] = velocity[2]

	// 空中落地减速
	if(!(gLastFlags[ent] & FL_ONGROUND) && (flag & FL_ONGROUND)){
		set_pdata_float(ent, m_flVelocityModifier, 0.67)
	}

	velocity = tmpVel
	static Float:vm
	vm = get_pdata_float(ent, m_flVelocityModifier)
	if(vm<1.0){
		velocity[2] = 0.0
		xs_vec_mul_scalar(velocity, vm, velocity)
		velocity[2] = tmpVel[2]
		vm += 0.08
		set_pdata_float(ent, m_flVelocityModifier, floatmin(vm,1.0))
	}

	vm = GetMD_float(ent, md_speedmodify)
	if(vm > 0.0){
		velocity[2] = 0.0
		xs_vec_mul_scalar(velocity, vm, velocity)
		velocity[2] = tmpVel[2]
	}

	if(flag & FL_ONGROUND){

		// hands
		origin[2] -= (mins[2] + 5.0)
		static Float:startPoint[2][3], Float:endPoint[2][3]
		static Float:tempAng[3][3]
		new Float:maxlen = floatmax(maxs[0] - mins[0], maxs[1] - mins[1]) * 0.5
		angles[0] = angles[2] = 0.0
		angles[1] -= 90.0
		angle_vector(angles, ANGLEVECTOR_FORWARD, tempAng[0])
		xs_vec_mul_scalar(tempAng[0],maxlen,tempAng[0])
		xs_vec_add(tempAng[0], origin, startPoint[0]);
		angles[1] += 180.0
		angle_vector(angles, ANGLEVECTOR_FORWARD, tempAng[1])
		//xs_vec_normalize(tempAng,tempAng)
		xs_vec_mul_scalar(tempAng[1],maxlen,tempAng[1])
		xs_vec_add(tempAng[1], origin, startPoint[1]);
		angles[1] -= 90.0
		angle_vector(angles, ANGLEVECTOR_FORWARD, tempAng[2])
		xs_vec_mul_scalar(tempAng[2],maxlen*1.8,tempAng[2])
		xs_vec_add(startPoint[0], tempAng[2], endPoint[0])
		xs_vec_add(startPoint[1], tempAng[2], endPoint[1])
		new right = can_see_ign_entity(ent, startPoint[0], endPoint[0])
		new left = can_see_ign_entity(ent, startPoint[1], endPoint[1])

		if(right && left && g_stopPathFinding[ent] < gtime || velocity[2]){
			ExecuteForward(g_fwMove, g_fwDummyResult, ent)
			engfunc(EngFunc_WalkMove, ent, angles[1], 0.5, WALKMOVE_NORMAL)
			set_pev(ent, pev_velocity, velocity)
		}else if(right != left){
			if(g_stopHandsChecking[ent] < gtime){
				if(g_stopPathFinding[ent] < gtime){
					if(g_nextPoint[ent]>0)
						g_nextPoint[ent] --

					g_stopPathFinding[ent] = gtime + 1.5 // 偏移路径x秒

					xs_vec_mul_scalar(tempAng[0], 3.6, endPoint[0])
					xs_vec_mul_scalar(tempAng[1], 3.6, endPoint[1])
					xs_vec_add(endPoint[0], origin, endPoint[0]);
					xs_vec_add(endPoint[1], origin, endPoint[1]);
					#if defined DEBUG
					DrawLine2(startPoint[0], endPoint[0])
					DrawLine2(startPoint[1], endPoint[1])
					#endif
					
					g_way[ent][g_nextPoint[ent]]  = left?endPoint[1]:endPoint[0]

					remove_task(ent +TASK2)
					set_task(1.4, "task_find", ent +TASK2)

					
				}else{
					engfunc(EngFunc_WalkMove, ent, angles[1], 0.5, WALKMOVE_NORMAL)
					set_pev(ent, pev_velocity, velocity)

					if(IsLengthLessThan(origin, goal, 18.0)){
						g_stopPathFinding[ent] = gtime
						remove_task(ent +TASK2)
						task_find(ent +TASK2)
						g_stopHandsChecking[ent] = gtime + 2.0
					}
				}
			}
		}else{
			if(g_stopHandsChecking[ent] < gtime){
				g_stopPathFinding[ent] = gtime
				remove_task(ent +TASK2)
				task_find(ent +TASK2)
				g_stopHandsChecking[ent] = gtime + 2.0
			}
		}
	}else{
		engfunc(EngFunc_WalkMove, ent, angles[1], 0.5, WALKMOVE_NORMAL)
		set_pev(ent, pev_velocity, velocity)
	}
	return 0
}


check_jumping(ent, Float:vecVel[3])
{
	new xp = 0

	if (!(pev(ent,pev_flags) & FL_ONGROUND)) return 0

	static Float:gtime
	gtime = get_gametime()
	if(gtime -g_checkInterval[ent] < 0.1) return 0 // 跳跃时间间隔
	g_checkInterval[ent] = gtime

	static Float:NpcOrg[3]
	pev(ent, pev_origin, NpcOrg)
	
	static Float:ang[3]
	if(!vecVel[0] && !vecVel[1])
		vector_to_angle(vecVel, ang)
	else
		pev(ent, pev_angles, ang)

	static Float:dest[3]
	angle_vector(ang, 1, dest)
	
	static Float:vec[3]
	xs_vec_mul_scalar(dest, 30.0, vec)
	
	xs_vec_add(NpcOrg, vec, vec)
	
	new tr;tr=create_tr2()
	engfunc(EngFunc_TraceMonsterHull, ent, NpcOrg, vec, 0, ent, tr)
	
	new Float:fraction
	get_tr2(tr, TR_flFraction, fraction)
	
	new Float:end[3], Float:plane[3]
	get_tr2(tr, TR_vecEndPos, end)
	get_tr2(tr, TR_vecPlaneNormal, plane)

	free_tr2(tr)
	
	vector_to_angle(plane, ang)
	
	if (fraction!=1.0 && ang[0]<30.0)
	{
		new Float:g, Float:h, Float:powerdif
		pev(ent, pev_gravity, g)
		g=g?g:1.0
		powerdif =  GetMD_float(ent, md_jumppower) - 350.0
		if(powerdif > 0)
			h=(350.0*350.0)/(1.0*get_cvar_float("sv_gravity")*g) + powerdif
		else
			h=(350.0*350.0)/(1.0*get_cvar_float("sv_gravity")*g)
		
		static Float:start[3]
		xs_vec_copy(NpcOrg, start)
		xs_vec_add(end, dest, end)
		xs_vec_add(end, dest, end)
		
		while (start[2]-NpcOrg[2] <h)
		{
			start[2]+=h/10.0
			end[2]+=h/10.0
			

			tr=create_tr2()
			engfunc(EngFunc_TraceMonsterHull, ent, start, end, 0, ent, tr)
			
			new iHit=get_tr2(tr,TR_pHit)
			
			if(pev_valid(iHit)/* && !rpg_is_monster(iHit)*/)
			{
				static myclass[33]
				pev(ent ,pev_classname, myclass, 32)
				
				static class[33]
				pev(iHit, pev_classname, class, 32)
				if(equal(class, "player") || pev(ent, PEV_MONSTER) == MONSTER || pev(ent, PEV_MONSTER) == NONPLAYER) 
				{
					break
				}
			}
			
			get_tr2(tr, TR_flFraction, fraction)
			
			if (get_tr2(tr,TR_StartSolid) || get_tr2(tr,TR_AllSolid) || !get_tr2(tr,TR_InOpen))
			{
				free_tr2(tr)
				break
			}
				
			free_tr2(tr)
				
			if (fraction==1.0)
			{
				if(start[2] - NpcOrg[2] <= 18.0)
				{
					break
				}
			
				xp = 1
				g_stopHandsChecking[ent] = gtime + 3.0 // 跳完后3秒内不用双手检测
			
				break
			}
		}
	}
	
	return xp
}

jump(ent, Float:v1[3], Float:out[3]){
	new Float:jumppower = GetMD_float(ent, md_jumppower)

	out[0] = v1[0]
	out[1] = v1[1]
	out[2] = jumppower?jumppower:350.0

	g_checkInterval[ent] = get_gametime() + 1.2

	// jump animation
	ExecuteForward(g_fwJump, g_fwDummyResult, ent)
}
