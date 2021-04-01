#include <amxmodx>
#include <fakemeta>
#include <hamsandwich>
#include <xs>
#include "rpg_guard.inc"

new gClass[] = "rpg_g_skele"

// 怪物类型
get_skeletype(ent) return pev(ent, pev_fuser3)
set_skeletype(ent, iValue) set_pev(ent, pev_fuser3, float(iValue))
get_skelearmor(ent) return pev(ent, pev_fuser2)
set_skelearmor(ent, iValue) set_pev(ent, pev_fuser2, float(iValue))

new const gSounds[][] = {
	"rpg/guardmod/zombi_attack_1.wav",
	"rpg/guardmod/zombi_attack_2.wav",
	"rpg/guardmod/zombi_attack_3.wav",
	"rpg/guardmod/alien_die1.wav"
}

new gMaxPlayers
public plugin_init()
{
	register_plugin("RPG:Guard - Zombie Level 4", "1.0", "zhiJiaN")
	RegisterHam(Ham_Think, "info_target", "HAM_projectilesThink")
	gMaxPlayers = get_maxplayers()

	register_forward(FM_Touch, "FM_NpcTouch")
}

new zb, animCode
new ball
public plugin_precache(){

	zb = rpg_precache_g("models/rpg/skeleton_boss1.mdl", 13, 50.0, 26, 144.0, animCode)

	for(new i;i< sizeof gSounds;++i)
		precache_sound(gSounds[i])

	ball = precache_model("models/rpg/projectiles.mdl")
}

new Float:Refresh
public rpg_fw_npc_refresh(level, numleft){
	static Float:gtime
	gtime = get_gametime()
	if(level == 4 && numleft){
		if(gtime >= Refresh){
			Refresh = gtime + 1.0

			rpg_create_g({0.0,0.0,0.0}, {0.0,0.0,0.0}, 3, gClass, "亡灵战士", 300.0, {-10.0,-10.0,0.0}, {10.0,10.0,75.0}, zb)
		}
	}
}

// 行为

public rpg_fw_npc_move(ent){

}

public rpg_fw_npc_jump(ent){
	if(isEntByClass(ent, gClass)){
		rpg_animation_g(ent, 26)
	}
}

public rpg_fw_npckilled_post(ent, killer, headshot){
	if(isEntByClass(ent, gClass)){
		if(headshot)
			rpg_animation_g(ent, 18)
		else
			rpg_animation_g(ent, random_num(20,22))
		engfunc(EngFunc_EmitSound, ent, CHAN_BODY, gSounds[3], 1.0, ATTN_NORM, 0, PITCH_NORM)
	}
}

public rpg_fw_npctakedamage_post(iEntity, attacker, Float:damage, damagetype, headshot){
	if(!isEntByClass(iEntity, gClass))
		return

	if(pev(iEntity, pev_deadflag) == DEAD_NO){
		new curAnim = pev(iEntity, pev_sequence)
		if(curAnim != 25 && curAnim != 27 && curAnim != 10 && curAnim != 11 && curAnim != 12){ // 不在攻击的时候才播放受伤动画
			if(headshot)
				rpg_animation_g(iEntity, 14, _, _, 1)
			else
				rpg_animation_g(iEntity, 4, _, _, 1)

			rpg_set_animinterrupt_g(iEntity, get_gametime() + 0.04)
		}
	}
}

public rpg_fw_npccreate_post(ent){
	if(isEntByClass(ent, gClass)){
		setSpeedByDarkLevel(ent, rpg_get_darklevel())
		rpg_animation_g(ent, 0)

		new skin[2]
		skin[0] = random_num(0, 11)
		skin[1] = random_num(0,1)?random_num(0, 9):10
		if(skin[1] == 9) skin[1] = 8

		fn_set_pevbody(ent, skin)
		set_skelearmor(ent, skin[0])
		set_skeletype(ent, skin[1])
		if(skin[0] == 10 || skin[0] == 11){
			new Float:maxhp
			pev(ent, pev_max_health, maxhp)
			set_pev(ent, pev_health, maxhp*1.5)
			set_pev(ent, pev_max_health, maxhp*1.5)
		}

		new type = skin[1],Float:tmprange = 90.0
		switch(type){
			case 0: tmprange = 90.0
			case 1..6: tmprange = 100.0
			case 7: tmprange = 110.0
			case 8..11: tmprange = 550.0
		}
		SetMD_float(ent, md_attackradius, tmprange)
	}

}

