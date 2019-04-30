#include "bvh.h"
#include "../platform/debug_renderer.h"
#include <memory/frame_heap.h>

namespace lambda
{
  namespace utilities
  {
    ///////////////////////////////////////////////////////////////////////////
	void BaseBVH::add(const entity::Entity& entity, void* user_data, const BVHAABB& aabb)
	{
		num_nodes_++;
		BVHNode* node = privateCreate();
		node->entity    = entity;
		node->user_data = user_data;
		node->aabb      = aabb;

		if (!base_node_)
			base_node_ = node;
		else
			insert(node, base_node_);
	}

	///////////////////////////////////////////////////////////////////////////
	void BaseBVH::remove(const entity::Entity& entity)
	{
		findAndRemove(base_node_, entity);
	}

	///////////////////////////////////////////////////////////////////////////
	void BaseBVH::clear()
	{
		if (base_node_)
			clear(base_node_);
		base_node_ = nullptr;
	}

	///////////////////////////////////////////////////////////////////////////
	void BaseBVH::clear(BVHNode* node)
	{
		if (node->child_left)
			clear(node->child_left);
		if (node->child_right)
			clear(node->child_right);
		privateRemove(node);
	}

	///////////////////////////////////////////////////////////////////////////
	void BaseBVH::findAndRemove(BVHNode* node, const entity::Entity& entity)
	{
		if (!node->is_leaf)
		{
			if (node->child_left)
				findAndRemove(node->child_left, entity);
			if (node->child_right)
				findAndRemove(node->child_right, entity);
		}
		else if (node->entity == entity)
			remove(node);
	}

