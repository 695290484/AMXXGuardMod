#include <amxmodx>
#include <fakemeta>
#include <hamsandwich>
#include <xs>
#include "rpg_guard.inc"

new gClass[] = "rpg_g_boomer"

new const gSounds[][] = {
	"rpg/guardmod/zombi_attack_1.wav",
	"rpg/guardmod/zombi_attack_2.wav",
	"rpg/guardmod/zombi_attack_3.wav",
	"rpg/boomer_death.wav"
}

new gMaxPlayers
public plugin_init()
{
	register_plugin("RPG:Guard - Zombie Level 5", "1.0", "zhiJiaN")
	gMaxPlayers = get_maxplayers()

	register_forward(FM_Think, "fw_Think_Post", 1)
}

new zb, animCode
new BoomerIndex, SkillSPR, SkillSPR2
public plugin_precache(){

	zb = rpg_precache_g("models/rpg/Dboomer.mdl", 3, 120.0, 4, 180.0, animCode)

	BoomerIndex = engfunc(EngFunc_PrecacheModel, "models/rpg/ef_boomer.mdl")
	SkillSPR = engfunc(EngFunc_PrecacheModel, "sprites/rpg/ef_boomer.spr")
	SkillSPR2 = engfunc(EngFunc_PrecacheModel, "sprites/rpg/ef_boomer_ex.spr")

	for(new i;i< sizeof gSounds;++i)
		precache_sound(gSounds[i])

}

new Float:Refresh
public rpg_fw_npc_refresh(level, numleft){
	static Float:gtime
	gtime = get_gametime()
	if(level == 5 && numleft){
		if(gtime >= Refresh){
			Refresh = gtime + 1.0

			rpg_create_g({0.0,0.0,0.0}, {0.0,0.0,0.0}, 5, gClass, "Boomer", 350.0, {-10.0,-10.0,-36.0}, {10.0,10.0,36.0}, zb)
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
		rpg_animation_g(ent, 114)

		engfunc(EngFunc_EmitSound, ent, CHAN_BODY, gSounds[3], 1.0, ATTN_NORM, 0, PITCH_NORM)

		new Float:fOrigin[3]
		pev(ent, pev_origin, fOrigin)
		BoomerSkill(ent, fOrigin)
	}
}

public rpg_fw_npctakedamage_post(iEntity, attacker, Float:damage, damagetype, headshot){
	if(!isEntByClass(iEntity, gClass))
		return

	if(pev(iEntity, pev_deadflag) == DEAD_NO){
		new curAnim = pev(iEntity, pev_sequence)
		if(curAnim != 84 && curAnim != 6){ 
			if(headshot)
				rpg_animation_g(iEntity, 110, _, _, 1)
			else
				rpg_animation_g(iEntity, 107, _, _, 1)

			rpg_set_animinterrupt_g(iEntity, get_gametime() + 0.04)
		}
	}
}

public rpg_fw_npccreate_post(ent){
	if(isEntByClass(ent, gClass)){
		setSpeedByDarkLevel(ent, rpg_get_darklevel())
		rpg_animation_g(ent, 1)
		SetMD_float(ent, md_attackradius, 88.0)
	}

}

public rpg_fw_npc_attack(ent, target){
	if(!isEntByClass(ent, gClass))
		return 0

	rpg_animation_g(ent, 84)
	rpg_delaydmg_g(ent, random_float(10.0, 15.0), 0.3, 90.0, DMG_FALL, 1)

	engfunc(EngFunc_EmitSound, ent, CHAN_BODY, gSounds[random_num(0,2)], 1.0, ATTN_NORM, 0, PITCH_NORM)
	set_pev(ent, pev_nextthink, get_gametime() + 1.5)
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
		rpg_animation_g(ent, 4, animCode)
	else if(speed > 50)
		rpg_animation_g(ent, 3, animCode)
	else
		rpg_animation_g(ent, 1)
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
		case 0: {set_pev(ent, pev_maxspeed, 90.0);}
		case 1: {set_pev(ent, pev_maxspeed, 110.0);}
		case 2: {set_pev(ent, pev_maxspeed, 140.0);}
		case 3: {set_pev(ent, pev_maxspeed, 160.0);}
		case 4: {set_pev(ent, pev_maxspeed, 180.0);}
		default:{set_pev(ent, pev_maxspeed, 220.0);}
	}
}

