#include <amxmodx>
#include <fakemeta>
#include "rpg_guard.inc"

new gClass[] = "rpg_g_abm"

new Float:gAttackSpeed1 // 普通僵尸攻击速度

new const gSounds[][] = {
	"rpg/guardmod/zombi_attack_1.wav",
	"rpg/guardmod/zombi_attack_2.wav",
	"rpg/guardmod/zombi_attack_3.wav",
	"rpg/guardmod/zbs_death_1.wav"
}

public plugin_init()
{
	register_plugin("RPG:Guard - Zombie Level 1", "1.0", "zhiJiaN")
}

new zb, animCode
public plugin_precache(){
	zb = rpg_precache_g("models/rpg/zombi_host_f3.mdl", 13, 72.0, 14, 657.0, animCode)

	for(new i;i< sizeof gSounds;++i)
		precache_sound(gSounds[i])
}

new Float:Refresh
public rpg_fw_npc_refresh(level, numleft){
	static Float:gtime
	gtime = get_gametime()
	if(level == 1 && numleft){
		if(gtime >= Refresh){
			Refresh = gtime + 1.0
			rpg_create_g({0.0,0.0,0.0}, {0.0,0.0,0.0}, 1, gClass, "普通僵尸", 100.0, {-16.0,-16.0,-36.0}, {16.0,16.0,36.0}, zb)
		}
	}
}

// 行为

public rpg_fw_npc_move(ent){

}

public rpg_fw_npc_jump(ent){
	if(!isEntByClass(ent, gClass))
		return

	rpg_animation_g(ent, 15)
}

public rpg_fw_npckilled_post(ent, killer, headshot){
	if(!isEntByClass(ent, gClass))
		return

	if(headshot)
		rpg_animation_g(ent, 5)
	else{
		new anim[] = {2,3,4,6,7,8,9,10,11}
		rpg_animation_g(ent, anim[random_num(0,charsmax(anim))])
	}
	engfunc(EngFunc_EmitSound, ent, CHAN_BODY, gSounds[3], 1.0, ATTN_NORM, 0, PITCH_NORM)
}

public rpg_fw_npctakedamage_post(iEntity, attacker, Float:damage, damagetype, headshot){
	if(!isEntByClass(iEntity, gClass))
		return

	if(pev(iEntity, pev_deadflag) == DEAD_NO){
		new curAnim = pev(iEntity, pev_sequence)
		if(curAnim != 22 && curAnim != 23){ // 不在攻击的时候才播放受伤动画
			if(headshot)
				rpg_animation_g(iEntity, 1, _, _, 1)
			else
				rpg_animation_g(iEntity, 0, _, _, 1)

			rpg_set_animinterrupt_g(iEntity, get_gametime() + 0.02)
		}
	}
}

public rpg_fw_npccreate_post(ent){
	if(!isEntByClass(ent, gClass))
		return

	setSpeedByDarkLevel(ent, rpg_get_darklevel())		// 最大移速
	rpg_animation_g(ent, 12)			// 静止动作
	SetMD_float(ent, md_attackradius, 80.0)		// 普攻范围
}

public rpg_fw_npc_attack(ent, target){
	if(!isEntByClass(ent, gClass))
		return 0

	new Float:velocity[3], Float:speed
	pev(ent, pev_velocity, velocity)
	velocity[2] = 0.0
	speed = vector_length(velocity)

	if(speed>140){
		rpg_animation_g(ent, 23)
		rpg_delaydmg_g(ent, 5.0, 0.2, 100.0, DMG_FALL, 1)
	}
	else{
		rpg_animation_g(ent, 22)
		rpg_delaydmg_g(ent, 3.0, 0.2, 100.0, DMG_FALL, 1)
	}
	engfunc(EngFunc_EmitSound, ent, CHAN_BODY, gSounds[random_num(0,2)], 1.0, ATTN_NORM, 0, PITCH_NORM)
	set_pev(ent, pev_nextthink, get_gametime() + gAttackSpeed1)
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
		rpg_animation_g(ent, 14, animCode)
	else if(speed > 50)
		rpg_animation_g(ent, 13, animCode)
	else
		rpg_animation_g(ent, 12)
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
		case 0: {set_pev(ent, pev_maxspeed, 110.0); gAttackSpeed1 = 2.2;}
		case 1: {set_pev(ent, pev_maxspeed, 150.0); gAttackSpeed1 = 2.05;}
		case 2: {set_pev(ent, pev_maxspeed, 210.0); gAttackSpeed1 = 1.85;}
		case 3: {set_pev(ent, pev_maxspeed, 230.0); gAttackSpeed1 = 1.7;}
		case 4: {set_pev(ent, pev_maxspeed, 260.0); gAttackSpeed1 = 1.5;}
	}
}
