#include <amxmodx>
#include <fakemeta>
#include "rpg_guard.inc"

new gClass[] = "rpg_g_abm"
new gClass2[] = "rpg_g_skboss"

new Float:gAttackSpeed2 // 攻击速度

new const gSounds[][] = {
	"rpg/guardmod/zombi_attack_1.wav",
	"rpg/guardmod/zombi_attack_2.wav",
	"rpg/guardmod/zombi_attack_3.wav",
	"rpg/guardmod/zbs_death_1.wav",
	"rpg/skboss_die1.wav"
}

public plugin_init()
{
	register_plugin("RPG:Guard - Zombie Level 2", "1.0", "zhiJiaN")
}

new zb, animCode, zb2, animCode2
public plugin_precache(){

	zb = rpg_precache_g("models/rpg/zombi_host_f3.mdl", 13, 72.0, 14, 657.0, animCode)
	zb2 = rpg_precache_g("models/rpg/skeleboss.mdl", 1, 72.0, 2, 144.0, animCode2)

	for(new i;i< sizeof gSounds;++i)
		precache_sound(gSounds[i])
}

new Float:Refresh
public rpg_fw_npc_refresh(level, numleft){
	static Float:gtime
	gtime = get_gametime()
	if(level == 2 && numleft){
		if(gtime >= Refresh){
			Refresh = gtime + 1.0
			
			if(!random_num(0, 4))
				rpg_create_g({0.0,0.0,0.0}, {0.0,0.0,0.0}, 3, gClass2, "亡灵骑士", 300.0, {-16.0,-16.0,-36.0}, {16.0,16.0,36.0}, zb2)
			else
				rpg_create_g({0.0,0.0,0.0}, {0.0,0.0,0.0}, 1, gClass, "普通僵尸", 100.0, {-16.0,-16.0,-36.0}, {16.0,16.0,36.0}, zb)
		}
	}
}

// 行为

public rpg_fw_npc_move(ent){

}

public rpg_fw_npc_jump(ent){
	if(isEntByClass(ent, gClass2)){
		rpg_animation_g(ent, 3)
	}
}

public rpg_fw_npckilled_post(ent, killer, headshot){
	if(isEntByClass(ent, gClass2)){
		if(headshot)
			rpg_animation_g(ent, 7)
		else{
			rpg_animation_g(ent, random_num(8,14))
		}
		engfunc(EngFunc_EmitSound, ent, CHAN_BODY, gSounds[4], 1.0, ATTN_NORM, 0, PITCH_NORM)
	}
}

public rpg_fw_npccreate_post(ent){
	if(isEntByClass(ent, gClass2)){
		setSpeedByDarkLevel(ent, rpg_get_darklevel())
		rpg_animation_g(ent, 0)
		SetMD_float(ent, md_attackradius, 100.0)
	}

}

public rpg_fw_npc_attack(ent, target){
	if(!isEntByClass(ent, gClass2))
		return 0

	rpg_animation_g(ent, 6)
	rpg_delaydmg_g(ent, random_float(8.0, 12.0), 0.42, 100.0, DMG_FALL, 1)

	engfunc(EngFunc_EmitSound, ent, CHAN_BODY, gSounds[random_num(0,2)], 1.0, ATTN_NORM, 0, PITCH_NORM)
	set_pev(ent, pev_nextthink, get_gametime() + gAttackSpeed2)
	return 1
}

public rpg_fw_npcthink_post(ent){
	if(!isEntByClass(ent, gClass2))
		return

	new Float:velocity[3], Float:speed
	pev(ent, pev_velocity, velocity)
	velocity[2] = 0.0
	speed = vector_length(velocity)

	if(speed>110)
		rpg_animation_g(ent, 2, animCode)
	else if(speed > 50)
		rpg_animation_g(ent, 1, animCode)
	else
		rpg_animation_g(ent, 0)
}

public rpg_fw_dark_change(darkLevel){
	// 根据不同的暗度设置怪物移速
	new ent=-1
	while ((ent=engfunc(EngFunc_FindEntityByString,ent,"classname", gClass2)))
	{
		if(pev_valid(ent))
		{
			setSpeedByDarkLevel(ent, darkLevel)
		}
	}
}

setSpeedByDarkLevel(ent, darkLv){
	switch(darkLv){
		case 0: {set_pev(ent, pev_maxspeed, 100.0); gAttackSpeed2 = 2.2;}
		case 1: {set_pev(ent, pev_maxspeed, 130.0); gAttackSpeed2 = 2.05;}
		case 2: {set_pev(ent, pev_maxspeed, 160.0); gAttackSpeed2 = 1.85;}
		case 3: {set_pev(ent, pev_maxspeed, 200.0); gAttackSpeed2 = 1.7;}
		case 4: {set_pev(ent, pev_maxspeed, 250.0); gAttackSpeed2 = 1.5;}
	}
}