	///////////////////////////////////////////////////////////////////////////
	void BaseBVH::remove(BVHNode* node)
	{
		if (!node)
			return;

		if (node->is_leaf)
		{
			BVHNode* parent = node->parent;
			if (parent)
			{
				if (node == parent->child_left)
					parent->child_left = nullptr;
				else
					parent->child_right = nullptr;
			}

			privateRemove(node);
			num_nodes_--;

			if (node->parent)
				privateRemove(node->parent);

			if (node == base_node_)
				base_node_ = nullptr;
		}
		else
		{
			if (node->child_left && node->child_right)
				return;

			BVHNode* child  = node->child_left ? node->child_left : node->child_right;
			BVHNode* parent = node->parent;
			child->parent   = parent;

			if (node == base_node_)
				base_node_ = child;

			if (parent)
			{
				if (node == parent->child_left)
					parent->child_left = child;
				else
					parent->child_right = child;
			}

			privateRemove(node);
			num_nodes_--;

			remove(parent);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	void BaseBVH::refit(BVHNode* node)
	{
		if (!node->is_leaf)
		{
			if (node->child_left && node->child_right)
				node->aabb = node->child_left->aabb.combine(node->child_right->aabb);
			else if (node->child_left)
				node->aabb = node->child_left->aabb;
			else if (node->child_right)
				node->aabb = node->child_right->aabb;
		}
	
		if (node->parent)
			refit(node->parent);
	}

	///////////////////////////////////////////////////////////////////////////
	Vector<void*> BaseBVH::getAllUserDataInAABB(const BVHAABB& aabb) const
	{
		Vector<void*> user_datas;

		if (base_node_)
			getUserDataInAABB(user_datas, aabb, base_node_);

		return user_datas;
	}

	///////////////////////////////////////////////////////////////////////////
	Vector<entity::Entity> BaseBVH::getAllEntityInAABB(const BVHAABB& aabb) const
	{
		Vector<entity::Entity> entities;

		if (base_node_)
			getEntityInAABB(entities, aabb, base_node_);
		
		return entities;
	}

	///////////////////////////////////////////////////////////////////////////
	void BaseBVH::getUserDataInAABB(Vector<void*>& user_datas, const BVHAABB& aabb, BVHNode* node) const
	{
		BVHNode* n = node;
		if (aabb.intersects(n->aabb))
		{
			if (!n->is_leaf)
			{
				if (n->child_left)
					getUserDataInAABB(user_datas, aabb, n->child_left);
				if (n->child_right)
					getUserDataInAABB(user_datas, aabb, n->child_right);
			}
			else if (n->user_data)
				user_datas.push_back(n->user_data);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	void BaseBVH::getEntityInAABB(Vector<entity::Entity>& entities, const BVHAABB& aabb, BVHNode* node) const
	{
		BVHNode* n = node;
		if (aabb.intersects(n->aabb))
		{
			if (!n->is_leaf)
			{
				if (n->child_left)
					getEntityInAABB(entities, aabb, n->child_left);
				if (n->child_right)
					getEntityInAABB(entities, aabb, n->child_right);
			}
			else
				entities.push_back(n->entity);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	static const uint32_t kColorCount = 12u;
	static const glm::vec4 kColors[kColorCount] = {
		{ 1.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 0.5f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 0.0f, 1.0f },
		{ 0.5f, 1.0f, 0.0f, 1.0f },

		{ 0.0f, 1.0f, 0.0f, 1.0f },
		{ 0.0f, 1.0f, 0.5f, 1.0f },
		{ 0.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.5f, 1.0f, 1.0f },

		{ 0.0f, 0.0f, 1.0f, 1.0f },
		{ 0.5f, 0.0f, 1.0f, 1.0f },
		{ 1.0f, 0.0f, 1.0f, 1.0f },
		{ 1.0f, 0.0f, 0.5f, 1.0f }
	};

	///////////////////////////////////////////////////////////////////////////
	void BaseBVH::insert(BVHNode* node, BVHNode* parent)
	{
		BVHNode* p = parent;

		// Found an end.
		if (p->is_leaf)
		{
			num_nodes_++;
			BVHNode* copy_node = privateCreate();
			copy_node->aabb      = p->aabb;
			copy_node->entity    = p->entity;
			copy_node->user_data = p->user_data;
			copy_node->parent    = p;

			p->is_leaf     = false;
			p->user_data   = nullptr;
			p->entity      = entity::InvalidEntity;
			p->child_left  = copy_node;
			p->child_right = node;
			node->parent   = p;
			
			refit(parent);
			return;
		}
		else
		{
			if (!p->child_left)
			{
				p->child_left = node;
				node->parent = p;
				refit(parent);
				return;
			}
			else if (!p->child_right)
			{
				p->child_right = node;
				node->parent   = p;
				refit(parent);
				return;
			}
			else
			{
				float left_length  = glm::length(p->child_left->aabb.combine(node->aabb).size);
				float right_length = glm::length(p->child_right->aabb.combine(node->aabb).size);
				
				if (left_length < right_length)
					insert(node, p->child_left);
				else
					insert(node, p->child_right);
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////
	void BaseBVH::draw(platform::DebugRenderer* renderer) const
	{
		if (base_node_)
			drawNode(base_node_, glm::vec2(0.0f), 0.0f, renderer);
	}

	///////////////////////////////////////////////////////////////////////////
	void BaseBVH::drawNode(BVHNode* node, size_t depth, platform::DebugRenderer* renderer) const
	{
		BVHNode* n = node;

		renderer->DrawLine(platform::DebugLine(
			glm::vec3(n->aabb.bl.x, depth / 100.0f, n->aabb.bl.y),
			glm::vec3(n->aabb.bl.x, depth / 100.0f, n->aabb.tr.y),
			kColors[depth % kColorCount]
		));
		renderer->DrawLine(platform::DebugLine(
			glm::vec3(n->aabb.bl.x, depth / 100.0f, n->aabb.tr.y),
			glm::vec3(n->aabb.tr.x, depth / 100.0f, n->aabb.tr.y),
			kColors[depth % kColorCount]
		));
		renderer->DrawLine(platform::DebugLine(
			glm::vec3(n->aabb.tr.x, depth / 100.0f, n->aabb.tr.y),
			glm::vec3(n->aabb.tr.x, depth / 100.0f, n->aabb.bl.y),
			kColors[depth % kColorCount]
		));
		renderer->DrawLine(platform::DebugLine(
			glm::vec3(n->aabb.tr.x, depth / 100.0f, n->aabb.bl.y),
			glm::vec3(n->aabb.bl.x, depth / 100.0f, n->aabb.bl.y),
			kColors[depth % kColorCount]
		));

		if (n->child_left)
			drawNode(n->child_left, depth + 1, renderer);
		if (n->child_right)
			drawNode(n->child_right, depth + 1, renderer);
	}

	///////////////////////////////////////////////////////////////////////////
	void BaseBVH::drawNode(BVHNode* node, glm::vec2 prev, float child, platform::DebugRenderer* renderer) const
	{
		BVHNode* n = node;

		glm::vec2 curr = prev;

		static constexpr float size = 0.125f;

		if (node != base_node_)
		{
			curr.y += size * 5.0f;
			curr.x += child * size * 2.0f;

			renderer->DrawLine(platform::DebugLine(
				glm::vec3(prev.x, 0.0f, prev.y),
				glm::vec3(curr.x, 0.0f, curr.y),
				kColors[0]
			));
		}

		if (n->is_leaf)
		{
			// Draw diamond.
			renderer->DrawLine(platform::DebugLine(
				glm::vec3(curr.x, 0.0f, curr.y),
				glm::vec3(curr.x + size, 0.0f, curr.y + size),
				kColors[1]
			));
			renderer->DrawLine(platform::DebugLine(
				glm::vec3(curr.x + size, 0.0f, curr.y + size),
				glm::vec3(curr.x, 0.0f, curr.y + size * 2.0f),
				kColors[1]
			));
			renderer->DrawLine(platform::DebugLine(
				glm::vec3(curr.x, 0.0f, curr.y + size * 2.0f),
				glm::vec3(curr.x - size, 0.0f, curr.y + size),
				kColors[1]
			));
			renderer->DrawLine(platform::DebugLine(
				glm::vec3(curr.x - size, 0.0f, curr.y + size),
				glm::vec3(curr.x, 0.0f, curr.y),
				kColors[1]
			));
		}
		else
		{
			// Draw triangle.
			renderer->DrawLine(platform::DebugLine(
				glm::vec3(curr.x, 0.0f, curr.y),
				glm::vec3(curr.x + size, 0.0f, curr.y + size),
				kColors[2]
			));
			renderer->DrawLine(platform::DebugLine(
				glm::vec3(curr.x + size, 0.0f, curr.y + size),
				glm::vec3(curr.x - size, 0.0f, curr.y + size),
				kColors[2]
			));
			renderer->DrawLine(platform::DebugLine(
				glm::vec3(curr.x, 0.0f, curr.y),
				glm::vec3(curr.x - size, 0.0f, curr.y + size),
				kColors[2]
			));
		}

		if (n->child_left)
			drawNode(n->child_left, curr + glm::vec2(-size, size), -1.0f, renderer);
		if (n->child_right)
			drawNode(n->child_right, curr + glm::vec2(size, size), 1.0f, renderer);
	}

	///////////////////////////////////////////////////////////////////////////
	void BVH::privateRemove(BVHNode* node)
	{
		foundation::Memory::destruct(node);
	}

	///////////////////////////////////////////////////////////////////////////
	BVHNode* BVH::privateCreate()
	{
		BVHNode* node = foundation::Memory::construct<BVHNode>();
		node->entity      = entity::InvalidEntity;
		node->user_data   = nullptr;
		node->parent      = nullptr;
		node->child_left  = nullptr;
		node->child_right = nullptr;
		node->aabb        = {};
		node->is_leaf     = true;
		return node;
	}








	///////////////////////////////////////////////////////////////////////////
	void TransientBVH::privateRemove(BVHNode* node)
	{
	}

	///////////////////////////////////////////////////////////////////////////
	BVHNode* TransientBVH::privateCreate()
	{
		BVHNode* node = foundation::GetFrameHeap()->construct<BVHNode>();
		node->entity      = entity::InvalidEntity;
		node->user_data   = nullptr;
		node->parent      = nullptr;
		node->child_left  = nullptr;
		node->child_right = nullptr;
		node->aabb        = {};
		node->is_leaf     = true;
		return node;
	}














	///////////////////////////////////////////////////////////////////////////
	BVHAABB::BVHAABB()
	{
	}

	///////////////////////////////////////////////////////////////////////////
	BVHAABB::BVHAABB(const glm::vec2& bl, const glm::vec2& tr)
		: bl(bl)
		, tr(tr)
		, center((bl + tr) * 0.5f)
		, size(tr - bl)
	{
	}

	///////////////////////////////////////////////////////////////////////////
	BVHAABB::BVHAABB(const BVHAABB& other)
		: bl(other.bl)
		, tr(other.tr)
		, center(other.center)
		, size(other.size)
	{
	}

	///////////////////////////////////////////////////////////////////////////
	BVHAABB BVHAABB::combine(const BVHAABB& other) const
	{
		return BVHAABB(glm::min(bl, other.bl), glm::max(tr, other.tr));
	}

#pragma optimize ("", off)
	///////////////////////////////////////////////////////////////////////////
	bool BVHAABB::intersects(const BVHAABB& other) const
	{
		glm::vec2 abs_center = glm::abs(center - other.center);
		glm::vec2 half_size  = (size + other.size) * 0.5f;
		return abs_center.x < half_size.x && abs_center.y < half_size.y;
	}
  }
}