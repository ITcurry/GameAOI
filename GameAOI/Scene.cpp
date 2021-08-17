
#include "Scene.h"
#include "Entity.h"

void Scene::InitGrids()
{
	//将场景中的每个坐标转换为9宫格坐标
	for (int w = 0; w < m_width; ++w)
	{
		for (int h = 0; h < m_height; ++h)
		{
			int gid = GetGid(w, h);
			auto iter = m_girds.find(gid);
			if (iter == m_girds.end())
			{
				m_girds.insert({ gid, {} });
			}
		}
	}
}

void Scene::Stat() const
{
	std::cout << m_girds.size() << std::endl;
}

//进入场景
void Scene::EnterScene(const Entity &entity)
{
	//实体进入场景，将自己进入的信息广播给对应的区域
	//同时将对应区域中其他实体的状态同步到实体
	int gid = GetGid(entity.x(), entity.y());
	EnterScene(entity.id(), gid);
	EnterBroadcast(entity, entity.x(), entity.y());
}

void Scene::EnterScene(const int id, const int gid)
{
	auto iter = m_girds.find(gid);
	if (iter != m_girds.end())
	{
		auto find_it = iter->second.find(id);
		if (find_it == iter->second.end())
		{
			iter->second.insert(id);
		}
	}
}
//离开场景
void Scene::LeaveScene(const Entity &entity)
{
	//实体离开场景或视野
	//发送离开通知到视野内的其他视野、更新该区域的实体集合数据
	int gid = GetGid(entity.x(), entity.y());
	LeaveScene(entity.id(), gid);
	LeaveBroadcast(entity, entity.x(), entity.y());
}

void Scene::LeaveScene(const int id, const int gid)
{
	auto iter = m_girds.find(gid);
	if (iter != m_girds.end())
	{
		auto find_it = iter->second.find(id);
		if (find_it != iter->second.end())
		{
			iter->second.erase(id);
		}
	}
}
//实体的位置信息自己更新和维护，场景不负责更新实体的坐标 
void Scene::MoveScene(const Entity &entity, const int x, const int y)
{
	//先向所在区域发送移动信息
	MoveBroadcast(entity, x, y);
	//移动的时候要区分是否跨格子，如果没有跨格子，只需要向视野区域广播移动包就可以
	//如果跨格子的话，需要向离开的区域广播离开消息，同时向进入的区域广播进入区域，同时在这三个区域都广播移动消息
	int old_gid = GetGid(entity.x(), entity.y());
	int new_gid = GetGid(x, y);
	if (old_gid == new_gid)
	{
		return;
	}
	else
	{
		int gx1 = entity.x() / GX;
		int gy1 = entity.y() / GY;
		int gx2 = x / GX;
		int gy2 = x / GY;
		int dir = GetDir(entity.x(), entity.y(), x, y);
		//根据不同的方向移动
		switch (dir)
		{
		case DIR_UP:
			MoveUp(entity, x, y);
			break;
		case DIR_DOWN:
			MoveDown(entity, x, y);
			break;
		case DIR_LEFT:
			MoveLeft(entity, x, y);
			break;
		case DIR_RIGHT:
			MoveRight(entity, x, y);
			break;
		case DIR_LEFT_UP:
			MoveLeftUp(entity, x, y);
			break;
		case DIR_RIGHT_UP:
			MoveRightUp(entity, x, y);
			break;
		case DIR_LEFT_DOWN:
			MoveLeftDown(entity, x, y);
			break;
		case DIR_RIGHT_DOWN:
			MoveRightDown(entity, x, y);
			break;
		default:
			std::cout << "error dir!!!" << std::endl;
			break;
		}
	}
}

//通知离开
void SendLeave(const Entity &entity, const std::unordered_set<int> &ids)
{
	for (auto id : ids)
	{
		//todo 封装一个离开包，发送给对应id的实体，或者直接发往id对应的网络连接
		std::cout << "notify: " << id << " " << entity.id() << " leave." << std::endl;
	}
}

//通知进入
void SendEnter(const Entity &entity, const std::unordered_set<int> &ids)
{
	for (auto id : ids)
	{
		//todo 封装一个包，发送给对应id的实体，或者直接发往id对应的网络连接
		std::cout << "notify: " << id << " " << entity.id() << " enter." << std::endl;
	}
}

void Scene::EnterBroadcast(const Entity &entity, const int x, const int y)
{
	std::unordered_set<int> entityes;
	ViewEntityes(entityes, x, y);
	//向视野内的实体广播entity进入
	SendEnter(entity, entityes);
}

