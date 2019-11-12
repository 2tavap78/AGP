#pragma once
#include <d3d11.h>
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <math.h>
#include <xnamath.h>
#include "Model.h"


class Scene_Node {

private:
	Model * m_p_model;
	vector<Scene_Node*>   m_children;

	float  m_x, m_y, m_z;
	float  m_xangle, m_zangle, m_yangle;
	float  m_scale;

	float m_world_centre_x;
	float m_world_centre_y;
	float m_world_centre_z;
	float m_world_scale;


public:
	Scene_Node();
	~Scene_Node();

	void update_collision_tree(XMMATRIX* world, float scale);
	XMVECTOR get_world_centre_position();
	//get/set funcs
	void SetXPos(float in) { m_x = in; };
	float GetXPos(void) { return m_x; };
	void SetYPos(float in) { m_y = in; };
	float GetYPos(void) { return m_y; };
	void SetZPos(float in) { m_z = in; };
	float GetZPos(void) { return m_z; };
	void LookAt_XZ(float x, float z);

	void SetXRot(float xRotation);
	void SetYRot(float yRotation);
	void SetZRot(float zRotation);

	void SetScale(float scale);

	float GetXRot();
	float GetYRot();
	float GetZRot();

	float GetScale();
	void Rotate(float degree_number);



	void addChildNode(Scene_Node *n);
	void SetModel(Model* model);
	bool detatchNode(Scene_Node *n);
	void execute(XMMATRIX *world, XMMATRIX* view, XMMATRIX* projection);
	bool check_collision(Scene_Node* compare_tree, Scene_Node* object_tree_root);
	bool IncX(float in, Scene_Node* root_node);
	bool IncY(float in, Scene_Node* root_node);
	bool IncZ(float in, Scene_Node* root_node);

	void Forward(float distance);
};