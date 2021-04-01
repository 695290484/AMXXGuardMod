
// native rpg_create_g(Float:origin[3], Float:angles[3], iMonsterlevel, szClass[], szName[], Float:fHealth,  Float:fMinsize[3], Float:fMaxsize[3], iModelID)
public _native_rpg_create(iPlugin,iParams){
	if(iParams != 9)
		return 0

	new Float:origin[3], Float:angles[3], iMonsterlevel, szClass[32], szName[32], Float:fHealth, Float:fMaxsize[3], Float:fMinsize[3], iModelID
	get_array_f(1, origin, 3)
	get_array_f(2, angles, 3)
	iMonsterlevel = get_param(3)
	get_string(4, szClass, charsmax(szClass))
	get_string(5, szName, charsmax(szName))
	fHealth = get_param_f(6)
	get_array_f(7, fMinsize, 3)
	get_array_f(8, fMaxsize, 3)
	iModelID = get_param(9)
	return create_monster(origin, angles, iMonsterlevel, szClass, szName, fHealth,  fMinsize, fMaxsize, iModelID)
}

// native rpg_precache_g(model[], walkSeq=-1, Float:walkDistance=0.0, runSeq=-1, Float:runDistance=0.0, &animCode)
public _native_rpg_precache(iPlugin,iParams){
	if(iParams != 6)
		return 0

	if(gMTCounter >= MONSTER_TYPES){
		set_param_byref(6, -1)
		return 0
	}

	new szTemp[128], precacheId
	get_string(1, szTemp, charsmax(szTemp))
	precacheId = engfunc(EngFunc_PrecacheModel, szTemp)

	for(new i=0;i<gMTCounter;++i){
		if(gMTModelIndex[i] == precacheId){
			set_param_byref(6, i)
			return precacheId
		}
	}

	new walkSeq = get_param(2), runSeq = get_param(4)
	if(walkSeq<0 && runSeq<0){
		set_param_byref(6, -1)
		return 0
	}

	gMTModelIndex[gMTCounter] = precacheId
	copy(gMTModel[gMTCounter], charsmax(gMTModel[]), szTemp)

	gMTWalkSeq[gMTCounter] = walkSeq>=0?walkSeq:-1
	gMTRunSeq[gMTCounter] = runSeq>=0?runSeq:-1

	if(walkSeq>=0 || runSeq>=0){
		new file = fopen(szTemp, "rt")
		new numseq, seqindex, data[33]
		fseek(file, 164, SEEK_SET)
		fread(file, numseq, BLOCK_INT)
		fread(file, seqindex, BLOCK_INT)
		for(new i = 0; i < numseq; i++){
			if(i == walkSeq || i == runSeq){
				fseek(file, seqindex + 176*i, SEEK_SET)
				fread_blocks(file, data/*gMTLabel[gMTCounter]*/, 32, BLOCK_CHAR)
				if(i == walkSeq)
					fread(file, gMTWalkFPS[gMTCounter], BLOCK_INT)
				else
					fread(file, gMTRunFPS[gMTCounter], BLOCK_INT)
				fseek(file, 20, SEEK_CUR)
				if(i == walkSeq)
					fread(file, gMTWalkFrames[gMTCounter], BLOCK_INT)
				else
					fread(file, gMTRunFrames[gMTCounter], BLOCK_INT)
			}
		}
		fclose(file)
	}

	new Float:walkDistance, Float:runDistance
	walkDistance = get_param_f(3)
	runDistance = get_param_f(5)
	if(walkSeq>=0 && gMTWalkFrames[gMTCounter] && gMTWalkFPS[gMTCounter]){
		gMTWalkSpeed[gMTCounter] = walkDistance / (gMTWalkFrames[gMTCounter] / gMTWalkFPS[gMTCounter])
	}
	if(runSeq>=0 && gMTRunFrames[gMTCounter] && gMTRunFPS[gMTCounter]){
		gMTRunSpeed[gMTCounter] = runDistance / (gMTRunFrames[gMTCounter] / gMTRunFPS[gMTCounter])
	}

	//server_print("model:%s,modelIndex:%d,walkFPS:%f,walkframes:%d,runFPS:%f,runframes:%d", gMTModel[gMTCounter],gMTModelIndex[gMTCounter],gMTWalkFPS[gMTCounter],gMTWalkFrames[gMTCounter],gMTRunFPS[gMTCounter],gMTRunFrames[gMTCounter])
	set_param_byref(6, gMTCounter)
	gMTCounter ++

	new szLeft[128], szRight[4]
	strtok(szTemp, szLeft, charsmax(szLeft), szRight, charsmax(szRight), '.')
	formatex(szTemp, charsmax(szTemp), "%sT.mdl", szLeft)
	if(file_exists(szTemp))
		engfunc(EngFunc_PrecacheModel, szTemp)

	return precacheId
}