void Scene::LeaveBroadcast(const Entity &entity, const int x, const int y)
{
	std::unordered_set<int> entities;
	ViewEntityes(entities, entity.x(), entity.y());
	SendLeave(entity, entities);
}

void Scene::MoveBroadcast(const Entity &entity, const int x, const int y)
{
	std::unordered_set<int> entityes;
	ViewEntityes(entityes, x, y);
	std::cout << "broadcast move num: " << entityes.size() << std::endl;
	//向视野内的实体通告entity进入
	for (auto id : entityes)
	{
		//todo 发送移动消息
		std::cout << "notify: " << id << " " << entity.id() << " move to " << x << " " << y << std::endl;
	}
}

int Scene::GetGid(const int x, const int y) const
{
	int gx = x / GX;
	int gy = y / GY;
	return GxGy2Gid(gx, gy);
}

int Scene::GxGy2Gid(const int gx, const int gy) const
{
	int gx1 = gx < 0 ? 0 : gx;
	int gy1 = gy < 0 ? 0 : gy;
	return gx1 * FACTOR + gy1;
}

//视野内的实体
void Scene::ViewEntityes(std::unordered_set<int> &entityes, const int x, const int y)
{
	std::unordered_set<int> view_grids;
	ViewGrids(view_grids, x, y);
	for (auto gid : view_grids)
	{
		auto grid_it = m_girds.find(gid);
		if (grid_it != m_girds.end())
		{
			for (auto grid_entity_id : grid_it->second)
			{
				entityes.insert(grid_entity_id);
			}
		}
	}
}
//视野内的9宫格
//格子加1可能为非法位置
void Scene::ViewGrids(std::unordered_set<int> &view_grids, const int x, const int y)
{
	int gx = x / GX;
	int gy = y / GY;
	view_grids.insert({
		GxGy2Gid(gx,gy+1),		//正上方
		GxGy2Gid(gx-1,gy+1),	//左上方
		GxGy2Gid(gx+1,gy+x),	//右上方
		GxGy2Gid(gx,gy),		//中间
		GxGy2Gid(gx-1,gy),		//左边
		GxGy2Gid(gx+1,gy),		//右边
		GxGy2Gid(gx,gy-1),		//正下方
		GxGy2Gid(gx-1,gy-1),	//左下方
		GxGy2Gid(gx+1,gy-1)		//右下方
	});
}

void Scene::GridEntityes(std::unordered_set<int> &entityes, int gid)
{
	auto grid_iter = m_girds.find(gid);
	if (grid_iter != m_girds.end())
	{
		for (auto grid_entity_id : grid_iter->second)
		{
			entityes.insert(grid_entity_id);
		}
	}
}
int Scene::GetDir(const int x1, const int y1, const int x2, const int y2)
{
	//垂直方向移动
	if (x1 == x2)
	{
		if (y2 > y1)
			return DIR_UP;
		else
			return DIR_DOWN;
	}
	//水平移动
	if (y1 == y2)
	{
		if (x2 > x1)
			return DIR_RIGHT;
		else
			return DIR_LEFT;
	}
	if (x2 > x1 && y2 > y1)
	{
		return DIR_RIGHT_UP;
	}
	if (x2 > x1 && y2 < y1)
	{
		return DIR_RIGHT_DOWN;
	}
	if (x1 > x2 && y2 > y1)
	{
		return DIR_LEFT_UP;
	}
	if (x1 > x2 && y2 < y1)
	{
		return DIR_LEFT_DOWN;
	}
	return DIR_NULL;
}
//进行相应的消息通知
void Scene::Notity(const Entity &entity, const std::unordered_set<int> &leave_grids, const std::unordered_set<int> &enter_grids)
{
	std::unordered_set<int> leave_entityes, enter_entityes;
	for (auto it : leave_grids)
	{
		//获取需要通知实体离开消息的其他实体
		GridEntityes(leave_entityes, it);
	}
	for (auto it : enter_grids)
	{
		//获取需要通知实体进入消息的其他实体
		GridEntityes(enter_entityes, it);
	}
	SendLeave(entity, leave_entityes);
	SendEnter(entity, enter_entityes);
}
//向上移动，下面3个格子会离开视野，上面3个新的格子进入视野
void Scene::MoveUp(const Entity &entity, int x, int y)
{
	std::unordered_set<int> leave_grids, enter_grids;
	int gx1 = entity.x() / GX;
	int gy1 = entity.y() / GY;
	int gx2 = x / GX;
	int gy2 = y / GY;
	leave_grids.insert({
		GxGy2Gid(gx1,gy1-1),	//下方
		GxGy2Gid(gx1-1,gy1-1),	//左下方
		GxGy2Gid(gx1+1,gy1-1)	//右下方
	});
	enter_grids.insert({
		GxGy2Gid(gx2,gy2+1),	//上方
		GxGy2Gid(gx2-1,gy2+1),	//左上方
		GxGy2Gid(gx2+1,gy2+1)	//右上方
	});
	LeaveScene(entity.id(), GxGy2Gid(gx1, gy1));
	EnterScene(entity.id(), GxGy2Gid(gx2, gy2));
	Notity(entity, leave_grids, enter_grids);
}

