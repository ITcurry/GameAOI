#include <cstdio>
#include <iostream>
#include <vector>
#include "Scene.h"
#include "Entity.h"

int main()
{
	//100*100的场景，10000个小格子,转行成9宫格，变成200多个大格子
	Scene scene(100, 100);
	//准备10000个实体，坐标分别是(0,0),(1,1),.....(99,99)
	std::vector<Entity> entity_vec;
	entity_vec.reserve(10000);
	for (int x = 0; x < 100; ++x)
	{
		for (int y = 0; y < 100; ++y)
		{
			Entity entity(x * 100 + y, x, y);
			entity_vec.push_back(entity);
		}
	}
	//全部进入场景
	for (auto it : entity_vec)
	{
		scene.EnterScene(it);
	}
	scene.Stat();
	getchar();
	//测试移动
	for (int i = 1; i < 100; ++i)
	{
		scene.MoveScene(entity_vec[0], i, i);
		getchar();
		entity_vec[0].SetX(i);
		entity_vec[1].SetY(i);
	}
	system("pause");
	getchar();
	return 0;
}