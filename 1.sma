#include <amxmodx>
#include <fakemeta>
#include "rpg_guard.inc"
//#include "navmesh.inc"

new Float:origin2[33][3]
public plugin_init()
{
	register_clcmd("say go", "go")
	register_clcmd("say zb", "create")

	register_srvcmd("test", "test")
}

new zb, animCode
public plugin_precache(){
	zb = rpg_precache_g("models/zombi_host_f3.mdl", 13, 72.0, 14, 657.0, animCode)
}

new Float:Refresh
public rpg_fw_npc_refresh(level, numleft){
	static Float:gtime
	gtime = get_gametime()
	if(level == 0 && numleft){
		if(gtime >= Refresh){
			Refresh = gtime + 1.0
			rpg_create_g({0.0,0.0,0.0}, {0.0,0.0,0.0}, 1, "rpg_g_abm", "普通僵尸", 100.0, {-16.0,-16.0,-36.0}, {16.0,16.0,36.0}, zb)
		}
	}
}


public test(){
	//new Float:goal[3]
	//new index = navmesh_getRandomAreaPos(origin2[1], 100.0, 4000.0, goal)
	//server_print("target:%d | %f,%f,%f", index, goal[0], goal[1], goal[2])
	new ent = rpg_create_g({0.0,0.0,0.0}, {0.0,0.0,0.0}, 1, "rpg_g_abm", "普通僵尸", 100.0, {-16.0,-16.0,-36.0}, {16.0,16.0,36.0}, zb)
	new Float:goal[3];pev(ent, pev_origin, goal)
	server_print("target:%d | %f,%f,%f", ent, goal[0], goal[1], goal[2])
}


public go(id){
	pev(id, pev_origin, origin2[id])
}

public create(id){
	new ent = rpg_create_g(origin2[id], {0.0,0.0,0.0}, 1, "rpg_g_abm", "普通僵尸", 100.0, {-16.0,-16.0,-36.0}, {16.0,16.0,36.0}, zb)
	if(ent){
		new Float:goal[3];pev(ent, pev_origin, goal)
		server_print("target:%d | %f,%f,%f", ent, goal[0], goal[1], goal[2])
	}
}

stock findRandomEnemy(){
	new players[32], count
	for(new id=1;id<=get_maxplayers();++id)
	{
		if(is_user_alive(id)){
			players[count] = id
			count++
		}
	}
	return count>0?players[random_num(0, count-1)]:0
}

// 行为

public rpg_fw_npc_move(ent){

}

public rpg_fw_npc_jump(ent){
	rpg_animation_g(ent, 15)
}

public rpg_fw_npckilled_post(ent, killer, headshot){
	if(headshot)
		rpg_animation_g(ent, 5)
	else{
		new anim[] = {2,3,4,6,7,8,9,10,11}
		rpg_animation_g(ent, anim[random_num(0,charsmax(anim))])
	}
}

public rpg_fw_npctakedamage_post(iEntity, attacker, Float:damage, damagetype, headshot){
	if(pev(iEntity, pev_deadflag) == DEAD_NO){
		if(headshot)
			rpg_animation_g(iEntity, 1, _, _, 1)
		else
			rpg_animation_g(iEntity, 0, _, _, 1)

		rpg_set_animinterrupt_g(iEntity, get_gametime() + 0.07)
	}
}

public rpg_fw_npccreate_post(ent){
	set_pev(ent, pev_enemy, rpg_get_princess())
	set_pev(ent, pev_maxspeed, 260.0)		// 最大移速
	rpg_animation_g(ent, 12)			// 静止动作
	SetMD_float(ent, md_attackradius, 95.0)		// 普攻范围
}

public rpg_fw_npc_attack(ent, target){
	new Float:velocity[3], Float:speed
	pev(ent, pev_velocity, velocity)
	velocity[2] = 0.0
	speed = vector_length(velocity)

	if(speed>140){
		rpg_animation_g(ent, 23)
		rpg_delaydmg_g(ent, 5.0, 0.2, 100.0, 1)
	}
	else{
		rpg_animation_g(ent, 22)
		rpg_delaydmg_g(ent, 3.0, 0.2, 100.0, 1)
	}

	set_pev(ent, pev_nextthink, get_gametime() + 1.5)
	return 1
}

public rpg_fw_npcthink_pre(ent){
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