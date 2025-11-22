#include "EntitiesFunctions.h"
#include "ECSGame.h"
#include "Entity.h"
#include "EntityManager.h"
#include "SceneNode.h"
#include <memory>
#include "Components.h"
#include "Component.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <random>
#include "ResourceManager.h"
#include "CommonGameCode.h"
#include "GameState.h"
#include "ParticlesConfigurations.h"


//This function creates an entity at the root node and return smart pointer to it
//String is not reference, because rvalues cannot be referenced, I use them a lot
std::weak_ptr<Entity> CreateNewEntityAtRoot(const std::string name) 
{
	// Create the entity in the entity manager
	std::weak_ptr<Entity> wpEntity = ECSGame::Instance().GetEntityManager().NewEntity(name);
	// Add it to the scene
	ECSGame::Instance().GetSceneRoot().AddChild(SceneNode(wpEntity));
	return wpEntity;
}


//This function creates an entity at node with an entity with provided name
//and return smart pointer to it
//String is not reference, because rvalues cannot be referenced, I use them a lot
std::weak_ptr<Entity> CreateNewEntityAt(const std::string nodeName, const std::string newEntityName)
{
	std::weak_ptr<Entity> wpEntity;
	//Get node with entity of that name
	std::weak_ptr<Entity> wSpEntity = ECSGame::Instance().GetEntityManager().FindEntity(nodeName);
	std::shared_ptr<Entity> sSpEntity = wSpEntity.lock();
	SceneNode* nodePtr = ECSGame::Instance().GetSceneRoot().FindChild(*sSpEntity);
	//Check if node with that entity exists
	if (nodePtr != nullptr)
	{
		// Create the entity in the entity manager
		wpEntity = ECSGame::Instance().GetEntityManager().NewEntity(newEntityName);
		// Add it to the scene
		nodePtr->AddChild(SceneNode(wpEntity));
	}

	return wpEntity;
}

//Returns camera component from the camera entity
std::shared_ptr<CameraComponent> GetCameraFromCameraEntity() 
{
	std::weak_ptr<Entity> wCamera = ECSGame::Instance().GetEntityManager().FindEntity("Camera");
	std::shared_ptr<Entity> sCamera = wCamera.lock();
	return GetCameraComponent(*sCamera);
}


//Returns camera component from the UIcamera entity
std::shared_ptr<CameraComponent> GetCameraFromUICameraEntity()
{
	std::weak_ptr<Entity> wCamera = ECSGame::Instance().GetEntityManager().FindEntity("UICamera");
	std::shared_ptr<Entity> sCamera = wCamera.lock();
	return GetCameraComponent(*sCamera);
}


//This function set new position of this entity
void SetNewPosition(std::weak_ptr<Entity> entity, const sf::Vector2f position)
{
	std::shared_ptr<Entity> sEntity = entity.lock();
	sf::Transformable sTransform = sEntity->GetTransformable();
	//Set position
	sTransform.setPosition(position);
	sEntity->SetTransformable(sTransform);
}


void SetupRectangleShape(std::shared_ptr<RectangleShapeComponent> recShape, const sf::Vector2f size, const std::string texturePath)
{
	recShape->shape = sf::RectangleShape(size);
	recShape->shape.setOrigin(size / 2.f);
	std::weak_ptr<sf::Texture> wTexture = ResourceManager::Instance().LoadTexture(texturePath);
	std::shared_ptr<sf::Texture> sTexture = wTexture.lock();
	recShape->shape.setTexture(sTexture.get());
}

void ResetCamera()
{
	//get camera
	std::weak_ptr<Entity> wCamera = ECSGame::Instance().GetEntityManager().FindEntity("Camera");
	std::shared_ptr<Entity> sCamera = wCamera.lock();
	//Get component
	std::shared_ptr<MovementComponent> spCameraMovCom = GetMovementComponent(*sCamera);
	//Reset CAMERA VELOCITY
	spCameraMovCom->velocity = sf::Vector2f{ 10.f,0.f };
}

