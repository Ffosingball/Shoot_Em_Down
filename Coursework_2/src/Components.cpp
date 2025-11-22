#include <memory>
#include "Component.h"
#include "Components.h"
#include "Entity.h"
#include <SFML/Graphics.hpp>


//Component to move entities
ComponentType MovementComponent::GetComponentType() const
{
	return ComponentType::Movement;
}

//Component to render tileMap
ComponentType TileMapComponent::GetComponentType() const
{
	return ComponentType::TileMap;
}

//Component to render rectangles
ComponentType RectangleShapeComponent::GetComponentType() const
{
	return ComponentType::RectangleShape;
}

//Component for text
ComponentType TextComponent::GetComponentType() const
{
	return ComponentType::Text;
}

//Component of camera
ComponentType CameraComponent::GetComponentType() const
{
	return ComponentType::Camera;
}

//Player component
ComponentType PlayerComponent::GetComponentType() const
{
	return ComponentType::Player;
}

//Bullet component
ComponentType BulletComponent::GetComponentType() const
{
	return ComponentType::Bullet;
}

//Enemy component
ComponentType EnemyComponent::GetComponentType() const
{
	return ComponentType::Enemy;
}

//Particles component
ComponentType ParticlesComponent::GetComponentType() const
{
	return ComponentType::Particles;
}

//Component of the shield
ComponentType ShieldComponent::GetComponentType() const
{
	return ComponentType::Shield;
}

//Component to render UI parts
ComponentType UIPartComponent::GetComponentType() const
{
	return ComponentType::UIPart;
}


//This function creates any component and return shared pointer to it
std::shared_ptr<Component> ComponentFactory(ComponentType ct) 
{
	switch (ct)
	{
	case ComponentType::Movement:
		return std::make_shared<MovementComponent>();
	case ComponentType::RectangleShape:
		return std::make_shared<RectangleShapeComponent>();
	case ComponentType::TileMap:
		return std::make_shared<TileMapComponent>();
	case ComponentType::Text:
		return std::make_shared<TextComponent>();
	case ComponentType::Camera:
		return std::make_shared<CameraComponent>();
	case ComponentType::Player:
		return std::make_shared<PlayerComponent>();
	case ComponentType::Bullet:
		return std::make_shared<BulletComponent>();
	case ComponentType::Enemy:
		return std::make_shared<EnemyComponent>();
	case ComponentType::Particles:
		return std::make_shared<ParticlesComponent>();
	case ComponentType::Shield:
		return std::make_shared<ShieldComponent>();
	case ComponentType::UIPart:
		return std::make_shared<UIPartComponent>();
	}

	return {};
}

//This function translates componentType into string
std::string PrintComponentName(ComponentType ct) 
{
	switch (ct)
	{
	case ComponentType::Movement:
		return "Movement";
	case ComponentType::RectangleShape:
		return "RectangleShape";
	case ComponentType::TileMap:
		return "TileMap";
	case ComponentType::Text:
		return "Text";
	case ComponentType::Camera:
		return "Camera";
	case ComponentType::Player:
		return "Player";
	case ComponentType::Bullet:
		return "Bullet";
	case ComponentType::Enemy:
		return "Enemy";
	case ComponentType::Particles:
		return "Particles";
	case ComponentType::Shield:
		return "Shield";
	case ComponentType::UIPart:
		return "UIPart";
	}

	return "---";
}


//These functions just return specific component of the entity
std::shared_ptr<MovementComponent> GetMovementComponent(const Entity& entity)
{
	std::weak_ptr<Component> wpComponentBase = entity.FindComponent(ComponentType::Movement);
	std::shared_ptr<Component> spComponentBase = wpComponentBase.lock();
	return std::static_pointer_cast<MovementComponent>(spComponentBase);
}

std::shared_ptr<TileMapComponent> GetTileMapComponent(const Entity& entity)
{
	std::weak_ptr<Component> wpComponentBase = entity.FindComponent(ComponentType::TileMap);
	std::shared_ptr<Component> spComponentBase = wpComponentBase.lock();
	return std::static_pointer_cast<TileMapComponent>(spComponentBase);
}

std::shared_ptr<RectangleShapeComponent> GetRectangleShapeComponent(const Entity& entity)
{
	std::weak_ptr<Component> wpComponentBase = entity.FindComponent(ComponentType::RectangleShape);
	std::shared_ptr<Component> spComponentBase = wpComponentBase.lock();
	return std::static_pointer_cast<RectangleShapeComponent>(spComponentBase);
}

std::shared_ptr<TextComponent> GetTextComponent(const Entity& entity)
{
	std::weak_ptr<Component> wpComponentBase = entity.FindComponent(ComponentType::Text);
	std::shared_ptr<Component> spComponentBase = wpComponentBase.lock();
	return std::static_pointer_cast<TextComponent>(spComponentBase);
}

std::shared_ptr<CameraComponent> GetCameraComponent(const Entity& entity)
{
	std::weak_ptr<Component> wpComponentBase = entity.FindComponent(ComponentType::Camera);
	std::shared_ptr<Component> spComponentBase = wpComponentBase.lock();
	return std::static_pointer_cast<CameraComponent>(spComponentBase);
}

std::shared_ptr<PlayerComponent> GetPlayerComponent(const Entity& entity)
{
	std::weak_ptr<Component> wpComponentBase = entity.FindComponent(ComponentType::Player);
	std::shared_ptr<Component> spComponentBase = wpComponentBase.lock();
	return std::static_pointer_cast<PlayerComponent>(spComponentBase);
}

std::shared_ptr<BulletComponent> GetBulletComponent(const Entity& entity)
{
	std::weak_ptr<Component> wpComponentBase = entity.FindComponent(ComponentType::Bullet);
	std::shared_ptr<Component> spComponentBase = wpComponentBase.lock();
	return std::static_pointer_cast<BulletComponent>(spComponentBase);
}

std::shared_ptr<EnemyComponent> GetEnemyComponent(const Entity& entity)
{
	std::weak_ptr<Component> wpComponentBase = entity.FindComponent(ComponentType::Enemy);
	std::shared_ptr<Component> spComponentBase = wpComponentBase.lock();
	return std::static_pointer_cast<EnemyComponent>(spComponentBase);
}

std::shared_ptr<ParticlesComponent> GetParticlesComponent(const Entity& entity)
{
	std::weak_ptr<Component> wpComponentBase = entity.FindComponent(ComponentType::Particles);
	std::shared_ptr<Component> spComponentBase = wpComponentBase.lock();
	return std::static_pointer_cast<ParticlesComponent>(spComponentBase);
}

std::shared_ptr<ShieldComponent> GetShieldComponent(const Entity& entity)
{
	std::weak_ptr<Component> wpComponentBase = entity.FindComponent(ComponentType::Shield);
	std::shared_ptr<Component> spComponentBase = wpComponentBase.lock();
	return std::static_pointer_cast<ShieldComponent>(spComponentBase);
}

std::shared_ptr<UIPartComponent> GetUIPartComponent(const Entity& entity)
{
	std::weak_ptr<Component> wpComponentBase = entity.FindComponent(ComponentType::UIPart);
	std::shared_ptr<Component> spComponentBase = wpComponentBase.lock();
	return std::static_pointer_cast<UIPartComponent>(spComponentBase);
}