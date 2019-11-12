#include "Scene_Node.h"

Scene_Node::Scene_Node()
{
	m_p_model = NULL;

	m_x = 0.0f; m_y = 0.0f; m_z = 0.0f;
	m_xangle = 0.0f; m_yangle = 0.0f; m_zangle = 0.0f;

	m_scale = 1.0f; //initial scale is 1.0f;

}

void Scene_Node::addChildNode(Scene_Node *n)
{
	m_children.push_back(n);
}

bool Scene_Node::detatchNode(Scene_Node *n)
{
	// traverse tree to find node to detatch
	for (int i = 0; i < m_children.size(); i++)
	{
		if (n == m_children[i])
		{
			m_children.erase(m_children.begin() + i);
			return true;
		}
		if (m_children[i]->detatchNode(n) == true) return true;
	}
	return false; // node not in this tree
}

XMVECTOR  Scene_Node::get_world_centre_position()
{
	return XMVectorSet(m_world_centre_x,
		m_world_centre_y,
		m_world_centre_z, 0.0);
}

void Scene_Node::update_collision_tree(XMMATRIX* world, float scale)
{
	//calc world scale
	m_world_scale = scale * m_scale;

	XMVECTOR v;
	if (m_p_model)
	{
		v = XMVectorSet(m_p_model->GetBoundingSphere_x(),
			m_p_model->GetBoundingSphere_y(),
			m_p_model->GetBoundingSphere_z(), 0.0);
	}
	else v = XMVectorSet(0, 0, 0, 0); // no model, default to 0

									  // the local_world matrix will be used to calc the local transformations for this node
	XMMATRIX local_world = XMMatrixIdentity();

	local_world = XMMatrixRotationX(XMConvertToRadians(m_xangle));
	local_world *= XMMatrixRotationY(XMConvertToRadians(m_yangle));
	local_world *= XMMatrixRotationZ(XMConvertToRadians(m_zangle));

	local_world *= XMMatrixScaling(m_scale, m_scale, m_scale);

	local_world *= XMMatrixTranslation(m_x, m_y, m_z);

	// the local matrix is multiplied by the passed in world matrix that contains the concatenated
	// transformations of all parent nodes so that this nodes transformations are relative to those
	local_world *= *world;

	// find and store world space bounding sphere centre
	v = XMVector3Transform(v, local_world);
	m_world_centre_x = XMVectorGetX(v);
	m_world_centre_y = XMVectorGetY(v);
	m_world_centre_z = XMVectorGetZ(v);

	// traverse all child nodes, passing in the concatenated world data   
	for (int i = 0; i< m_children.size(); i++)
	{
		m_children[i]->update_collision_tree(
			&local_world, m_world_scale);
	}

}

bool Scene_Node::check_collision(Scene_Node* compare_tree, Scene_Node* object_tree_root)
{

	// check to see if root of tree being compared is same as root node of object tree being checked
	// i.e. stop object node and children being checked against each other
	if (object_tree_root == compare_tree) return false;

	// only check for collisions if both nodes contain a model
	if (m_p_model && compare_tree->m_p_model)
	{
		m_p_model->CalculateModelCentrePoint();
		XMVECTOR v1 = get_world_centre_position();
		XMVECTOR v2 = compare_tree->get_world_centre_position();
		XMVECTOR vdiff = v1 - v2;

		//XMVECTOR a = XMVector3Length(vdiff);
		float x1 = XMVectorGetX(v1);
		float x2 = XMVectorGetX(v2);
		float y1 = XMVectorGetY(v1);
		float y2 = XMVectorGetY(v2);
		float z1 = XMVectorGetZ(v1);
		float z2 = XMVectorGetZ(v2);

		float dx = x1 - x2;
		float dy = y1 - y2;
		float dz = z1 - z2;

		// check bounding sphere collision
		if (sqrt(dx*dx + dy * dy + dz * dz) <
			(compare_tree->m_p_model->GetBoundingSphereRadius() * compare_tree->m_world_scale) +
			(this->m_p_model->GetBoundingSphereRadius() * m_world_scale))
		{
			return true;
		}
	}

	// iterate through compared tree child nodes
	for (int i = 0; i< compare_tree->m_children.size(); i++)
	{
		// check for collsion against all compared tree child nodes 
		if (check_collision(compare_tree->m_children[i], object_tree_root) == true) return true;
	}

	// iterate through composite object child nodes
	for (int i = 0; i< m_children.size(); i++)
	{
		// check all the child nodes of the composite object against compared tree
		if (m_children[i]->check_collision(compare_tree, object_tree_root) == true) return true;
	}

	return false;



}