public rpg_fw_npc_attack(ent, target){
	if(!isEntByClass(ent, gClass))
		return 0

	new armor = get_skelearmor(ent)
	new type = get_skeletype(ent)

	if(type != 8 && type != 10){
		new Float:atkRadius = GetMD_float(ent, md_attackradius)
		rpg_animation_g(ent, random_num(10,12), _, _, 1)
		rpg_delaydmg_g(ent, random_float(6.0, 8.0), 0.3, atkRadius, DMG_FALL, 1)
		set_pev(ent, pev_nextthink, get_gametime() + 1.2)
		set_attack_angles(ent, target)
		return 1
	}

	new skin[2]
	skin[0] = armor
	skin[1] = type
	if(type == 10){
		fn_set_pevbody(ent, skin)

		rpg_animation_g(ent, 27, _, _, 1)
		set_task(1.4, "task_arrowloose", ent + custom_task2)
		set_task(0.5, "task_arrowup", ent + custom_task3)
		set_task(0.1, "task_angleFix", ent + custom_task4, _, _, "a", 20)

		set_pev(ent, pev_nextthink, get_gametime() + 2.2)
		return 1
	}else if(type == 8){
		fn_set_pevbody(ent, skin)

		rpg_animation_g(ent, 12, _, _, 1)
		set_task(0.5, "task_poisonloose", ent + custom_task2)
		set_task(0.1, "task_angleFix", ent + custom_task4, _, _, "a", 10)
		set_pev(ent, pev_nextthink, get_gametime() + 1.4)
		return 1
	}
	
	//engfunc(EngFunc_EmitSound, ent, CHAN_BODY, gSounds[random_num(0,2)], 1.0, ATTN_NORM, 0, PITCH_NORM)
	set_pev(ent, pev_nextthink, get_gametime() + 1.0)
	return 1
}

public rpg_fw_npcthink_post(ent){
	if(!isEntByClass(ent, gClass))
		return

	new Float:velocity[3], Float:speed
	pev(ent, pev_velocity, velocity)
	velocity[2] = 0.0
	speed = vector_length(velocity)

	if(speed>140)
		rpg_animation_g(ent, 26, animCode)
	else if(speed > 50)
		rpg_animation_g(ent, 13, animCode)
	else
		rpg_animation_g(ent, 0)
}

public rpg_fw_dark_change(darkLevel){
	// 根据不同的暗度设置怪物移速
	new ent=-1
	while ((ent=engfunc(EngFunc_FindEntityByString,ent,"classname", gClass)))
	{
		if(pev_valid(ent))
		{
			setSpeedByDarkLevel(ent, darkLevel)
		}
	}
}

setSpeedByDarkLevel(ent, darkLv){
	switch(darkLv){
		case 0: {set_pev(ent, pev_maxspeed, 130.0);}
		case 1: {set_pev(ent, pev_maxspeed, 170.0);}
		case 2: {set_pev(ent, pev_maxspeed, 230.0);}
		case 3: {set_pev(ent, pev_maxspeed, 250.0);}
		case 4: {set_pev(ent, pev_maxspeed, 280.0);}
	}
}

#define MAX_GROUP_NUM 2
new g_byBodyGroupNum[MAX_GROUP_NUM]={12, 12}