/* ============= 怪物技能 ============= */


BoomerSkill(iEntity, Float:origin[3])
{
	engfunc(EngFunc_MessageBegin, MSG_PVS, SVC_TEMPENTITY, origin, 0)
	write_byte(TE_SPRITE)
	engfunc(EngFunc_WriteCoord, origin[0])
	engfunc(EngFunc_WriteCoord, origin[1])
	engfunc(EngFunc_WriteCoord, origin[2])
	write_short(SkillSPR2)
	write_byte(8)
	write_byte(255)
	message_end()

	engfunc(EngFunc_MessageBegin, MSG_PVS, SVC_TEMPENTITY, origin, 0)
	write_byte(TE_SPRITE)
	engfunc(EngFunc_WriteCoord, origin[0])
	engfunc(EngFunc_WriteCoord, origin[1])
	engfunc(EngFunc_WriteCoord, origin[2]-13.0)
	write_short(SkillSPR)
	write_byte(11)
	write_byte(255)
	message_end()

	new Float:fCurTime
	global_get(glb_time, fCurTime)
	new iBoom = engfunc(EngFunc_CreateNamedEntity, engfunc(EngFunc_AllocString, "info_target"))
	set_pev(iBoom, pev_classname, "zbboom")
	set_pev(iBoom, pev_solid, SOLID_TRIGGER)
	set_pev(iBoom, pev_movetype, MOVETYPE_TOSS)
	set_pev(iBoom, pev_modelindex, BoomerIndex)
	set_pev(iBoom, pev_sequence, 0)
	set_pev(iBoom, pev_animtime, fCurTime)
	set_pev(iBoom, pev_frame, 0.0)
	set_pev(iBoom, pev_framerate, 1.0)
	set_pev(iBoom, pev_nextthink, fCurTime+1.5)
	engfunc(EngFunc_SetSize, iBoom, {-16.0, -16.0, -36.0}, {16.0, 16.0, 36.0})
	engfunc(EngFunc_SetOrigin, iBoom, origin)
	engfunc(EngFunc_SetOrigin, iEntity, Float:{-8000.0,-8000.0,-8000.0})

	new Float:origin2[3], Float:distance

	static i
	while((i = engfunc(EngFunc_FindEntityInSphere, i, origin, 120.0)) > 0)
	{
		if(i==iEntity || !pev_valid(i))
			continue

		 if(rpg_is_monster(i))
		 	continue

		if(i<gMaxPlayers && is_user_alive(i)){
			pev(i, pev_origin, origin2)
			distance = get_distance_f(origin, origin2)

			message_begin(MSG_ONE, get_user_msgid("ScreenShake"), {0,0,0}, i)
			write_short(1<<14)
			write_short(1<<14)
			write_short(1<<14)
			message_end()

			new Float:velocity[3]
			GetVelocityFromOrigin(origin2, origin, 900.0, velocity)
			set_pev(i, pev_velocity, velocity)
			ExecuteHamB(Ham_TakeDamage, i, iEntity, iEntity, 50.0 - 50.0/120.0*distance, DMG_FALL)
		}else if(rpg_is_nonplayer(i)){
			ExecuteHamB(Ham_TakeDamage, i, iEntity, iEntity, 50.0 - 50.0/120.0*distance, DMG_FALL)
		}
	}

}


public fw_Think_Post(iEntity)
{
	if(!pev_valid(iEntity))
		return

	static classname[33]
	pev(iEntity, pev_classname, classname, charsmax(classname))
	if(strcmp(classname, "zbboom"))
		return

	engfunc(EngFunc_RemoveEntity, iEntity)
}

stock GetVelocityFromOrigin(Float:origin1[3], Float:origin2[3], Float:speed, Float:velocity[3])
{
	xs_vec_sub(origin1, origin2, velocity)
	new Float:valve = get_distance_f(origin1, origin2)/speed
	
	if(valve <= 0.0)
		return
	
	xs_vec_div_scalar(velocity, valve, velocity)
}