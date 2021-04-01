
/* cvars */

init_cvars(){
	// 是否用csdm提供的复活点
	// 0: 用地图出生点 1: 地图出生点+附近nav路点 2: 用csdm复活点 3:csdm+附近nav路点
	spawn_use_csdm = e_register_cvar("rg_spawn_use_csdm", "1")
	load_spawns()

	// 怪物尸体延迟消失时间
	cvar_bodydelay = e_register_cvar("rg_bodydelay", "5.0")

	// 暗度成长速度
	cvar_gameseconds = e_register_cvar("rg_darkrate", "75")

	// 移除武器时间
	cvar_removeweapon = e_register_cvar("rg_removeweapon", "15.0")

	server_cmd("mp_round_infinite 1;mp_maxmoney 999999999;mp_respawn_immunitytime 3;mp_timelimit 0;")
	server_cmd("mp_infinite_ammo 2;mp_give_player_c4 0;mp_buy_anywhere 0")
	server_cmd("mp_autoteambalance 0")
	server_cmd("sv_rehlds_hull_centering 1") // requires REHLDS
}