//Creates a tilemap and camera
void InitializeTileMapAndCamera(const sf::Vector2u& windowSize)
{
	//Create tileMap
	std::shared_ptr<Entity> spTileMap = CreateNewEntityAtRoot("TileMap").lock();
	// Add components
	spTileMap->AddComponent(ComponentType::TileMap);
	//Get component
	std::shared_ptr<TileMapComponent> spTileMapCom = GetTileMapComponent(*spTileMap);
	//set tilemap properties
	spTileMapCom->tileMap.tileSize = sf::Vector2i{16,16};
	spTileMapCom->tileMap.marginSize = sf::Vector2i{ 0,0 };
	spTileMapCom->tileMap.paddingSize = sf::Vector2i{ 1,1 };
	spTileMapCom->tileMap.numTilesInTileset = sf::Vector2i{ 12,10 };
	spTileMapCom->tileMap.tileSetPath = "media/other/kenney_shmup_tilemap.csv";
	//Iitialize all tiles
	spTileMapCom->tileMap.Initialize();

	//Create camera
	std::shared_ptr<Entity> spCamera = CreateNewEntityAtRoot("Camera").lock();
	//Add component
	spCamera->AddComponent(ComponentType::Camera);
	spCamera->AddComponent(ComponentType::Movement);
	//Get component
	std::shared_ptr<CameraComponent> spCameraCom = GetCameraComponent(*spCamera);
	//set camera properties
	sf::Vector2f mapSize = static_cast<sf::Vector2f>(spTileMapCom->tileMap.getMapSize());
	float windowSizeRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
	//Set camera sizes
	spCameraCom->view.setSize(sf::Vector2f{mapSize.y*windowSizeRatio,mapSize.y});
	spCameraCom->view.setCenter(sf::Vector2f{ mapSize.y * (windowSizeRatio/2.f)*1.f,mapSize.y/2.f });
	//Set boundaries
	spCameraCom->rightBorder = mapSize.x - ((mapSize.y * windowSizeRatio)/2.f);
	spCameraCom->leftBorder = spCameraCom->view.getCenter().x;
}


//Creates UI camera
void InitializeUICamera(const sf::Vector2u& windowSize)
{
	//Create camera
	std::shared_ptr<Entity> spCamera = CreateNewEntityAtRoot("UICamera").lock();
	//Add component
	spCamera->AddComponent(ComponentType::Camera);
	//Get component
	std::shared_ptr<CameraComponent> spCameraCom = GetCameraComponent(*spCamera);
	//set camera properties
	spCameraCom->view.setSize(static_cast<sf::Vector2f>(windowSize));
	spCameraCom->view.setCenter(static_cast<sf::Vector2f>(windowSize)/2.f);
}


void ResetPlayer(const sf::Vector2u& windowSize)
{
	//Get player`s component
	std::weak_ptr<Entity> wPlayer = ECSGame::Instance().GetEntityManager().FindEntity("Player");
	std::shared_ptr<Entity> sPlayer = wPlayer.lock();
	std::shared_ptr<MovementComponent> sMovCom = GetMovementComponent(*sPlayer);
	//Get tilemap sizes
	std::weak_ptr<Entity> wTL = ECSGame::Instance().GetEntityManager().FindEntity("TileMap");
	std::shared_ptr<Entity> sTL = wTL.lock();
	std::shared_ptr<TileMapComponent> sTileMapCom = GetTileMapComponent(*sTL);
	//Set position
	SetNewPosition(wPlayer, { -8.f,sTileMapCom->tileMap.getMapSize().y * 0.5f });

	//Check if player does not have player component, then add it
	if (!sPlayer->HasComponent(ComponentType::Player))
		sPlayer->AddComponent(ComponentType::Player);

	//set movement speed
	sMovCom->velocity = sf::Vector2f{ 100.f,100.f };
	//Get component
	std::shared_ptr<PlayerComponent> spPlayerCom = GetPlayerComponent(*sPlayer);
	//Set player properties
	spPlayerCom->hp = 3;
	spPlayerCom->invulnerable = false;
}


//Create player
void InitializePlayer(const sf::Vector2u& windowSize)
{
	//Create player
	std::shared_ptr<Entity> spPlayer = CreateNewEntityAt("PlayerNode", "Player").lock();
	// Add components
	spPlayer->AddComponent(ComponentType::Movement);
	spPlayer->AddComponent(ComponentType::RectangleShape);
	spPlayer->AddComponent(ComponentType::Player);
	spPlayer->AddComponent(ComponentType::Particles);
	//Get components
	std::shared_ptr<RectangleShapeComponent> spPlayerShape = GetRectangleShapeComponent(*spPlayer);
	std::shared_ptr<ParticlesComponent> spParticles = GetParticlesComponent(*spPlayer);
	//set ptoperties
	//PLAYER SIZE
	sf::Vector2f size{ 22.f,28.f };
	SetupRectangleShape(spPlayerShape,size, "media/textures/player.png");
	//Set player position and other propertise
	ResetPlayer(windowSize);
	//create fumes particles
	spParticles->particleSystems.config = std::make_shared<Fumes>(false);
	spParticles->particleSystems.transformable.setPosition(sf::Vector2f(- size.x, size.y / -2.f));
	spParticles->particleSystems.config->pTexture = ResourceManager::Instance().LoadTexture("media/textures/particle2.png").lock().get();
	spParticles->particleSystems.Start();
}


