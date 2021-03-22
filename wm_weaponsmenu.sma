#include <amxmodx>
#include <fakemeta>
#include <hamsandwich>

#define MAX_WEAPONS 50 // 最大武器数
new gNameWeapon[MAX_WEAPONS][32], gFuncWeapon[MAX_WEAPONS][64], gTypeWeapon[MAX_WEAPONS]={-1,...}
new gCountWeapon, gDefaultWeapon, gIsDefaultWeapon[MAX_WEAPONS]

new gHasWeapon[33][MAX_WEAPONS], gUseWeapon[33][4], gUserCanChoose[33]

new gfwDummyResult, gfwGetWeapon

public plugin_init()
{
	register_weapons(0, "AK-47", "@weapon_ak47", 1)
	register_weapons(0, "M4A1", "@weapon_m4a1", 1)
	register_weapons(0, "M249", "@weapon_m249", 1)
	register_weapons(1, "USP", "@weapon_usp", 1)
	register_weapons(1, "沙漠之鹰", "@weapon_deagle", 1)
	register_weapons(2, "海豹短刀", "@weapon_knife", 1)
	register_weapons(3, "高爆手雷", "@weapon_hegrenade", 1)

	register_clcmd("wp", "show_weaponmenu")
	register_clcmd("say /wp", "show_weaponmenu")
	register_clcmd("say wp", "show_weaponmenu")

	register_event("ResetHUD", "EventResetHUD", "b")
	gfwGetWeapon = CreateMultiForward("wm_fw_get_weapon", ET_IGNORE, FP_CELL, FP_CELL, FP_CELL)
}

// 类型0,1,2,3 - 主,副,近身,投掷武器
// register_weapons(类型, "AK-47", "@weapon_ak47", 1)
// register_weapons(类型, "名称", "@+自带武器的字符串", 是否为默认武器)
// register_weapons(类型, "名称", "新武器的函数名称,如abc", 是否为默认武器) 然后外面加上public abc(id) give_new_weapon(id)
// cs自带的武器用@+weapon_xxx,新武器随便填个名称,然后在下面用public 名称(id)再填武器接口,默认武器就是会显示在菜单里不用购买的


public item_give_gravity(id) set_pev(id, pev_gravity, 0.5)
public item_give_hp(id) set_pev(id, pev_health, 200.0)

// ----------------------------------------------------------------------------------------------

public show_weaponmenu(id)
{
	if(!is_user_alive(id)) return PLUGIN_HANDLED
	new menuid, menu[256]
	formatex(menu, charsmax(menu), "\y选择武器:")
	menuid = menu_create(menu, "wpnmenuhandle")

	formatex(menu, 255, "主武器[\y%s\w]", gUseWeapon[id][0]<0?"无":gNameWeapon[gUseWeapon[id][0]])
	menu_additem(menuid, menu)
	formatex(menu, 255, "副武器[\y%s\w]", gUseWeapon[id][1]<0?"无":gNameWeapon[gUseWeapon[id][1]])
	menu_additem(menuid, menu)
	formatex(menu, 255, "近身武器[\y%s\w]", gUseWeapon[id][2]<0?"无":gNameWeapon[gUseWeapon[id][2]])
	menu_additem(menuid, menu)
	formatex(menu, 255, "投掷武器[\y%s\w]^n", gUseWeapon[id][3]<0?"无":gNameWeapon[gUseWeapon[id][3]])
	menu_additem(menuid, menu)
	menu_additem(menuid, "\y获得武器")

	formatex(menu, charsmax(menu), "退出")
	menu_setprop(menuid, MPROP_EXITNAME, menu)
	menu_display(id, menuid)
	return PLUGIN_HANDLED
}

public wpnmenuhandle(id, menuid, menukey)
{
	if(menukey<0)
	{
		menu_destroy(menuid)
		return
	}
	switch(menukey)
	{
		case 0..3: choose_weapon(id, menukey)
		case 4: get_weapons(id)
	}
	menu_destroy(menuid)
}

