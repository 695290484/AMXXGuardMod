#include <amxmodx>
#include <fakemeta>
#include <hamsandwich>
#include <xs>
#include "rpg_guard.inc"

new gClass[] = "rpg_g_clown"

new const gSounds[][] = {
	"rpg/guardmod/laugh1.wav",
	"rpg/guardmod/laugh2.wav",
	"rpg/guardmod/laugh3.wav",
	"rpg/guardmod/laugh_box.wav"
}

new gMaxPlayers
public plugin_init()
{
	register_plugin("RPG:Guard - Zombie Level 6", "1.0", "zhiJiaN")
	gMaxPlayers = get_maxplayers()

	register_forward(FM_Think, "fw_Think_Post", 1)
}

new zb, zb2, animCode, bettyspr3
public plugin_precache(){

	zb = rpg_precache_g("models/rpg/clown.mdl", 2, 240.0, 3, 360.0, animCode)
	zb2 = engfunc(EngFunc_PrecacheModel, "models/rpg/clownbox.mdl")

	for(new i;i< sizeof gSounds;++i)
		precache_sound(gSounds[i])

	bettyspr3 = precache_model("sprites/rpg/bettyspr3.spr")
}

new Float:Refresh
public rpg_fw_npc_refresh(level, numleft){
	static Float:gtime
	gtime = get_gametime()
	if(level == 6 && numleft){
		if(gtime >= Refresh){
			Refresh = gtime + 1.0

			rpg_create_g({0.0,0.0,0.0}, {0.0,0.0,0.0}, 15, gClass, "Clown", 900.0, {-10.0,-10.0,-36.0}, {10.0,10.0,36.0}, zb)
		}
	}
}

// 行为

public rpg_fw_npc_move(ent){

}

public rpg_fw_npc_jump(ent){
	if(isEntByClass(ent, gClass)){
		rpg_animation_g(ent, 4)
	}
}

public rpg_fw_npckilled_post(ent, killer, headshot){
	if(isEntByClass(ent, gClass)){
		rpg_animation_g(ent, 7)

		engfunc(EngFunc_EmitSound, ent, CHAN_BODY, gSounds[random_num(0,2)], 1.0, ATTN_NORM, 0, PITCH_NORM)

		new Float:fOrigin[3]
		pev(ent, pev_origin, fOrigin)
		ClownKilled(ent, fOrigin)
	}
}

public rpg_fw_npctakedamage_post(iEntity, attacker, Float:damage, damagetype, headshot){
	if(!isEntByClass(iEntity, gClass))
		return

	if(pev(iEntity, pev_deadflag) == DEAD_NO){
		new curAnim = pev(iEntity, pev_sequence)

		if(headshot)
			rpg_animation_g(iEntity, 6, _, _, 1)
		else
			rpg_animation_g(iEntity, 5, _, _, 1)

		rpg_set_animinterrupt_g(iEntity, get_gametime() + 0.04)

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


	new Float:fOrigin[3]
	pev(ent, pev_origin, fOrigin)
	ClownKilled(ent, fOrigin, false)

	rpg_animation_g(ent, 7)
	engfunc(EngFunc_EmitSound, ent, CHAN_BODY, gSounds[random_num(0,2)], 1.0, ATTN_NORM, 0, PITCH_NORM)

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
		rpg_animation_g(ent, 3, animCode)
	else if(speed > 50)
		rpg_animation_g(ent, 2, animCode)
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
		case 4: {set_pev(ent, pev_maxspeed, 210.0);}
		default:{set_pev(ent, pev_maxspeed, 260.0);}
	}
}

/* ============= 怪物技能 ============= */


ClownKilled(iEntity, Float:origin[3], bool:killed=true)
{
	if(!killed){
		set_pev(iEntity, pev_deadflag, DEAD_DYING)
		set_pev(iEntity, pev_solid, SOLID_NOT)
		set_pev(iEntity, pev_movetype, MOVETYPE_NONE)
	}

	static TE_FLAG
	TE_FLAG |= TE_EXPLFLAG_NODLIGHTS
	TE_FLAG |= TE_EXPLFLAG_NOPARTICLES

	engfunc(EngFunc_MessageBegin, MSG_PAS, SVC_TEMPENTITY, origin, 0)
	write_byte(TE_EXPLOSION)
	engfunc(EngFunc_WriteCoord, origin[0])
	engfunc(EngFunc_WriteCoord, origin[1])
	engfunc(EngFunc_WriteCoord, origin[2])
	write_short(bettyspr3)
	write_byte(20)
	write_byte(24) 
	write_byte(TE_FLAG)
	message_end()


	new Float:fCurTime
	global_get(glb_time, fCurTime)
	new iBoom = engfunc(EngFunc_CreateNamedEntity, engfunc(EngFunc_AllocString, "info_target"))
	set_pev(iBoom, pev_classname, "clownbox")
	set_pev(iBoom, pev_solid, SOLID_NOT)
	set_pev(iBoom, pev_movetype, MOVETYPE_TOSS)
	set_pev(iBoom, pev_modelindex, zb2)
	set_pev(iBoom, pev_nextthink, fCurTime+1.5)
	set_pev(iBoom, pev_fuser3, fCurTime+20.0)
	engfunc(EngFunc_SetSize, iBoom, {-5.0, -5.0, 0.0}, {5.0, 5.0, 15.0})
	engfunc(EngFunc_SetOrigin, iBoom, origin)
	set_pev(iBoom, pev_velocity, Float:{0.0,0.0,0.001})
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
			ExecuteHamB(Ham_TakeDamage, i, iEntity, iEntity, 80.0 - 80.0/120.0*distance, DMG_FALL)
		}else if(rpg_is_nonplayer(i)){
			ExecuteHamB(Ham_TakeDamage, i, iEntity, iEntity, 80.0 - 80.0/120.0*distance, DMG_FALL)
		}
	}

}