stock fn_set_pevbody(ent,setBodyGroup[]){

	static temp_BodyGroup,temp_PevBody,temp_CurrentByte,temp_setBodyGroup[MAX_GROUP_NUM];
	for(temp_BodyGroup=temp_PevBody=0;temp_BodyGroup<MAX_GROUP_NUM;)
	{

		temp_setBodyGroup[temp_BodyGroup]=setBodyGroup[temp_BodyGroup];
		for(temp_CurrentByte=1;temp_CurrentByte<=temp_BodyGroup;)
			temp_setBodyGroup[temp_BodyGroup]*=g_byBodyGroupNum[temp_CurrentByte++-1];

		temp_PevBody+=temp_setBodyGroup[temp_BodyGroup++];
	}
	set_pev(ent,pev_body,temp_PevBody);

}

stock set_attack_angles(ent, enemy)
{
	new Float:o1[3], Float:o2[3]
	pev(ent, pev_origin, o1)
	pev(enemy, pev_origin, o2)
	xs_vec_sub(o2, o1, o1)
	o1[2] = 0.0

	new Float:an[3]
	vector_to_angle(o1, an)
	an[0] = an[2] = 0.0
	set_pev(ent, pev_angles, an)
}

/* ============= 怪物技能 ============= */


public task_angleFix(ent){
	ent -= custom_task4
	if(!pev_valid(ent)){
		remove_task(ent + custom_task4)
		return	
	}

	if(!isEntByClass(ent, gClass))
		return

	new deadflag = pev(ent, pev_deadflag)
	if(deadflag == DEAD_DYING || deadflag == DEAD_DEAD){
		remove_task(ent + custom_task4)
		return
	}

	new enemy = pev(ent, pev_enemy)
	if(pev_valid(enemy) && (rpg_is_nonplayer(enemy) || rpg_is_monster(enemy)))
		set_attack_angles(ent, enemy)
}

public task_arrowup(ent){
	ent -= custom_task3
	if(!pev_valid(ent)){
		return	
	}

	if(!isEntByClass(ent, gClass))
		return

	new deadflag = pev(ent, pev_deadflag)
	if(deadflag == DEAD_DYING || deadflag == DEAD_DEAD)
		return

	new armor = get_skelearmor(ent)
	new type = get_skeletype(ent)
	new enemy = pev(ent, pev_enemy)

	new skin[2]
	skin[0] = armor
	skin[1] = type + 1
	fn_set_pevbody(ent, skin)

	set_attack_angles(ent, enemy)
}


public task_arrowloose(ent){
	ent -= custom_task2
	if(!pev_valid(ent)){
		return	
	}

	if(!isEntByClass(ent, gClass))
		return

	new deadflag = pev(ent, pev_deadflag)
	if(deadflag == DEAD_DYING || deadflag == DEAD_DEAD)
		return

	new armor = get_skelearmor(ent)
	new type = get_skeletype(ent)
	new enemy = pev(ent, pev_enemy)

	new skin[2]
	skin[0] = armor
	skin[1] = type
	fn_set_pevbody(ent, skin)

	if(pev_valid(enemy) && pev(enemy, pev_deadflag)==DEAD_NO)
		set_attack_angles(ent, enemy)

	new Float:end[3]
	pev(enemy, pev_origin, end)
	if(rpg_is_nonplayer(enemy)) end[2] += 15.0
	Drop_projectiles(ent, 47, end, 1100.0)
}

public task_poisonloose(ent){
	ent -= custom_task2
	if(!pev_valid(ent)){
		return	
	}

	if(!isEntByClass(ent, gClass))
		return

	new deadflag = pev(ent, pev_deadflag)
	if(deadflag == DEAD_DYING || deadflag == DEAD_DEAD)
		return

	new armor = get_skelearmor(ent)
	new type = get_skeletype(ent)
	new enemy = pev(ent, pev_enemy)

	if(pev_valid(enemy) && pev(enemy, pev_deadflag)==DEAD_NO)
		set_attack_angles(ent, enemy)

	new Float:end[3]
	pev(enemy, pev_origin, end)
	if(rpg_is_nonplayer(enemy)) end[2] += 15.0
	Drop_projectiles(ent, 0, end, 800.0, 1)
}

