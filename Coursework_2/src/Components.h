#pragma once

#include <memory>
#include "Component.h"
#include "Entity.h"
#include <SFML/Graphics.hpp>
#include "TileMap.h"
#include "ParticleSystem.h"

//List of all components
enum class ComponentType
{
	Movement,
	RectangleShape,
	TileMap,
	Text,
	Camera,
	Player,
	Bullet,
	Enemy,
	Particles,
	Shield,
	UIPart
};


//Component for free movement
class MovementComponent : public Component
{
public:

	sf::Vector2f velocity{0.f,0.f};

	ComponentType GetComponentType() const override;
};

//Component for the player stats
class PlayerComponent : public Component
{
public:

	int hp{0};
	//Invulnerable to bullets certain period of time
	bool invulnerable{false};
	float invulnerabilityTimeLeft{ 0.f };
	//Invulnerable to other airplanes certain amount of times
	int durability{ 0 };

	ComponentType GetComponentType() const override;
};

//Component of the bullet
class BulletComponent : public Component
{
public:
	bool firedByEnemy{ false };

	ComponentType GetComponentType() const override;
};

//Component of the tilemap
class TileMapComponent : public Component
{
public:

	TileMap tileMap;

	ComponentType GetComponentType() const override;
};

//Component of the camera
class CameraComponent : public Component
{
public:

	sf::View view;
	bool moveCamera{ false };
	float rightBorder{0.f};
	float leftBorder{ 0.f };

	ComponentType GetComponentType() const override;
};

//Component of the rectangular object
class RectangleShapeComponent : public Component
{
public:

	sf::RectangleShape shape;

	ComponentType GetComponentType() const override;
};

//Text component which has text
class TextComponent : public Component
{
public:
	//Stores text
	std::shared_ptr<sf::Text> text;

	ComponentType GetComponentType() const override;
};

//UI component which has info to move or blink entity
class UIPartComponent : public Component
{
public:
	//Properties to make text to blink
	bool isBlinking{ false };
	float blinkingPeriod{ 1.f };
	bool decreasingVisibility{ false };
	float blinkTime{ 0.f };
	bool flatLine{ false };
	float flatLinePeriod{ 0.5f };
	//Properties to move text
	bool moveIt{ false };
	bool destroyAtTarget{ false };
	sf::Vector2f targetPosition{ 0.f,0.f };

	ComponentType GetComponentType() const override;
};

//Component of the enemy
class EnemyComponent : public Component
{
public:
	float timeLeftToFire{ 1.f };

	ComponentType GetComponentType() const override;
};

//Component of particles
class ParticlesComponent : public Component
{
public:
	ParticleSystem particleSystems;

	ComponentType GetComponentType() const override;
};

//Component of shield
class ShieldComponent : public Component 
{
public:
	//Sound properties
	float currentSoundTime{ 0.f };
	float soundPeriod{ 1.f };
	//Blinking properties
	float currentTime{0.5f};
	float blinkingPeriod{ 0.5f };
	bool decreasingVisibility{ true };

	ComponentType GetComponentType() const override;
};


//Shortcut functions to get specific component from the entity
std::shared_ptr<MovementComponent> GetMovementComponent(const Entity& entity);

std::shared_ptr<TileMapComponent> GetTileMapComponent(const Entity& entity);

std::shared_ptr<RectangleShapeComponent> GetRectangleShapeComponent(const Entity& entity);

std::shared_ptr<TextComponent> GetTextComponent(const Entity& entity);

std::shared_ptr<CameraComponent> GetCameraComponent(const Entity& entity);

std::shared_ptr<PlayerComponent> GetPlayerComponent(const Entity& entity);

std::shared_ptr<BulletComponent> GetBulletComponent(const Entity& entity);

std::shared_ptr<EnemyComponent> GetEnemyComponent(const Entity& entity);

std::shared_ptr<ParticlesComponent> GetParticlesComponent(const Entity& entity);

std::shared_ptr<ShieldComponent> GetShieldComponent(const Entity& entity);

std::shared_ptr<UIPartComponent> GetUIPartComponent(const Entity& entity);