void Scene::MoveDown(const Entity &entity, int x, int y)
{
	std::unordered_set<int> leave_grids, enter_grids;
	int gx1 = entity.x() / GX;
	int gy1 = entity.y() / GY;
	int gx2 = x / GX;
	int gy2 = y / GY;
	leave_grids.insert({
		GxGy2Gid(gx1, gy1 + 1),    //上方
		GxGy2Gid(gx1 - 1,gy1 + 1), //左上方
		GxGy2Gid(gx1 + 1,gy1 + 1)  //右上方
		});
	enter_grids.insert({
		GxGy2Gid(gx2, gy2 - 1),    //下方
		GxGy2Gid(gx2 - 1,gy2 - 1), //左下方
		GxGy2Gid(gx2 + 1,gy2 - 1)  //右下方
		});
	LeaveScene(entity.id(), GxGy2Gid(gx1, gy1));
	EnterScene(entity.id(), GxGy2Gid(gx2, gy2));
	Notity(entity, leave_grids, enter_grids);
}

void Scene::MoveLeft(const Entity &entity, int x, int y)
{
	std::unordered_set<int> leave_grids, enter_grids;
	int gx1 = entity.x() / GX;
	int gy1 = entity.y() / GY;
	int gx2 = x / GX;
	int gy2 = y / GY;
	leave_grids.insert({
		GxGy2Gid(gx1, gy1 + 1),    //右方
		GxGy2Gid(gx1 + 1,gy1 + 1), //右上方
		GxGy2Gid(gx1 + 1,gy1 - 1)  //右下方

		});
	enter_grids.insert({
		GxGy2Gid(gx2 - 1,gy2 + 1), //左上方
		GxGy2Gid(gx2 - 1,gy2 - 1), //左下方
		GxGy2Gid(gx2 - 1,gy1), //左方
		});
	LeaveScene(entity.id(), GxGy2Gid(gx1, gy1));
	EnterScene(entity.id(), GxGy2Gid(gx2, gy2));
	Notity(entity, leave_grids, enter_grids);
}

void Scene::MoveRight(const Entity &entity, int x, int y)
{
	std::unordered_set<int> leave_grids, enter_grids;
	int gx1 = entity.x() / GX;
	int gy1 = entity.y() / GY;
	int gx2 = x / GX;
	int gy2 = y / GY;
	leave_grids.insert({
		GxGy2Gid(gx1 - 1,gy1 + 1), //左上方
		GxGy2Gid(gx1 - 1,gy1 - 1), //左下方
		GxGy2Gid(gx1 - 1,gy1), //左方
		});
	enter_grids.insert({
		GxGy2Gid(gx2, gy2 + 1),    //右方
		GxGy2Gid(gx2 + 1,gy2 + 1), //右上方
		GxGy2Gid(gx2 + 1,gy2 - 1)  //右下方
		});
	LeaveScene(entity.id(), GxGy2Gid(gx1, gy1));
	EnterScene(entity.id(), GxGy2Gid(gx2, gy2));
	Notity(entity, leave_grids, enter_grids);
}

void Scene::MoveLeftUp(const Entity &entity, int x, int y)
{
	std::unordered_set<int> leave_grids, enter_grids;
	int gx1 = entity.x() / GX;
	int gy1 = entity.y() / GY;
	int gx2 = x / GX;
	int gy2 = y / GY;
	leave_grids.insert({
		GxGy2Gid(gx1,gy1 - 1),    //下方
		GxGy2Gid(gx1 - 1,gy1 - 1),    //左下方
		GxGy2Gid(gx1 + 1,gy1 + 1), //右上方
		GxGy2Gid(gx1 + 1,gy1 - 1),  //右下方
		GxGy2Gid(gx1 + 1,gy1) //右边
		});
	enter_grids.insert({
		GxGy2Gid(gx2 - 1,gy2 + 1), //左上方
		GxGy2Gid(gx2 - 1,gy2 - 1), //左下方
		GxGy2Gid(gx2 - 1,gy2), //左方
		GxGy2Gid(gx2 + 1,gy2 + 1), //右上
		GxGy2Gid(gx2,gy2 + 1) //上
		});
	LeaveScene(entity.id(), GxGy2Gid(gx1, gy1));
	EnterScene(entity.id(), GxGy2Gid(gx2, gy2));
	Notity(entity, leave_grids, enter_grids);
}

