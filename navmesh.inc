#if defined _navmesh_included
  #endinput
#endif
#define _navmesh_included

enum RouteType
{
	FASTEST_ROUTE,
	SAFEST_ROUTE,
}

enum NavAttributeType
{
	NAV_CROUCH  = 0x01, // must crouch to use this node/area
	NAV_JUMP    = 0x02, // must jump to traverse this area
	NAV_PRECISE = 0x04, // do not adjust for obstacles, just move along area
	NAV_NO_JUMP = 0x08, // inhibit discontinuity jumping
}


/*
	执行寻路(起点,终点,路径类型) 返回路点数量
	注：内部由数组存储路点，每次寻路前会重置，所以get只能用在下次find前
*/
native navmesh_find(Float:start[3], Float:ent[3], iRouteType)


/*
	通过数组索引获取路点坐标(数组索引0-路点数量)
	注：索引超过路点数组只会返回0,0,0坐标
*/
native navmesh_get(pathIndex, Float:origin[3])

/*
	通过数组索引获取路点类型(数组索引0-路点数量)
	注：索引超过路点数组返回0
*/
native navmesh_attribute(pathIndex)

/*
	寻找里路径最近的点(当前位置,起点,终点,最近点)
*/
native navmesh_closetpoint(Float:position[3], start, end, Float:out[3])


/*
	调整目标位置(angle1,眼睛位置,目标位置)
*/
native navmesh_feeleradjustment(Float:yaw, Float:eyeOrigin[3], Float:goal[3])


/*
	通过位置获取位置的路点类型(位置)
	返回 NavAttributeType
*/
native navmesh_positionattribute(Float:position[3])


/*
	随机navmesh上的一个点(随机坐标)
	返回区域id
*/
native navmesh_randomPosition(Float:goal[3])


/*
	通过位置距离条件内的随机区域中心坐标(目标位置,最小距离,最大距离,随机坐标)
	返回区域id
*/
native navmesh_getRandomAreaPos(Float:position[3], Float:minsDistance, Float:maxDistance, Float:goal[3])


/* ========== 以下是util ========== */

stock Float:length2D(const Float:vec1[3], const Float:vec2[3])
{
	new Float:vec[3]
	for(new i;i<3;++i)
		vec[i] = vec2[i]-vec1[i]

	return xs_sqrt(vec[0]*vec[0] + vec[1]*vec[1]);
}

stock Float:LengthSquared(const Float:vec1[3], const Float:vec2[3])
{
	new Float:vec[3]
	for(new i;i<3;++i)
		vec[i] = vec2[i]-vec1[i]

	return vec[0]*vec[0] + vec[1]*vec[1];
}

stock bool:IsLengthLessThan(const Float:vec1[3], const Float:vec2[3], const Float:length)
{
	return LengthSquared(vec1, vec2) < length*length;
}

stock bool:IsLengthGreaterThan(const Float:vec1[3], const Float:vec2[3], const Float:length)
{
	return LengthSquared(vec1, vec2) > length*length;
}
