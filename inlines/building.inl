
/* building */

setupBuilding(name[], Float:health, Float:mins[3], Float:maxs[3], model[], cost){
	formatex(gBuildingName[gBuildingCount], charsmax(gBuildingName[]), name)
	gBuildingHealth[gBuildingCount] = health
	gBuildingMins[gBuildingCount] = mins
	gBuildingMaxs[gBuildingCount] = maxs
	gBuildingCost[gBuildingCount] = cost

	new szTemp[128]
	formatex(szTemp, charsmax(szTemp), model)
	gBuildingModelIndex[gBuildingCount] = engfunc(EngFunc_PrecacheModel, szTemp)
	copy(szTemp[strlen(szTemp)-4], charsmax(szTemp) - (strlen(szTemp)-4), "T.mdl")
	if(file_exists(szTemp)) engfunc(EngFunc_PrecacheModel, szTemp)

	gBuildingCount ++
	return gBuildingCount - 1
}

buildMenu(iPlayer){
	new menuid, menu[256]
	formatex(menu, charsmax(menu), "建造\R价格")
	menuid = menu_create(menu, "buildermenuhandle")

	for(new i;i<gBuildingCount;++i){
		format(menu, charsmax(menu), "%s\R$%d", gBuildingName[i], gBuildingCost[i])
		menu_additem(menuid, menu)
	}

	formatex(menu, charsmax(menu), "上页")
	menu_setprop(menuid, MPROP_BACKNAME, menu)
	formatex(menu, charsmax(menu), "下页")
	menu_setprop(menuid, MPROP_NEXTNAME, menu)
	formatex(menu, charsmax(menu), "退出")
	menu_setprop(menuid, MPROP_EXITNAME, menu)
	menu_display(iPlayer, menuid)
}

public buildermenuhandle(id, menuid, key){
	menu_destroy(menuid)
	if(key < 0)
		return

	client_cmd(id, "weapon_knife")

	if(fm_get_user_money(id) >= gBuildingCost[key]){
		if(CreateBuilding(id, key)){
			client_print(id, print_center, "方法: R键旋转 | 左键确认 | 右键取消")
		}else{
			client_print(id, print_center, "不能重复选择!")
		}
	}else{
		client_print(id, print_chat, "* 金钱不够%d !", gBuildingCost[key])
	}
}

CreateBuilding(iPlayer, BuildingId)
{
	new iEntity = gIsBuilding[iPlayer]
	if(iEntity){
		if(pev(iEntity, PEV_BUILDINGID) != BuildingId){
			set_pev(iEntity, pev_targetname, gBuildingName[BuildingId])
			set_pev(iEntity, pev_modelindex, gBuildingModelIndex[BuildingId])
			set_pev(iEntity, pev_health, gBuildingHealth[BuildingId])
			set_pev(iEntity, pev_max_health, gBuildingHealth[BuildingId])
			set_pev(iEntity, PEV_BUILDINGID, BuildingId)
			return iEntity
		}else{
			return 0
		}
	}

	new Float:fOrigin[3]
	get_aim_origin_vector(iPlayer, 150.0, 0.0, 0.0, fOrigin)

	iEntity = engfunc(EngFunc_CreateNamedEntity, g_AllocString)
	set_pev(iEntity, pev_classname, "rg_building")
	set_pev(iEntity, pev_targetname, gBuildingName[BuildingId])
	set_pev(iEntity, pev_movetype, MOVETYPE_PUSHSTEP)
	set_pev(iEntity, pev_deadflag, DEAD_NO)
	set_pev(iEntity, pev_solid, SOLID_NOT)
	set_pev(iEntity, pev_modelindex, gBuildingModelIndex[BuildingId])
	set_pev(iEntity, pev_rendermode, kRenderTransColor)
	set_pev(iEntity, pev_renderfx, 0)
	set_pev(iEntity, pev_rendercolor, Float:{255.0,255.0,255.0})
	set_pev(iEntity, pev_renderamt, 130.0)
	set_pev(iEntity, pev_nextthink, get_gametime()+0.1)
	set_pev(iEntity, pev_gamestate, 1.0)
	set_pev(iEntity, PEV_BUILDINGID, BuildingId)
	set_pev(iEntity, PEV_BUILDER, iPlayer)
	set_pev(iEntity, pev_takedamage, DAMAGE_NO)
	set_pev(iEntity, PEV_MONSTER, NONPLAYER)
	set_pev(iEntity, pev_health, gBuildingHealth[BuildingId])
	set_pev(iEntity, pev_max_health, gBuildingHealth[BuildingId])
	engfunc(EngFunc_SetSize, iEntity, gBuildingMins[BuildingId], gBuildingMaxs[BuildingId])
	engfunc(EngFunc_SetOrigin, iEntity, fOrigin)
	gIsBuilding[iPlayer] = iEntity
	//engfunc(EngFunc_EmitSound, iEntity, CHAN_STATIC, SOUND_BUILDING_SELECT, 1.0, ATTN_NORM, 0, PITCH_NORM)
	return iEntity
}

