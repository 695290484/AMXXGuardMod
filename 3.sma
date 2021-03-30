#include <amxmodx>
#include <fakemeta>
#include "rpg_guard.inc"

new gClass[] = "rpg_g_alien"
new gClass2[] = "rpg_g_skboss"

new Float:gAttackSpeed2 // 攻击速度

#define pev_nextskill pev_fuser3 // 怪物技能CD

new const gSounds[][] = {
	"rpg/guardmod/zombi_attack_1.wav",
	"rpg/guardmod/zombi_attack_2.wav",
	"rpg/guardmod/zombi_attack_3.wav",
	"rpg/guardmod/salien_die1.wav"
}

public plugin_init()
{
	register_plugin("RPG:Guard - Zombie Level 3", "1.0", "zhiJiaN")
}

new zb, animCode, zb2, animCode2
public plugin_precache(){

	zb = rpg_precache_g("models/rpg/Alien.mdl", -1, 0.0, 1, 144.0, animCode)
	zb2 = rpg_precache_g("models/rpg/skeleboss.mdl", 1, 72.0, 2, 144.0, animCode2)

	for(new i;i< sizeof gSounds;++i)
		precache_sound(gSounds[i])
}

new Float:Refresh
public rpg_fw_npc_refresh(level, numleft){
	static Float:gtime
	gtime = get_gametime()
	if(level == 3 && numleft){
		if(gtime >= Refresh){
			Refresh = gtime + 1.0
			
			if(random_num(0, 4))
				rpg_create_g({0.0,0.0,0.0}, {0.0,0.0,0.0}, 3, gClass2, "亡灵骑士", 300.0, {-16.0,-16.0,-36.0}, {16.0,16.0,36.0}, zb2)
			else
				rpg_create_g({0.0,0.0,0.0}, {0.0,0.0,0.0}, 5, gClass, "小型异形", 500.0, {-10.0,-10.0,0.0}, {10.0,10.0,72.0}, zb)
		}
	}
}

// 行为

public rpg_fw_npc_move(ent){

}

public rpg_fw_npc_jump(ent){
	if(isEntByClass(ent, gClass)){
		rpg_animation_g(ent, 1)
	}
}

public rpg_fw_npckilled_post(ent, killer, headshot){
	if(isEntByClass(ent, gClass)){
		rpg_animation_g(ent, 7)
		engfunc(EngFunc_EmitSound, ent, CHAN_BODY, gSounds[3], 1.0, ATTN_NORM, 0, PITCH_NORM)
	}
}

public rpg_fw_npctakedamage_post(iEntity, attacker, Float:damage, damagetype, headshot){
	if(!isEntByClass(iEntity, gClass))
		return

	if(pev(iEntity, pev_deadflag) == DEAD_NO){
		new curAnim = pev(iEntity, pev_sequence)
		if(curAnim != 2 && curAnim != 3 && curAnim != 4){ // 不在攻击的时候才播放受伤动画
			if(headshot)
				rpg_animation_g(iEntity, 5, _, _, 1)
			else
				rpg_animation_g(iEntity, 6, _, _, 1)

			if(random_num(0,3)){
				set_pdata_float(iEntity, m_flVelocityModifier, 0.7)
				rpg_set_animinterrupt_g(iEntity, get_gametime() + 0.5)
			}else
				rpg_set_animinterrupt_g(iEntity, get_gametime() + 0.02)
		}
	}
}

public rpg_fw_npccreate_post(ent){
	if(isEntByClass(ent, gClass)){
		setSpeedByDarkLevel(ent, rpg_get_darklevel())
		rpg_animation_g(ent, 0)
		SetMD_float(ent, md_attackradius, 90.0)
	}

}

public rpg_fw_npc_attack(ent, target){
	if(!isEntByClass(ent, gClass))
		return 0

	rpg_animation_g(ent, random_num(2,4))
	rpg_delaydmg_g(ent, random_float(10.0, 15.0), 0.3, 90.0, DMG_FALL, 1)

	engfunc(EngFunc_EmitSound, ent, CHAN_BODY, gSounds[random_num(0,2)], 1.0, ATTN_NORM, 0, PITCH_NORM)
	set_pev(ent, pev_nextthink, get_gametime() + gAttackSpeed2)
	return 1
}

public rpg_fw_npcthink_post(ent){
	if(!isEntByClass(ent, gClass))
		return

	new Float:velocity[3], Float:speed
	pev(ent, pev_velocity, velocity)
	velocity[2] = 0.0
	speed = vector_length(velocity)

	if(speed > 50)
		rpg_animation_g(ent, 1, animCode)
	else
		rpg_animation_g(ent, 0)

	skill(ent)
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
		case 0: {set_pev(ent, pev_maxspeed, 170.0); gAttackSpeed2 = 2.2;}
		case 1: {set_pev(ent, pev_maxspeed, 200.0); gAttackSpeed2 = 2.05;}
		case 2: {set_pev(ent, pev_maxspeed, 230.0); gAttackSpeed2 = 1.85;}
		case 3: {set_pev(ent, pev_maxspeed, 250.0); gAttackSpeed2 = 1.7;}
		case 4: {set_pev(ent, pev_maxspeed, 280.0); gAttackSpeed2 = 1.5;}
	}
}

/* ============= 怪物技能 ============= */

skill(ent){
	new Float:fCurTime
	global_get(glb_time, fCurTime)

	new enemy = pev(ent, pev_enemy)
	if(!enemy || !is_user_alive(enemy)){
		set_pev(ent, pev_nextskill, fCurTime + random_float(15.0, 19.0))
		return
	}

	new Float:nextskilltime
	pev(ent, pev_nextskill, nextskilltime)

	if(nextskilltime >= fCurTime) return
	if(random_num(0,1)) ts(ent)
	set_pev(ent, pev_nextskill, fCurTime + random_float(15.0, 19.0))
}

ts(ent){
	util_Rendering(ent, kRenderFxHologram, 0, 0, 0, kRenderTransTexture, 160)
	set_task(5.2, "task_stopts", ent)
}

public task_stopts(ent){
	if(pev(ent, pev_deadflag)!=DEAD_NO) return

	if(!isEntByClass(ent, gClass)) return

	util_Rendering(ent)
}