//Create a bullet
void CreateBullet(Entity* firedByPtr) 
{
	//Create a bullet
	std::shared_ptr<Entity> spBullet = CreateNewEntityAt("Bullets", "Bullet").lock();
	//Add components
	spBullet->AddComponent(ComponentType::Movement);
	spBullet->AddComponent(ComponentType::RectangleShape);
	spBullet->AddComponent(ComponentType::Bullet);
	//Get component
	std::shared_ptr<RectangleShapeComponent> spBulletShape = GetRectangleShapeComponent(*spBullet);
	std::shared_ptr<MovementComponent> spBulletMovement = GetMovementComponent(*spBullet);
	std::shared_ptr<BulletComponent> spBulletCom = GetBulletComponent(*spBullet);
	//setup rectangleShape component
	sf::Vector2f size{ 8.f,8.f };
	SetupRectangleShape(spBulletShape,size, "media/textures/bullet.png");
	//Get player position
	std::weak_ptr<Entity> wPlayer = ECSGame::Instance().GetEntityManager().FindEntity("Player");
	std::shared_ptr<Entity> sPlayer = wPlayer.lock();

	//Check if player still alive
	if (sPlayer != nullptr)
	{
		//Get player tranform
		sf::Transformable transformablePlayer = sPlayer->GetTransformable();

		//Check if bullet was fired by player or not
		if (firedByPtr == nullptr)
		{
			//Set position
			SetNewPosition(spBullet, { transformablePlayer.getPosition().x,transformablePlayer.getPosition().y });
			//BULLET SPEED FROM PLAYER
			spBulletMovement->velocity = sf::Vector2f{ 300.f,0.f };
			//Set that it was not fired by enemy
			spBulletCom->firedByEnemy = false;
		}
		else
		{
			//Fired by enemy
			//Get enemy position
			sf::Transformable transformableEnemy = firedByPtr->GetTransformable();

			//BULLET SPEED FROM ENEMY
			float horizontalVelocity = 120.f;
			//Calculate vertical velocity
			float time = (transformableEnemy.getPosition().x - transformablePlayer.getPosition().x) / horizontalVelocity;
			float verticalVelocity = (transformableEnemy.getPosition().y - transformablePlayer.getPosition().y) / time;
			//Check that vertical speed do not exceed horizontal speed
			if (verticalVelocity > horizontalVelocity)
				verticalVelocity = horizontalVelocity;
			else if (verticalVelocity < -horizontalVelocity)
				verticalVelocity = -horizontalVelocity;

			//Set position
			SetNewPosition(spBullet, { transformableEnemy.getPosition().x,transformableEnemy.getPosition().y + 8.f });
			//set movement speed
			spBulletMovement->velocity = sf::Vector2f{ -horizontalVelocity,-verticalVelocity };
			//Set that it was fired by enemy
			spBulletCom->firedByEnemy = true;
		}
	}
}

//Create enemy plane
void CreateEnemy(const sf::Vector2f position) 
{
	//Create enemy
	std::shared_ptr<Entity> spEnemy = CreateNewEntityAt("Enemies","Enemy").lock();
	// Add components
	spEnemy->AddComponent(ComponentType::Movement);
	spEnemy->AddComponent(ComponentType::RectangleShape);
	spEnemy->AddComponent(ComponentType::Enemy);
	spEnemy->AddComponent(ComponentType::Particles);
	//Get components
	std::shared_ptr<MovementComponent> spEnemyMovement = GetMovementComponent(*spEnemy);
	std::shared_ptr<RectangleShapeComponent> spEnemyShape = GetRectangleShapeComponent(*spEnemy);
	std::shared_ptr<ParticlesComponent> spParticles = GetParticlesComponent(*spEnemy);
	//set MOVEMENT SPEED
	spEnemyMovement->velocity = sf::Vector2f{ -20.f+gel::Randf(-5.f,5.f),gel::Randf(-2.f,2.f) };
	//Setup RectangleShape
	sf::Vector2f size{ 30.f,30.f };
	SetupRectangleShape(spEnemyShape, size, "media/textures/enemy.png");
	//Set position
	SetNewPosition(spEnemy,position);
	//create fumes particles
	spParticles->particleSystems.config = std::make_shared<Fumes>(true);
	spParticles->particleSystems.transformable.setPosition(sf::Vector2f{ 0.f,size.y / -2.f });
	spParticles->particleSystems.config->pTexture = ResourceManager::Instance().LoadTexture("media/textures/particle2.png").lock().get();
	spParticles->particleSystems.Start();
}

