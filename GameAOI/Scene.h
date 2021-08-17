#ifndef __SCENE_H__
#define __SCENE_H__
#include <iostream>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#define GX 8
#define GY 6
#define FACTOR 100
class Entity;
class Scene
{
public:
	Scene(int width = 100, int height = 100) :m_width(width), m_height(height) 
	{
		InitGrids();
	};
	Scene() = delete;
	~Scene() {};

	//8个方向
	enum DIR
	{
		DIR_NULL = 0,
		DIR_UP,
		DIR_DOWN,
		DIR_LEFT,
		DIR_RIGHT,
		DIR_LEFT_UP,
		DIR_RIGHT_UP,
		DIR_LEFT_DOWN,
		DIR_RIGHT_DOWN
	};
public:
	void EnterScene(const Entity &entity);
	void EnterScene(const int id, const int gid);
	void LeaveScene(const Entity &entity);
	void LeaveScene(const int id, const int gid);
	void MoveScene(const Entity &entity, const int x, const int y);
	int width() const { return m_width; }
	int height() const { return m_height; }
	void Stat() const;

private:
	//初始化9宫格数据
	void InitGrids();
	//对应AOI广播
	void EnterBroadcast(const Entity &entity, const int x, const int y);
	void LeaveBroadcast(const Entity &entity, const int x, const int y);
	void MoveBroadcast(const Entity &entity, const int x, const int y);
	//返回实体对应的Gid
	int GetGid(const int x, const int y) const;
	//九宫格坐标到九宫格id的转换
	int GxGy2Gid(const int gx, const int gy) const;
	//视野内的实体
	void ViewEntityes(std::unordered_set<int> &entityes, const int x, const int y);
	//视野内的格子
	void ViewGrids(std::unordered_set<int> &view_grids, const int x, const int y);
	//格子内的实体
	void GridEntityes(std::unordered_set<int> &entityes, int gid);
	void Notity(const Entity &entity, const std::unordered_set<int> &leave_grids, const std::unordered_set<int> &enter_grids);
	//计算移动方向
	int GetDir(const int x1, const int y1, const int x2, const int y2);
	//8个方向上的移动
	void MoveUp(const Entity &entity, int x, int y);
	void MoveDown(const Entity &entity, int x, int y);
	void MoveLeft(const Entity &entity, int x, int y);
	void MoveRight(const Entity &entity, int x, int y);
	void MoveLeftUp(const Entity &entity, int x, int y);
	void MoveRightUp(const Entity &entity, int x, int y);
	void MoveLeftDown(const Entity &entity, int x, int y);
	void MoveRightDown(const Entity &entity, int x, int y);
private:
	int m_width;//场景宽
	int m_height;//场景高
	typedef int Gid;
	typedef std::unordered_map<Gid, std::unordered_set<int>> Grids;
	Grids m_girds;
};
#endif