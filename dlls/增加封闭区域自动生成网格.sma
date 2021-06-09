#include<amxmodx>
#include<fakemeta>
#include<fakemeta_util>

public plugin_init(){
	register_clcmd("bot_tp", "bottp") 	// 1.先将bot移动到你的位置
	register_clcmd("bot_a", "bota")		// 2.再指着bot使用此命令分析
}

public bottp(id){
	new Float:origin[3]
	pev(id, pev_origin, origin)
	origin[0]+=random_num(0,1)?random_float(20.0,60.0):random_float(-60.0,-20.0)
	origin[1]+=random_num(0,1)?random_float(20.0,60.0):random_float(-60.0,-20.0)
	for(new i=1;i<=get_maxplayers();++i){
		if(is_user_bot(i))
			set_pev(i, pev_origin, origin)
	}
}

public bota(id){
	new tar, body
	get_user_aiming(id,tar,body)

	if(tar <=get_maxplayers() && is_user_bot(tar)){
		new name[32]
		get_user_name(tar, name, 31)
		server_cmd("bot_nav_analyze2 ^"%s^"", name)
	}
}