//Create explosion particles
void CreateExplosion(const sf::Vector2f position) 
{
	//Create explosion
	std::shared_ptr<Entity> spExplosion = CreateNewEntityAt("Particles", "Explosion").lock();
	// Add components
	spExplosion->AddComponent(ComponentType::Particles);
	//Get component
	std::shared_ptr<ParticlesComponent> spParticles = GetParticlesComponent(*spExplosion);
	//create explosion particles
	spParticles->particleSystems.config = std::make_shared<Explosion>();
	spParticles->particleSystems.transformable.setPosition(position);
	spParticles->particleSystems.config->pTexture = ResourceManager::Instance().LoadTexture("media/textures/particle2.png").lock().get();
	spParticles->particleSystems.Start();
}

//Make player object just usual entity, by removing player component
void RemovePlayerComponent() 
{
	//Get player
	std::weak_ptr<Entity> wPlayer = ECSGame::Instance().GetEntityManager().FindEntity("Player");
	std::shared_ptr<Entity> sPlayer = wPlayer.lock();
	//Check that player exist
	if (sPlayer != nullptr)
	{
		//Remove component
		sPlayer->RemoveComponent(ComponentType::Player);
		//Get component
		std::shared_ptr<MovementComponent> sMovCom = GetMovementComponent(*sPlayer);
		//set new movement speed
		sMovCom->velocity = sf::Vector2f{ 100.f,0.f };
	}
}

//Create shield
void CreateShield()
{
	//Create shield
	std::shared_ptr<Entity> spShield = CreateNewEntityAt("Player", "Shield").lock();
	// Add components
	spShield->AddComponent(ComponentType::RectangleShape);
	spShield->AddComponent(ComponentType::Shield);
	//Get components
	std::shared_ptr<RectangleShapeComponent> spShape = GetRectangleShapeComponent(*spShield);
	//set ptoperties
	//SHIELD SIZE
	sf::Vector2f size{ 32.f,32.f };
	SetupRectangleShape(spShape, size, "media/textures/shield.png");
}

//Reset shield properties
void ResetShield() 
{
	//Get player node
	std::shared_ptr<Entity> spPlayer = ECSGame::Instance().GetEntityManager().FindEntity("Player").lock();
	SceneNode* playerNodePtr = ECSGame::Instance().GetSceneRoot().FindChild(*spPlayer);
	//Get shield
	std::weak_ptr<Entity> wpShield = ECSGame::Instance().GetEntityManager().FindEntity("Shield");
	std::shared_ptr<Entity> spShield = wpShield.lock();
	//If shield is not child of the player node, than fix it
	if (playerNodePtr->FindChild(*spShield) == nullptr)
		playerNodePtr->AddChild(SceneNode(wpShield));

	//Get components
	std::shared_ptr<ShieldComponent> spShieldCom = GetShieldComponent(*spShield);
	std::shared_ptr<RectangleShapeComponent> spShape = GetRectangleShapeComponent(*spShield);
	//Reset color
	sf::Color color = spShape->shape.getFillColor();
	color.a = 255;
	spShape->shape.setFillColor(color);
	//Reset shield properties
	spShieldCom->currentTime=0.5f;
	spShieldCom->blinkingPeriod=0.5f;
	spShieldCom->currentSoundTime=0.f;
	spShieldCom->soundPeriod=1.f;
	spShieldCom->decreasingVisibility=true;
}

//Create powerUps
void CreatePowerUp(int powerUpType, const sf::Vector2f position)
{
	//Create power up
	std::shared_ptr<Entity> spPowerUp = CreateNewEntityAt("PlayerNode", "PowerUp"+std::to_string(powerUpType)).lock();
	// Add components
	spPowerUp->AddComponent(ComponentType::RectangleShape);
	//Get components
	std::shared_ptr<RectangleShapeComponent> spShape = GetRectangleShapeComponent(*spPowerUp);
	//set ptoperties
	//SHIELD SIZE
	sf::Vector2f size{ 25.f,25.f };
	SetupRectangleShape(spShape, size, "media/textures/kenney_shmup_powerUps.png");
	//Set position
	SetNewPosition(spPowerUp,position);
	//Depending on power up type, set different texture
	if(powerUpType==1)
		spShape->shape.setTextureRect(sf::IntRect({ 0,0 }, { 16,16 }));
	else
		spShape->shape.setTextureRect(sf::IntRect({ 16,0 }, { 16,16 }));
}