new typechoosed[33], menukeytoweaponid[33][MAX_WEAPONS], menucount[33]
choose_weapon(id, menukey)
{
	typechoosed[id] = menukey
	menucount[id] = 0
	new menuid, menu[256]
	formatex(menu, charsmax(menu), "\y选择武器:")
	menuid = menu_create(menu, "choosemenuhandle")

	for(new w;w<gCountWeapon;++w)
	{
		if(!gHasWeapon[id][w] || gTypeWeapon[w]!=menukey) continue

		menukeytoweaponid[id][menucount[id]] = w
		menucount[id] ++

		formatex(menu, 255, "%s", gNameWeapon[w])
		menu_additem(menuid, menu)
	}

	formatex(menu, charsmax(menu), "返回")
	menu_setprop(menuid, MPROP_EXITNAME, menu)
	menu_display(id, menuid)
}

public choosemenuhandle(id, menuid, menukey)
{
	if(menukey<0)
	{
		show_weaponmenu(id)
		menu_destroy(menuid)
		return
	}

	new wpnid = menukeytoweaponid[id][menukey]
	if(!gHasWeapon[id][wpnid])
	{
		show_weaponmenu(id)
		menu_destroy(menuid)
		return
	}

	gUseWeapon[id][typechoosed[id]] = wpnid

	show_weaponmenu(id)
	menu_destroy(menuid)
}

get_weapons(id)
{
	if(!is_user_alive(id)) return
	if(!gUserCanChoose[id]) return

	drop_weapons(id, 1)
	drop_weapons(id, 2)

	fm_strip_user_weapons(id)
	fm_give_item(id, "weapon_knife")

	new wpnid, temp[64], weaponid
	for(new t;t<4;++t)
	{
		wpnid = gUseWeapon[id][t]
		formatex(temp, 63, gFuncWeapon[wpnid])
		if(temp[0] == '@')
		{
			fm_give_item(id, temp[1])

			weaponid = get_weaponid(temp[1])
			refill_ammo(id, weaponid)
		}
		else if(gFuncWeapon[wpnid][0])
		{
			set_task(0.0, gFuncWeapon[wpnid], id)
		}
		ExecuteForward(gfwGetWeapon, gfwDummyResult, id, t, wpnid)
	}

	gUserCanChoose[id] = 0
}

public client_putinserver(id)
{
	for(new h;h<4;++h) gUseWeapon[id][h] = -1

	for(new w;w<gCountWeapon;++w)
	{
		if(!gIsDefaultWeapon[w])
		{
			gHasWeapon[id][w] = 0
			continue
		}
		gHasWeapon[id][w] = 1

		for(new d;d<4;++d)
		{
			if(gUseWeapon[id][d]<0 && gTypeWeapon[w]==d) gUseWeapon[id][d]=w
		}
	}
	gUserCanChoose[id] = 1
}

public client_infochanged(id)
{
	if(!is_user_connected(id)) return

	new newname[32], oldname[32]
	get_user_name(id,oldname,31)
	get_user_info(id, "name", newname,31)

	if(!equali(oldname,newname))
	{
		client_putinserver(id)
	}
}


register_weapons(type, name[], szFunc[], isDefault)
{
	if(gCountWeapon>=MAX_WEAPONS) return -1

	gTypeWeapon[gCountWeapon] = type
	formatex(gNameWeapon[gCountWeapon], 31, name)
	formatex(gFuncWeapon[gCountWeapon], 63, szFunc)
	gIsDefaultWeapon[gCountWeapon] = isDefault

	if(isDefault) gDefaultWeapon ++
	gCountWeapon ++
	return gCountWeapon-1
}

stock fm_give_item(iPlayer, const wEntity[])
{
	new iEntity = engfunc(EngFunc_CreateNamedEntity, engfunc(EngFunc_AllocString, wEntity))
	new Float:origin[3]
	pev(iPlayer, pev_origin, origin)
	set_pev(iEntity, pev_origin, origin)
	set_pev(iEntity, pev_spawnflags, pev(iEntity, pev_spawnflags) | SF_NORESPAWN)
	dllfunc(DLLFunc_Spawn, iEntity)
	new save = pev(iEntity, pev_solid)
	dllfunc(DLLFunc_Touch, iEntity, iPlayer)
	if(pev(iEntity, pev_solid) != save)
	return iEntity
	engfunc(EngFunc_RemoveEntity, iEntity)
	return -1
}