public fw_Think_Post(iEntity)
{
	if(!pev_valid(iEntity))
		return

	static classname[33]
	pev(iEntity, pev_classname, classname, charsmax(classname))
	if(strcmp(classname, "clownbox"))
		return

	new Float:fCurTime, Float:fuser3
	global_get(glb_time, fCurTime)
	pev(iEntity, pev_fuser3, fuser3)
	if(fCurTime > fuser3){
		engfunc(EngFunc_RemoveEntity, iEntity)
		return
	}

	if(!pev(iEntity, pev_effects)){
		set_pev(iEntity, pev_nextthink, fCurTime+1.0)
		set_pev(iEntity, pev_effects, EF_NODRAW)
		return
	}

	new Float:o[3], Float:origin2[3], Float:distance
	pev(iEntity, pev_origin, o)
	new i, e
	while((i = engfunc(EngFunc_FindEntityInSphere, i, o, 120.0)) > 0){
		if(i==iEntity || !pev_valid(i))
			continue

		 if(rpg_is_monster(i))
		 	continue

		if(i<gMaxPlayers && is_user_alive(i)){
			pev(i, pev_origin, origin2)
			distance = get_distance_f(o, origin2)

			message_begin(MSG_ONE, get_user_msgid("ScreenShake"), {0,0,0}, i)
			write_short(1<<14)
			write_short(1<<14)
			write_short(1<<14)
			message_end()

			new Float:velocity[3]
			GetVelocityFromOrigin(origin2, o, 900.0, velocity)
			set_pev(i, pev_velocity, velocity)
			ExecuteHamB(Ham_TakeDamage, i, iEntity, iEntity, 40.0 - 40.0/120.0*distance, DMG_BURN)

			e = 1

			ScreenFadeColor(i, 255, 25, 25, 85, 0.5)
		}
	}
	if(e){
		set_pev(iEntity, pev_effects, 0)
		set_pev(iEntity, pev_sequence, 0)
		set_pev(iEntity, pev_animtime, fCurTime)
		set_pev(iEntity, pev_frame, 0.0)
		set_pev(iEntity, pev_framerate, 1.0)
		o[2] += 5.0
		light(o)

		set_pev(iEntity, pev_nextthink, fCurTime+3.1)
		set_pev(iEntity, pev_fuser3, fCurTime+3.0)

		engfunc(EngFunc_EmitSound, iEntity, CHAN_AUTO, gSounds[3], 1.0, ATTN_STATIC, 0, PITCH_NORM)
		return
	}

	set_pev(iEntity, pev_nextthink, fCurTime+0.1)
}

stock GetVelocityFromOrigin(Float:origin1[3], Float:origin2[3], Float:speed, Float:velocity[3])
{
	xs_vec_sub(origin1, origin2, velocity)
	new Float:valve = get_distance_f(origin1, origin2)/speed
	
	if(valve <= 0.0)
		return
	
	xs_vec_div_scalar(velocity, valve, velocity)
}

light(Float:Origin[3]){

	message_begin(MSG_BROADCAST,SVC_TEMPENTITY);
	write_byte(TE_DLIGHT);
	engfunc(EngFunc_WriteCoord, Origin[0]); // x
	engfunc(EngFunc_WriteCoord, Origin[1]); // y
	engfunc(EngFunc_WriteCoord, Origin[2]); // z
	write_byte(7); // radius
	write_byte(255); // r
	write_byte(25); // g
	write_byte(25); // b
	write_byte(15); // life <<<<<<<<
	write_byte(2); // decay rate
	message_end();
}

ScreenFadeColor(id, r, g, b, alp, Float:fTime=1.0)
{
	new msgScreenFade = get_user_msgid("ScreenFade")
	new Float:amount = 4096.0*fTime
	new iShort = floatround(amount)
	message_begin(MSG_ONE, msgScreenFade, {0, 0, 0}, id);
	write_short(iShort);
	write_short(iShort);
	write_short(1<<12); 
	write_byte(r);
	write_byte(g); 
	write_byte(b);
	write_byte(alp); 
	message_end();

	new maxplayers = get_maxplayers()
	for(new i=1;i<=maxplayers;++i)
	{
		if(is_user_alive(i)) continue
		if(!is_user_connected(i) || pev(i, pev_iuser2) != id) continue

		message_begin(MSG_ONE, msgScreenFade, {0, 0, 0}, i);
		write_short(iShort);
		write_short(iShort);
		write_short(1<<12); 
		write_byte(r);
		write_byte(g); 
		write_byte(b);
		write_byte(alp); 
		message_end();
	}
}
