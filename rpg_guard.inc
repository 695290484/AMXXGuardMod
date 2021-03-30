/* 请务必用UTF8无BOM编码格式保存 */

const m_flVelocityModifier = 10	// 借用一下假装m_flVelocityModifier a.a

enum MONSTER_DATA{
	md_attackradius = 11,	// 普通攻击范围(float) 默认80.0
	md_jumppower,		// 跳跃力度(float) 默认350.0 [越高检测高度越高]
	md_speedmodify,		// 速度修改(float) 默认0.0 [大于0则 速度*=值]
	md_healthbar		// 显示血条(int) 默认0 [1为开启,运行时值变血条实体id] 血条pev_iuser4表示z坐标偏移
}

enum MISSION_TRIGGER{
	mt_PrincessShowUp,	// 公主出现时(公主实体id)
	mt_ChangeLevel,		// 难度切换时(当前难度)
	mt_MissionOver		// 任务结束时(0平局1失败2成功)
}

/* 设置怪物特殊属性(已封装好) */
stock GetMD_int(ent, md) return get_pdata_int(ent, md)
stock Float:GetMD_float(ent, md) return get_pdata_float(ent, md)
stock SetMD_int(ent, md, value) set_pdata_int(ent, md, value)
stock SetMD_float(ent, md, Float:value) set_pdata_float(ent, md, value)

#if !defined EXCLUDE_SELF

native rpg_create_g(Float:origin[3], Float:angles[3], iMonsterlevel, szClass[], szName[], Float:fHealth,  Float:fMinsize[3], Float:fMaxsize[3], iModelID) // 创建怪物
native rpg_delaydmg_g(ent, Float:damage, Float:delay, Float:distance, dmgType, f2f=1) // 延迟伤害(f2f的1表示正面,-1表示背面,0无要求)
native rpg_get_princess() // 获得公主的实体id
native rpg_get_darklevel() // 获得当前暗度等级

/**
 * 动作相关(自动使用移速来计算walk和run动作播放的速度)
 * 使用rpg_precache_g()来缓存怪物模型,指定walk和run的序号和一个动作播放完毕后移动距离(从ms3d查看或估计)
 * rpg_animation_g()只在walk和run播放时有额外处理,其他动作没有
 */
native rpg_precache_g(model[], walkSeq=-1, Float:walkDistance=0.0, runSeq=-1, Float:runDistance=0.0, &animCode)
native rpg_animation_g(iEntity, iAnim, animCode=-1, Float:framerate=1.0, again=0, Float:gaptime=0.0)
native rpg_set_animinterrupt_g(iEntity, Float:stopUntil) // 不允许播放动作直到gametime到stopUntil


/**
 * 总伤害 = CS原伤害 + 普通额外伤害 + 特殊额外伤害
 * 攻击时显示 "总伤害(+特殊额外伤害)"
 * 伤害事件除了post 其它的回调damage参数均为CS原伤害(理论上),需要用下面的接口修改伤害
 * 以下接口用于takedamage pre事件中
 */
native rpg_set_user_extradamage(id, Float:extdamage) // 设置特殊额外伤害
native Float:rpg_get_user_extradamage(id)
native rpg_set_user_normaldamage(id, Float:extdamage) // 设置普通额外伤害
native Float:rpg_get_user_normaldamage(id)

/**
 * 指定公主和僵尸出生的区域中心，然后会自动出生在区域中心附近区域里
 * 如果僵尸的未指定,则出生点见参数 rg_spawn_use_csdm 的说明
 * 如果公主的未指定,则出生在全图随机区域
 */
native rpg_set_princessArea(Float:origin[3])
native rpg_set_monsterArea(Float:origin[3])


/**
 * 各种事件
 */
forward rpg_fw_npckilled_post(ent, killer, headshot)
forward rpg_fw_npctakedamage_post(iEntity, attacker, Float:damage, damagetype, headshot)
forward rpg_fw_npccreate_post(ent)
forward rpg_fw_npcthink_pre(ent)
forward rpg_fw_npcthink_post(ent)
forward rpg_fw_npctraceattack_pre(iEntity, attacker, damage, damagetype, HitGroup)
forward rpg_fw_npctraceattack_post(iEntity, attacker, damage, damagetype, HitGroup)

forward rpg_fw_npc_jump(ent)
forward rpg_fw_npc_move(ent)
forward rpg_fw_npc_attack(ent, target)
forward rpg_fw_npc_refresh(level, numleft) // 怪物刷新(难度等级,剩下可刷的数目)
forward rpg_fw_mission_tigger(MISSION_TRIGGER:mt, data) // 任务节点触发时
forward rpg_fw_dark_change(darkLevel) // 亮度改变时(0-4 越大越暗)

stock isEntByClass(ent, const classname[]){
	new class[32]
	pev(ent, pev_classname, class, charsmax(class))
	if(!strcmp(class, classname))
		return 1
	return 0
}


stock util_Rendering(entity, fx = kRenderFxNone, r = 255, g = 255, b = 255, render = kRenderNormal, amount = 16, fTime=0.0)
{
	if (!pev_valid(entity))
		return

	new Float:color[3]
	color[0] = float(r)
	color[1] = float(g)
	color[2] = float(b)
	
	set_pev(entity, pev_renderfx, fx)
	set_pev(entity, pev_rendercolor, color)
	set_pev(entity, pev_rendermode, render)
	set_pev(entity, pev_renderamt, float(amount))

	if(fTime>0.0)
	{
		if(task_exists(entity+9501)) remove_task(entity+9501)
		set_task(fTime, "task_reset_rendering", entity+9501)
	}
}

public task_reset_rendering(tid)
{
	new id = tid - 9901
	if(!pev_valid(id)) return

	util_Rendering(id)
}

#endif