const PRIMARY_WEAPONS_BIT_SUM = (1<<CSW_SCOUT)|(1<<CSW_XM1014)|(1<<CSW_MAC10)|(1<<CSW_AUG)|(1<<CSW_UMP45)|(1<<CSW_SG550)|(1<<CSW_GALIL)|(1<<CSW_FAMAS)|(1<<CSW_AWP)|(1<<CSW_MP5NAVY)|(1<<CSW_M249)|(1<<CSW_M3)|(1<<CSW_M4A1)|(1<<CSW_TMP)|(1<<CSW_G3SG1)|(1<<CSW_SG552)|(1<<CSW_AK47)|(1<<CSW_P90)
const SECONDARY_WEAPONS_BIT_SUM = (1<<CSW_P228)|(1<<CSW_ELITE)|(1<<CSW_FIVESEVEN)|(1<<CSW_USP)|(1<<CSW_GLOCK18)|(1<<CSW_DEAGLE)
stock drop_weapons(id, dropwhat)
{

	static weapons[32], num, i, weaponid
	num = 0
	get_user_weapons(id, weapons, num)

	for (i = 0; i < num; i++)
	{
		// Prevent re-indexing the array
		weaponid = weapons[i]
		
		if ((dropwhat == 1 && ((1<<weaponid) & PRIMARY_WEAPONS_BIT_SUM)) || (dropwhat == 2 && ((1<<weaponid) & SECONDARY_WEAPONS_BIT_SUM)))
		{
			static wname[32]; get_weaponname(weaponid, wname, charsmax(wname))
			engclient_cmd(id, "drop", wname)
		}
	}
}

stock fm_strip_user_weapons(id)
{
	static ent
	ent = engfunc(EngFunc_CreateNamedEntity, engfunc(EngFunc_AllocString, "player_weaponstrip"))
	if (!pev_valid(ent)) return;
	
	dllfunc(DLLFunc_Spawn, ent)
	dllfunc(DLLFunc_Use, ent, id)
	engfunc(EngFunc_RemoveEntity, ent)
}

new const MAXBPAMMO[] = { -1, 52, -1, 90, 1, 32, 1, 100, 90, 1, 120, 100, 100, 90, 90, 90, 100, 120,
			30, 120, 200, 32, 90, 120, 90, 2, 35, 90, 90, -1, 100 }

new const AMMOTYPE[][] = { "", "357sig", "", "762nato", "", "buckshot", "", "45acp", "556nato", "", "9mm", "57mm", "45acp",
			"556nato", "556nato", "556nato", "45acp", "9mm", "338magnum", "9mm", "556natobox", "buckshot",
			"556nato", "9mm", "762nato", "", "50ae", "556nato", "762nato", "", "57mm" }

refill_ammo(id, weaponid)
{
	if(weaponid>0) ExecuteHamB(Ham_GiveAmmo, id, MAXBPAMMO[weaponid], AMMOTYPE[weaponid], MAXBPAMMO[weaponid])
}

public EventResetHUD(id)
{
	gUserCanChoose[id] = 1
	show_weaponmenu(id)
}

public plugin_precache() register_plugin("Weapons Menu", "4.28", "zhiJiaN")

public plugin_natives()
{
	register_native("wm_give_weapon_by_name", "give_weapon_by_name")
	register_native("wm_take_weapon_by_name", "take_weapon_by_name")
	register_native("wm_get_wpnid_by_name", "get_weaponid_by_name")
}

// native wm_give_weapon_by_name(id, wp_name[])
public give_weapon_by_name(a, b)
{
	if(b != 2) return -1

	new id = get_param(1)
	new name[32]
	get_string(2, name, 31)

	for(new w;w<gCountWeapon;++w)
	{
		if(gHasWeapon[id][w] || strcmp(gNameWeapon[w], name)) continue
		gHasWeapon[id][w] = 1
		return w
	}
	return -1
}

// native wm_take_weapon_by_name(id, wp_name[])
public take_weapon_by_name(a, b)
{
	if(b != 2) return -1

	new id = get_param(1)
	new name[32]
	get_string(2, name, 31)

	for(new w;w<gCountWeapon;++w)
	{
		if(!gHasWeapon[id][w] || strcmp(gNameWeapon[w], name)) continue
		gHasWeapon[id][w] = 0
		return w
	}
	return -1
}

// native wm_get_wpnid_by_name(wp_name[])
public get_weaponid_by_name(a, b)
{
	if(b != 1) return -1

	new name[32]
	get_string(1, name, 31)

	for(new w;w<gCountWeapon;++w)
	{
		if(strcmp(gNameWeapon[w], name)) continue
		return w
	}
	return -1
}

// foward wm_fw_get_weapon(id, type, wpnid)