moveBuilding(iPlayer, uc){
	new iEntity = gIsBuilding[iPlayer]
	if(!IsNonPlayer(iEntity)){
		gIsBuilding[iPlayer] = 0
		return
	}

	static Float:vOrigin[3], Float:fOrigin[3], Float:Origin[3], Float:fAngle[3]

	pev(iPlayer, pev_origin, vOrigin)
	get_aim_origin_vector(iPlayer, 150.0, 0.0, 0.0, fOrigin)
	fOrigin[2] = vOrigin[2]-(pev(iPlayer, pev_flags) & FL_DUCKING ? 15.0 : 26.0)
	pev(iEntity, pev_angles, fAngle)

	engfunc(EngFunc_SetOrigin, iEntity, fOrigin)
	engfunc(EngFunc_DropToFloor, iEntity)
	pev(iEntity, pev_origin, Origin)

	static iButton ; iButton = get_uc(uc, UC_Buttons)
	static iOldButton ; iOldButton = pev(iPlayer, pev_oldbuttons)
	if(iButton & IN_RELOAD && !(iOldButton & IN_RELOAD))
	{
		fAngle[1] += 90.0

		gRotation[iPlayer]++
		if(gRotation[iPlayer] > 3) gRotation[iPlayer] = 0
		
		set_pev(iEntity, pev_angles, fAngle)
		reset_box(iPlayer, iEntity, pev(iEntity, PEV_BUILDINGID))
	}
}

SetBuildingComplete(iEntity)
{
	new iPlayer = pev(iEntity, PEV_BUILDER)
	new BuildingId = pev(iEntity, PEV_BUILDINGID)
	new money = fm_get_user_money(iPlayer)
	if(money<gBuildingCost[BuildingId]){
		CancleBuilding(iPlayer)
		client_print(iPlayer, print_chat, "* 建造失败,金钱不够%d !", gBuildingCost[BuildingId])
		return
	}

	fm_set_user_money(iPlayer, money - gBuildingCost[BuildingId], 1)

	set_pev(iEntity, pev_renderfx, 0)
	set_pev(iEntity, pev_rendercolor, Float:{255.0, 255.0, 255.0})
	set_pev(iEntity, pev_rendermode, kRenderNormal)
	set_pev(iEntity, pev_renderamt, 16.0)
	set_pev(iEntity, pev_takedamage, DAMAGE_YES)
	set_pev(iEntity, pev_solid, SOLID_BBOX)

	reset_box(iPlayer, iEntity, BuildingId)
	gIsBuilding[iPlayer] = 0
	gRotation[iPlayer] = 0
	engfunc(EngFunc_EmitSound, iEntity, CHAN_STATIC, gSounds[3], 1.0, ATTN_NORM, 0, PITCH_NORM)
}

CancleBuilding(iPlayer, tips = 0)
{
	new ent = gIsBuilding[iPlayer]
	if(IsNonPlayer(ent)){
		if(tips){
			new BuildingId = pev(ent, PEV_BUILDINGID)
			client_print(iPlayer, print_center, "你取消了 %s 的建造！", gBuildingName[BuildingId])
		}
		engfunc(EngFunc_RemoveEntity, ent)
		gIsBuilding[iPlayer] = 0
		gRotation[iPlayer] = 0
	}
}

reset_box(iPlayer, iEntity, BuildingId){
	new Float:fMin[3], Float:fMax[3]
	if(gRotation[iPlayer] == 1 || gRotation[iPlayer] == 3)
	{
		fMin[0] = gBuildingMins[BuildingId][1]; fMin[1] = gBuildingMins[BuildingId][0]; fMin[2] = gBuildingMins[BuildingId][2]
		fMax[0] = gBuildingMaxs[BuildingId][1]; fMax[1] = gBuildingMaxs[BuildingId][0]; fMax[2] = gBuildingMaxs[BuildingId][2]
	}
	else
	{
		xs_vec_copy(gBuildingMins[BuildingId], fMin)
		xs_vec_copy(gBuildingMaxs[BuildingId], fMax)
	}
	engfunc(EngFunc_SetSize, iEntity, fMin, fMax)
}

// --------------------- events --------------------- 

public HAM_Knife_PrimaryAttack_Post(iWpEntity)
{
	new iPlayer = get_pdata_cbase(iWpEntity, 41, 4)
	if(!is_user_alive(iPlayer)) return

	new iEntity = gIsBuilding[iPlayer]
	if(!IsNonPlayer(iEntity)) return

	new Float:Origin[3], Float:fMin[3], Float:fMax[3]
	pev(iEntity, pev_mins, fMin)
	pev(iEntity, pev_maxs, fMax)

	pev(iEntity, pev_origin, Origin)
	if(!CheckStuck(Origin, fMin, fMax) || engfunc(EngFunc_PointContents, Origin) != CONTENTS_EMPTY || overlapInSphere(iEntity, Origin, fMin, fMax))
	{
		client_cmd(iPlayer, "spk %s", gSounds[2])
		client_print(iPlayer, print_center, "建造空间不足!")
		return
	}
	

	SetBuildingComplete(iEntity)

	set_pdata_float(iWpEntity, 46, 2.0, 4)
}

public HAM_Knife_SecondaryAttack_Post(iWpEntity)
{
	new iPlayer = get_pdata_cbase(iWpEntity, 41, 4)
	if(!is_user_alive(iPlayer)) return

	new iEntity = gIsBuilding[iPlayer]
	if(!IsNonPlayer(iEntity)) return

	CancleBuilding(iPlayer, 1)

	set_pdata_float(iWpEntity, 47, 2.0, 4)
}

public HAM_Knife_Holster_Post(iWpEntity)
{
	if(!pev_valid(iWpEntity))
		return
	
	new iPlayer = get_pdata_cbase(iWpEntity, 41, 4)
	if(!is_user_alive(iPlayer)) return

	CancleBuilding(iPlayer, 1)
}