void Scene::MoveRightUp(const Entity &entity, int x, int y)
{
	std::unordered_set<int> leave_grids, enter_grids;
	int gx1 = entity.x() / GX;
	int gy1 = entity.y() / GY;
	int gx2 = x / GX;
	int gy2 = y / GY;
	leave_grids.insert({
		GxGy2Gid(gx1,gy1 - 1),    //下方
		GxGy2Gid(gx1 + 1,gy1 - 1),  //右下方
		GxGy2Gid(gx1 - 1,gy1 - 1),    //左下方
		GxGy2Gid(gx1 - 1,gy1 + 1), //左上方
		GxGy2Gid(gx1 - 1,gy1) //左边
		});
	enter_grids.insert({
		GxGy2Gid(gx2 - 1,gy2 + 1), //左上方
		GxGy2Gid(gx2,gy2 + 1), //上
		GxGy2Gid(gx2 + 1,gy2 + 1), //右上
		GxGy2Gid(gx2 + 1,gy2), //右方
		GxGy2Gid(gx2 + 1,gy2 - 1) //右下方
		});
	//printf("gx1 %d gy1 %d gx2 %d gy2 %d\n", gx1, gy1, gx2, gy2);
	LeaveScene(entity.id(), GxGy2Gid(gx1, gy1));
	EnterScene(entity.id(), GxGy2Gid(gx2, gy2));
	Notity(entity, leave_grids, enter_grids);
}

void Scene::MoveLeftDown(const Entity &entity, int x, int y)
{
	std::unordered_set<int> leave_grids, enter_grids;
	int gx1 = entity.x() / GX;
	int gy1 = entity.y() / GY;
	int gx2 = x / GX;
	int gy2 = y / GY;
	leave_grids.insert({
		GxGy2Gid(gx1 - 1,gy1 + 1), //左上方
		GxGy2Gid(gx1,gy1 + 1), //上
		GxGy2Gid(gx1 + 1,gy1 + 1), //右上
		GxGy2Gid(gx1 + 1,gy1), //右方
		GxGy2Gid(gx1 + 1,gy1 - 1) //右下方
		});
	enter_grids.insert({
		GxGy2Gid(gx2,gy2 - 1),    //下方
		GxGy2Gid(gx2 + 1,gy2 - 1),  //右下方
		GxGy2Gid(gx2 - 1,gy2 - 1),    //左下方
		GxGy2Gid(gx2 - 1,gy2 + 1), //左上方
		GxGy2Gid(gx2 - 1,gy2) //左边
		});
	LeaveScene(entity.id(), GxGy2Gid(gx1, gy1));
	EnterScene(entity.id(), GxGy2Gid(gx2, gy2));
	Notity(entity, leave_grids, enter_grids);
}

void Scene::MoveRightDown(const Entity &entity, int x, int y)
{
	std::unordered_set<int> leave_grids, enter_grids;
	int gx1 = entity.x() / GX;
	int gy1 = entity.y() / GY;
	int gx2 = x / GX;
	int gy2 = y / GY;
	leave_grids.insert({
		GxGy2Gid(gx1 - 1,gy1 + 1), //左上方
		GxGy2Gid(gx1 - 1,gy1 - 1), //左下方
		GxGy2Gid(gx1 - 1,gy1), //左方
		GxGy2Gid(gx1 + 1,gy1 + 1), //右上
		GxGy2Gid(gx1,gy1 + 1) //上
		});
	enter_grids.insert({
		GxGy2Gid(gx2,gy2 - 1),    //下方
		GxGy2Gid(gx2 - 1,gy2 - 1),    //左下方
		GxGy2Gid(gx2 + 1,gy2 + 1), //右上方
		GxGy2Gid(gx2 + 1,gy2 - 1),  //右下方
		GxGy2Gid(gx2 + 1,gy2) //右边
		});
	LeaveScene(entity.id(), GxGy2Gid(gx1, gy1));
	EnterScene(entity.id(), GxGy2Gid(gx2, gy2));
	Notity(entity, leave_grids, enter_grids);
}