Drop_projectiles(id, ibody, Float:end[3], Float:power, type=0){
	new Float:fOrigin[3], Float:fVforward[3], Float:fAngles[3]
	pev(id, pev_origin, fOrigin)
	pev(id, pev_angles, fAngles)
	engfunc(EngFunc_MakeVectors, fAngles)
	global_get(glb_v_forward, fAngles)
	xs_vec_mul_scalar(fAngles, 30.0, fVforward)
	fAngles[0] = fAngles[1] = 0.0
	xs_vec_add(fOrigin, fVforward, fOrigin)
	new iEntity = engfunc(EngFunc_CreateNamedEntity, engfunc(EngFunc_AllocString, "info_target"))
	set_pev(iEntity,pev_movetype,MOVETYPE_FLY)
	set_pev(iEntity, pev_classname, "rpg_proj2")
	set_pev(iEntity, pev_angles, fAngles)
	set_pev(iEntity, pev_owner, id)
	set_pev(iEntity, pev_iuser4, type)
	fOrigin[2] += 60.0
	engfunc(EngFunc_SetOrigin, iEntity, fOrigin)
	set_pev(iEntity, pev_modelindex, ball)
	engfunc(EngFunc_SetSize, iEntity, {-1.0,-1.0,-1.0}, {1.0,1.0,1.0})
	set_pev(iEntity,pev_solid,SOLID_BBOX)

	set_pev(iEntity, pev_body, ibody)
	xs_vec_sub(end, fOrigin, end)
	xs_vec_normalize(end, end)
	xs_vec_mul_scalar(end, power, end)
	set_pev(iEntity, pev_velocity, end)
	
	vector_to_angle(end, fAngles)
	set_pev(iEntity, pev_angles, fAngles)

	return iEntity
}

public FM_NpcTouch(ent, id){ // id = 箭或法球
	if(id < gMaxPlayers || !pev_valid(id))
		return

	if(!isEntByClass(id, "rpg_proj2"))
		return

	new owner = pev(id, pev_owner)

	new type = pev(id, pev_iuser4)
	if(!type){
		if(ent<1){
			set_pev(id, pev_movetype, MOVETYPE_NONE)
		}else if((ent<gMaxPlayers || rpg_is_nonplayer(ent)) && pev(ent, pev_deadflag)==DEAD_NO && pev(id, pev_movetype) != MOVETYPE_NONE){
			ExecuteHamB(Ham_TakeDamage, ent, id, owner, 30.0, DMG_FALL)
			set_pev(id, pev_flags, FL_KILLME)
			return
		}else{
			set_pev(id, pev_flags, FL_KILLME)
			return
		}
		set_pev(id, pev_nextthink, get_gametime()+3.0)
	}
	else if(type == 1){
		new Float:endorigini[3]
		pev(id, pev_origin, endorigini)
		message_begin(MSG_BROADCAST, SVC_TEMPENTITY)
		write_byte(TE_TAREXPLOSION) //4
		engfunc(EngFunc_WriteCoord, endorigini[0])
		engfunc(EngFunc_WriteCoord, endorigini[1])
		engfunc(EngFunc_WriteCoord, endorigini[2])
		message_end()

		new others = -1
		while((others = engfunc(EngFunc_FindEntityInSphere, others, endorigini, 100.0)) > 0){
			if(ent == others || pev(others, pev_deadflag)!=DEAD_NO || (!rpg_is_nonplayer(others) && others>gMaxPlayers))
				continue
			
			ExecuteHamB(Ham_TakeDamage, others, id, owner, 15.0, DMG_FALL)
		}

		set_pev(id, pev_flags, FL_KILLME)
	}
}

public HAM_projectilesThink(iEntity){
	if(!pev_valid(iEntity)) return HAM_IGNORED

	if(isEntByClass(iEntity, "rpg_proj2"))
		set_pev(iEntity, pev_flags, FL_KILLME)

	return HAM_IGNORED
}