// native rpg_animation_g(iEntity, iAnim, animCode=-1, Float:framerate=1.0, again=0, Float:gaptime=0.0)
public _native_rpg_animation(iPlugin,iParams){
	if(iParams != 6)
		return

	new iEntity, iAnim, Float:framerate, again, Float:gaptime
	new animCode = get_param(3)
	iEntity = get_param(1)
	iAnim = get_param(2)
	framerate = get_param_f(4)
	if(animCode>=0){
		static Float:velocity[3], Float:speed
		pev(iEntity, pev_velocity, velocity)
		velocity[2] = 0.0
		speed = vector_length(velocity)

		if(iAnim == gMTWalkSeq[animCode]){
			framerate *= (speed / gMTWalkSpeed[animCode]) * get_param_f(4)

		}else if(iAnim == gMTRunSeq[animCode]){
			framerate *= (speed / gMTRunSpeed[animCode]) * get_param_f(4)
		}
		//server_print(">=0 :%f,%f | %f,%f,%f | %f", framerate,get_param_f(4),speed,gMTWalkSpeed[animCode],gMTRunSpeed[animCode],(speed / gMTWalkSpeed[animCode]) * get_param_f(4))
	}
	again = get_param(5)
	gaptime = get_param_f(6)

	SetAnimationGaptime(iEntity, iAnim, framerate, again, gaptime)
}

// native rpg_set_animinterrupt_g(iEntity, Float:stopUntil)
public _native_rpg_set_animinterrupt(iPlugin,iParams){
	if(iParams != 2)
		return
	
	gAnimInterrupt[get_param(1)] = get_param_f(2)
}

// native rpg_delaydmg_g(ent, Float:damage, Float:delay, Float:distance, dmgType, f2f=1)
public _native_rpg_delaydmg(iPlugin,iParams){
	if(iParams != 6)
		return
	
	dmgDelay(get_param(1), get_param_f(2), get_param_f(3), get_param_f(4), get_param(5), get_param(6))
}

public _native_set_user_extradamage(iPlugin,iParams){
	if(iParams != 2)
		return
	
	extDmgCount2[get_param(1)] = get_param_f(2)
}

public Float:_native_get_user_extradamage(id){
	return extDmgCount2[id]
}

public _native_set_user_normaldamage(iPlugin,iParams){
	if(iParams != 2)
		return
	
	extDmgCount[get_param(1)] = get_param_f(2)
}

public Float:_native_get_user_normaldamage(id){
	return extDmgCount[id]
}

public _native_get_princess(){
	return gPrincess
}

public _native_set_princessArea(iPlugin,iParams){
	if(iParams != 1)
		return
	
	get_array_f(1, gPrincessCenter, 3)
}

public _native_set_monsterArea(iPlugin,iParams){
	if(iParams != 1)
		return
	
	get_array_f(1, gMonsterCenter, 3)
}

public _native_get_darklevel(){
	return gCurrentDarkLevel
}

public _native_is_monster(ent) return IsMonster(ent)
public _native_is_nonplayer(ent) return IsNonPlayer(ent)