bool Scene_Node::IncX(float in, Scene_Node* root_node)
{
	float old_x = m_x;	// save current state 
	m_x += in;		// update state

	XMMATRIX identity = XMMatrixIdentity();

	// since state has changed, need to update collision tree
	// this basic system requires entire hirearchy to be updated
	// so start at root node passing in identity matrix
	root_node->update_collision_tree(&identity, 1.0);
	for (int i = 0; i++; i > m_children.size())
	{

		// check for collision of this node (and children) against all other nodes
		if (check_collision(m_children[i], root_node) == true)
		{
			// if collision restore state
			m_x = old_x;

			return true;
		}
	}

	return false;
};

bool Scene_Node::IncY(float in, Scene_Node* root_node)
{
	float old_y = m_y;	// save current state 
	m_y += in;		// update state

	XMMATRIX identity = XMMatrixIdentity();

	// since state has changed, need to update collision tree
	// this basic system requires entire hirearchy to be updated
	// so start at root node passing in identity matrix
	root_node->update_collision_tree(&identity, 1.0);

	for (int i = 0; i++; i > m_children.size())
	{

		// check for collision of this node (and children) against all other nodes
		if (check_collision(m_children[i], root_node) == true)
		{
			// if collision restore state
			m_y = old_y;

			return true;
		}
	}

	return false;
};

bool Scene_Node::IncZ(float in, Scene_Node* root_node)
{
	float old_z = m_z;	// save current state 
	m_z += in;		// update state

	XMMATRIX identity = XMMatrixIdentity();

	// since state has changed, need to update collision tree
	// this basic system requires entire hirearchy to be updated
	// so start at root node passing in identity matrix
	root_node->update_collision_tree(&identity, 1.0);

	for (int i = 0; i++; i > m_children.size())
	{

		// check for collision of this node (and children) against all other nodes
		if (check_collision(m_children[i], root_node) == true)
		{
			// if collision restore state
			m_z = old_z;

			return true;
		}
	}
	return false;
};

void Scene_Node::execute(XMMATRIX *world, XMMATRIX* view, XMMATRIX* projection)
{
	// the local_world matrix will be used to calc the local transformations for this node
	XMMATRIX local_world = XMMatrixIdentity();

	local_world = XMMatrixRotationX(XMConvertToRadians(m_xangle));
	local_world *= XMMatrixRotationY(XMConvertToRadians(m_yangle));
	local_world *= XMMatrixRotationZ(XMConvertToRadians(m_zangle));

	local_world *= XMMatrixScaling(m_scale, m_scale, m_scale);

	local_world *= XMMatrixTranslation(m_x, m_y, m_z);

	// the local matrix is multiplied by the passed in world matrix that contains the concatenated
	// transformations of all parent nodes so that this nodes transformations are relative to those
	local_world *= *world;

	// only draw if there is a model attached
	if (m_p_model) m_p_model->Draw(&local_world, view, projection);

	// traverse all child nodes, passing in the concatenated world matrix
	for (int i = 0; i < m_children.size(); i++)
	{
		m_children[i]->execute(&local_world, view, projection);
	}
}

void Scene_Node::SetModel(Model* model)
{
	m_p_model = model;
	m_p_model->CalculateBoundingSphereRadius();

}

void Scene_Node::LookAt_XZ(float x, float z)
{
	//calculate dx and dz between the object and the look at position passed in.
	float DX = x - m_x;
	float DZ = z - m_z;

	//update m_yangle using the arctangent calculation and converting to degrees.
	m_yangle = atan2(DX, DZ) * (180.0 / XM_PI);

}

void Scene_Node::Rotate(float degree_number)
{

	m_yangle += degree_number;

}

void Scene_Node::Forward(float distance)
{

	m_x += sin(m_yangle * (XM_PI / 180.0)) * distance;
	m_z += cos(m_yangle * (XM_PI / 180.0)) * distance;
}

void Scene_Node::SetXRot(float xRotation) { m_xangle = xRotation; }
void Scene_Node::SetYRot(float yRotation) { m_yangle = yRotation; }
void Scene_Node::SetZRot(float zRotation) { m_zangle = zRotation; }

void Scene_Node::SetScale(float scale) { m_scale = scale; }

float Scene_Node::GetXRot() { return m_xangle; }
float Scene_Node::GetYRot() { return m_yangle; }
float Scene_Node::GetZRot() { return m_zangle; }

float Scene_Node::GetScale() { return m_scale; }