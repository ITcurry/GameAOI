#ifndef __ENTITY_H__
#define __ENTITY_H__
//简单场景实体 
class Entity
{
public:
	Entity(int id):m_id(id),m_x(0),m_y(0) {}
	Entity(int id, int x, int y):m_id(id),m_x(x),m_y(y) {}
	Entity() = delete;
	Entity& operator=(const Entity& rhs) = delete;
	~Entity() {};
public:
	int id() const { return m_id; }
	int x() const { return m_x; }
	int y() const { return m_y; }
	void SetX(int x) { m_x = x; }
	void SetY(int y) { m_y = y; }
private:
	int m_id;
	int m_x;
	int m_y;
};

#endif