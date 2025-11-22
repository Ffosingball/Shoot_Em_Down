#pragma once
#include "ECSGame.h"
#include "Entity.h"
#include "EntityManager.h"
#include "SceneNode.h"
#include <memory>
#include "Components.h"
#include "Component.h"
#include <SFML/Graphics.hpp>
#include "GameState.h"

//Helper functions
std::weak_ptr<Entity> CreateNewEntityAtRoot(const std::string name);
std::weak_ptr<Entity> CreateNewEntityAt(const std::string nodeName, const std::string newEntityName);
std::shared_ptr<CameraComponent> GetCameraFromCameraEntity();
std::shared_ptr<CameraComponent> GetCameraFromUICameraEntity();
void SetNewPosition(std::weak_ptr<Entity> entity, const sf::Vector2f position);
void SetupRectangleShape(std::shared_ptr<RectangleShapeComponent> recShape, const sf::Vector2f size, const std::string texturePath);

//Initializing functions
void InitializeTileMapAndCamera(const sf::Vector2u& windowSize);
void InitializeUICamera(const sf::Vector2u& windowSize);
void InitializePlayer(const sf::Vector2u& windowSize);
void CreateBullet(Entity* firedByPtr);
void CreateEnemy(const sf::Vector2f position);
void CreateExplosion(const sf::Vector2f position);
void CreateShield();
void CreatePowerUp(int powerUpType, const sf::Vector2f position);

//Reset functions
void ResetCamera();
void ResetPlayer(const sf::Vector2u& windowSize);
void ResetShield();

//UI functions
void InitializeStartUI();
void InitializePauseText();
void InitializeGameEndText(int shipsDestroyed, int bulletsAvoided, int bulletsFired);
void InitializeGameUI();
void InitializeDurabilityBars(int durability);
void CreateDifficultyLevelText(const DifficultyLevel level);
void ChangeDifficultyLevelText(const DifficultyLevel currentLevel, const DifficultyLevel previousLevel);

//Animation functions
void MoveStartText();
void MovePauseText();
void MoveGameEndText();
void RemoveGameUI();

//Remove components functions
void RemovePlayerComponent();