#include <memory>
#include "Systems.h"
#include <SFML/Graphics.hpp>
#include "EntitiesFunctions.h"
#include "SceneNodeVisitors.h"
#include <iostream>
#include "CommonGameCode.h"
#include <SFML/Audio.hpp>
#include "ResourceManager.h"


//INPUT SYSTEM
void InputSystem::Initialize() 
{
	//Subscribe functions to these signals
	signals::onKeyPressed.connect(&InputSystem::OnKeyPressed, this);
	signals::onKeyReleased.connect(&InputSystem::OnKeyReleased, this);
}

//Process keys they are pressed
void InputSystem::OnKeyPressed(sf::Event::KeyPressed key) 
{
	if (key.code == sf::Keyboard::Key::Q && !keyQPressed) 
	{
		//Change game state
		if (ECSGame::Instance().GetGameState() == GameState::Game)
		{
			ECSGame::Instance().SetGameState(GameState::Pause);
			InitializePauseText();
		}
		else if (ECSGame::Instance().GetGameState() == GameState::Pause)
		{
			ECSGame::Instance().SetGameState(GameState::Game);
			//Move pause text
			MovePauseText();
		}
		else if (ECSGame::Instance().GetGameState() == GameState::Start) 
		{
			//If it start, then start plane animation
			ECSGame::Instance().SetGameState(GameState::Animation);
			InitializePlayer(ECSGame::Instance().GetWindowSize());
			//Remove start text
			MoveStartText();
			//Start moving camera
			ResetCamera();
			std::shared_ptr<CameraComponent> spCameraCom = GetCameraFromCameraEntity();
			spCameraCom->moveCamera = true;
		}

		keyQPressed = true;
	}
	else if (key.code == sf::Keyboard::Key::R && !keyRPressed)
	{
		//Ensures that game will not be restarted during animation or start state
		if (ECSGame::Instance().GetGameState() != GameState::Start && ECSGame::Instance().GetGameState() != GameState::Animation)
		{
			//Restart game and start camera animation
			ECSGame::Instance().SetGameState(GameState::Animation);
			//This is required, to fix weird bug with game UI text
			RemovePlayerComponent();
			signals::onGameRestart();
			keyRPressed = true;
		}
	}
	else if (key.code == sf::Keyboard::Key::Space && !keySpacePressed)
	{
		//Check if game is not paused
		if (ECSGame::Instance().GetGameState() == GameState::Game)
		{
			//Then fire a bullet
			signals::onPlayShotSound();
			CreateBullet(nullptr);
			signals::onBulletFired();
			keySpacePressed = true;
		}
	}
	else if (key.code == sf::Keyboard::Key::Escape)
	{
		//Close game
		ECSGame::Instance().CloseGame();
	}
	else if (key.code == sf::Keyboard::Key::Num1 && ECSGame::Instance().GetGameState()==GameState::Start)
	{
		//1 - is easy difficulty
		signals::onDifficultyLevelChange(DifficultyLevel::Easy);
	}
	else if (key.code == sf::Keyboard::Key::Num2 && ECSGame::Instance().GetGameState() == GameState::Start)
	{
		//2 - is medium difficulty
		signals::onDifficultyLevelChange(DifficultyLevel::Medium);
	}
	else if (key.code == sf::Keyboard::Key::Num3 && ECSGame::Instance().GetGameState() == GameState::Start)
	{
		//3 - is hard difficulty
		signals::onDifficultyLevelChange(DifficultyLevel::Hard);
	}
}


void InputSystem::OnKeyReleased(sf::Event::KeyReleased key)
{
	//If key released then reset bool values
	if (key.code == sf::Keyboard::Key::Q)
		keyQPressed = false;
	else if (key.code == sf::Keyboard::Key::Space)
		keySpacePressed = false;
	else if (key.code == sf::Keyboard::Key::R)
		keyRPressed = false;
}


//I process movement and fire keys in every frame, because game reacts to the key press
//on the same frame as it was pressed, and it will react every fram until the key
//is released. If I would use events, they are not called every frame, which is bad
void InputSystem::Update(SceneNode& scene, float deltaTime)
{
	//Set direction to 0.1,0
	sf::Vector2f direction{ 0.1f,0 };

	if (ECSGame::Instance().GetGameState() == GameState::Game)
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
		{
			//Change direction to positive
			direction.y -= 1.f;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
		{
			//Change direction to negative
			direction.y += 1.f;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
		{
			direction.x -= 1.f;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
		{
			direction.x += 1.f;
		}
	}
	else if(ECSGame::Instance().GetGameState() == GameState::Animation || ECSGame::Instance().GetGameState() == GameState::Won)
		direction.x += 1.f;

	//Signal the direction to the movement system
	signals::onMovePlayer(direction);
}


//MOVEMENT SYSTEM
void MovementSystem::Initialize()
{
	//Subscribe to this signal, to get player direction every frame
	signals::onMovePlayer.connect(&MovementSystem::OnMovePlayer, this);
}


void MovementSystem::Update(SceneNode& scene, float deltaTime) 
{
	//If game paused, then do nothing
	if (ECSGame::Instance().GetGameState() != GameState::Pause)
	{
		SceneNodeVisitorMovement visitor(*this);
		scene.AcceptVisitor(visitor);
	}
}


//UI SYSTEM
void UISystem::Initialize() 
{
	//Subscribe to some signals
	signals::onGameRestart.connect(&UISystem::OnGameRestart, this);
	signals::onPlayerHurt.connect(&UISystem::OnPlayerHurt, this);
	signals::onDurabilityUsed.connect(&UISystem::OnDurabilityUsed, this);
	signals::onDifficultyLevelChange.connect(&UISystem::OnDifficultyLevelChange, this);
	signals::onDifficultyLevelShow.connect(&UISystem::OnDifficultyLevelShow, this);
	//Show difficulty level text
	OnDifficultyLevelShow();
}

void UISystem::Update(SceneNode& scene, float deltaTime) 
{
	SceneNodeVisitorUI visitor(*this);
	scene.AcceptVisitor(visitor);
}

//Reset UI
void UISystem::OnGameRestart() 
{
	//Remove end game UI
	MoveGameEndText();

	//Check if game text still exist, if they exist then destroy them
	std::weak_ptr<Entity> wpT1 = ECSGame::Instance().GetEntityManager().FindEntity("GameText");
	if (wpT1.lock() == nullptr)
		signals::onDeleteEntity(wpT1);
	//Check if start text still exist, if they exist then destroy them
	std::weak_ptr<Entity> wpT2 = ECSGame::Instance().GetEntityManager().FindEntity("StartText");
	if (wpT2.lock() == nullptr)
		signals::onDeleteEntity(wpT2);
}

//Update hp circles
void UISystem::OnPlayerHurt(int hpLeft)
{
	//Get hp entity
	std::weak_ptr<Entity> wpEntity = ECSGame::Instance().GetEntityManager().FindEntity("HP"+std::to_string(hpLeft));
	std::shared_ptr<Entity> spEntity = wpEntity.lock();

	//Get component
	std::shared_ptr<RectangleShapeComponent> spShapeCom = GetRectangleShapeComponent(*spEntity);
	spShapeCom->shape.setTextureRect(sf::IntRect({21,0}, {20,20}));
}

//Update durability lines
void UISystem::OnDurabilityUsed(int durabilityLeft)
{
	//Get durability entity
	std::weak_ptr<Entity> wpEntity = ECSGame::Instance().GetEntityManager().FindEntity("Durability" + std::to_string(durabilityLeft));
	//Destroy it
	signals::onDeleteEntity(wpEntity);
}

void UISystem::OnDifficultyLevelChange(DifficultyLevel level)
{
	ChangeDifficultyLevelText(level, this->level);
	this->level = level;
}

void UISystem::OnDifficultyLevelShow()
{
	//Show difficulty level choosed
	CreateDifficultyLevelText(level);
}



//ENEMIES SYSTEM
void EnemiesSystem::Initialize()
{
	//Subscribe to this signal, to get player direction every frame
	signals::onDifficultyLevelChange.connect(&EnemiesSystem::OnDifficultyLevelChange, this);
}

void EnemiesSystem::Update(SceneNode& scene, float deltaTime)
{
	//If game paused, then do nothing
	if (ECSGame::Instance().GetGameState() != GameState::Pause && ECSGame::Instance().GetGameState() != GameState::Start)
	{
		//Spawn new enemies only when it is game state
		if (ECSGame::Instance().GetGameState() == GameState::Game)
		{
			timeLeftToSpawnNextEnemy -= deltaTime;

			if (timeLeftToSpawnNextEnemy < 0)
			{
				//Get camera component
				std::shared_ptr<CameraComponent> sCameraCom = GetCameraFromCameraEntity();
				//Get new random position
				float xOffset = 9.f;
				sf::Vector2f position{ 0.f,0.f };
				position.x = sCameraCom->view.getCenter().x + (sCameraCom->view.getSize().x / 2.f) + xOffset;
				position.y = gel::Randf(sCameraCom->view.getCenter().y - (sCameraCom->view.getSize().y / 2.f), sCameraCom->view.getCenter().y + (sCameraCom->view.getSize().y / 2.f));

				//Create new enemy at thet position
				CreateEnemy(position);
				timeLeftToSpawnNextEnemy += spawnEnemyPeriod;
			}
		}

		SceneNodeVisitorEnemy visitor(*this);
		scene.AcceptVisitor(visitor);
	}
}

//Change enemy spawn period and bullet fire period
void EnemiesSystem::OnDifficultyLevelChange(DifficultyLevel level) 
{
	switch (level) 
	{
	case DifficultyLevel::Easy:
		spawnEnemyPeriod = 2.f;
		firePeriod = 3.f;
		break;
	case DifficultyLevel::Medium:
		spawnEnemyPeriod = 1.f;
		firePeriod = 2.f;
		break;
	case DifficultyLevel::Hard:
		spawnEnemyPeriod = 0.6f;
		firePeriod = 1.5f;
		break;
	}
}


//COLLISIONS SYSTEM
void CollisionSystem::Initialize()
{
	//Subscribe to this signal, to get player direction every frame
	signals::onDifficultyLevelChange.connect(&CollisionSystem::OnDifficultyLevelChange, this);
}

void CollisionSystem::Update(SceneNode& scene, float deltaTime)
{
	//If game paused, then do nothing
	if (ECSGame::Instance().GetGameState() != GameState::Pause && ECSGame::Instance().GetGameState() != GameState::Start)
	{
		SceneNodeVisitorCollision visitor(*this);
		scene.AcceptVisitor(visitor);
	}
}

//Change power up spawn period
void CollisionSystem::OnDifficultyLevelChange(DifficultyLevel level)
{
	switch (level)
	{
	case DifficultyLevel::Easy:
		powerUpDropRate = 0.12f;
		powerUp1Correlation = 0.6f;
		break;
	case DifficultyLevel::Medium:
		powerUpDropRate = 0.075f;
		powerUp1Correlation = 0.6f;
		break;
	case DifficultyLevel::Hard:
		powerUpDropRate = 0.03f;
		powerUp1Correlation = 0.5f;
		break;
	}
}


//MUSIC SYSTEM
void MusicSystem::Initialize()
{
	//Subscribe to some signals
	signals::onGameRestart.connect(&MusicSystem::OnGameRestart, this);
	signals::onPlayExplosionSound.connect(&MusicSystem::OnPlayExplosionSound, this);
	signals::onPlayerHurt.connect(&MusicSystem::OnPlayHurtSound, this);
	signals::onPlayShotSound.connect(&MusicSystem::OnPlayShotSound, this);
	signals::onPlayWarningSound.connect(&MusicSystem::OnPlayWarningSound, this);
	signals::onPlayPowerUpSound.connect(&MusicSystem::OnPlayPowerUpSound, this);
	signals::onPlayManyExplosionsSounds.connect(&MusicSystem::OnPlayManyExplosionsSounds, this);
	signals::onPlayWonSound.connect(&MusicSystem::OnPlayWonSound, this);
	//Initialize music
	mainMusic = ResourceManager::Instance().LoadMusic("media/music/time_for_adventure.mp3").lock();
	mainMusic->setLooping(true);
	mainMusic->play();
	//Load sound
	spExplosionSound = std::make_shared<sf::Sound>(*ResourceManager::Instance().LoadSoundBuffer("media/sfx/explosion.wav").lock());
	spHurtSound = std::make_shared<sf::Sound>(*ResourceManager::Instance().LoadSoundBuffer("media/sfx/hurt.wav").lock());
	spShotSound = std::make_shared<sf::Sound>(*ResourceManager::Instance().LoadSoundBuffer("media/sfx/tap.wav").lock());
	spWarningSound = std::make_shared<sf::Sound>(*ResourceManager::Instance().LoadSoundBuffer("media/sfx/jump.wav").lock());
	spPowerUpSound = std::make_shared<sf::Sound>(*ResourceManager::Instance().LoadSoundBuffer("media/sfx/power_up.wav").lock());
	spWonSound = std::make_shared<sf::Sound>(*ResourceManager::Instance().LoadSoundBuffer("media/sfx/coin.wav").lock());
}

void MusicSystem::Update(SceneNode& scene, float deltaTime)
{
	//Check if we played all explosion sound
	if (playExplosionSound > 0)
	{
		timeLeftToPlaySound -= deltaTime;
		//Check when we need to play it again
		if (timeLeftToPlaySound <= 0)
		{
			spExplosionSound->play();
			timeLeftToPlaySound += gel::Randf(0.1f, 0.5f);
			playExplosionSound--;
		}
	}

	//Check if we played all won sound
	if (playWonSound > 0)
	{
		timeLeftToPlayWonSound -= deltaTime;
		//Check when we need to play it again
		if (timeLeftToPlayWonSound <= 0)
		{
			spWonSound->play();
			timeLeftToPlayWonSound += 1.f;
			playWonSound--;
		}
	}
}

void MusicSystem::OnGameRestart()
{
	//Play music from begining
	mainMusic->setPlayingOffset(sf::seconds(0.f));
}

void MusicSystem::OnPlayExplosionSound()
{
	spExplosionSound->play();
}

void MusicSystem::OnPlayManyExplosionsSounds(int numOfTimesToPlay)
{
	playExplosionSound = numOfTimesToPlay;
}

void MusicSystem::OnPlayWonSound()
{
	playWonSound = 3;
}

void MusicSystem::OnPlayHurtSound(int n)
{
	spHurtSound->play();
}

void MusicSystem::OnPlayWarningSound()
{
	spWarningSound->play();
}

void MusicSystem::OnPlayPowerUpSound()
{
	spPowerUpSound->play();
}

void MusicSystem::OnPlayShotSound() 
{
	spShotSound->play();
}


//PARTICLES MANAGER
void ParticlesManager::Update(SceneNode& scene, float deltaTime)
{
	//If game paused, then do nothing
	if (ECSGame::Instance().GetGameState() != GameState::Pause)
	{
		SceneNodeVisitorParticles visitor;
		scene.AcceptVisitor(visitor);
	}
}


//GAME SYSTEM
void GameSystem::Initialize()
{
	//Subscribe to some signals
	signals::onGameRestart.connect(&GameSystem::OnGameRestart, this);
	signals::onGameWon.connect(&GameSystem::OnGameWon, this);
	signals::onGameLost.connect(&GameSystem::OnGameLost, this);
	signals::onEnemyDestroyed.connect(&GameSystem::OnEnemyDestroyed, this);
	signals::onBulletAvoided.connect(&GameSystem::OnBulletAvoided, this);
	signals::onBulletFired.connect(&GameSystem::OnBulletFired, this);
}

//Update player`s invulnerability and shield
void GameSystem::Update(SceneNode& scene, float deltaTime)
{
	//Get player
	std::weak_ptr<Entity> wSpEntity = ECSGame::Instance().GetEntityManager().FindEntity("Player");
	std::shared_ptr<Entity> sSpEntity = wSpEntity.lock();
	//Check if player exist
	if (sSpEntity != nullptr)
	{
		std::shared_ptr<PlayerComponent> spPlayerCom = GetPlayerComponent(*sSpEntity);
		//Check if player component exist
		if (spPlayerCom != nullptr)
		{
			//Process invulnerability
			spPlayerCom->invulnerabilityTimeLeft -= deltaTime;

			if (spPlayerCom->invulnerabilityTimeLeft < 0.f)
			{
				//If time passed then destroy shield
				signals::onDeleteEntity(ECSGame::Instance().GetEntityManager().FindEntity("Shield"));
				spPlayerCom->invulnerable = false;
			}
			else if (spPlayerCom->invulnerabilityTimeLeft < 5.f)
			{
				//If time left is 5 seconds or less than start to blink text
				//and every second play sound, to indicate that shield will end soon
				std::weak_ptr<Entity> wpShield = ECSGame::Instance().GetEntityManager().FindEntity("Shield");
				std::shared_ptr<Entity> spShield = wpShield.lock();
				std::shared_ptr<ShieldComponent> spShieldCom = GetShieldComponent(*spShield);
				std::shared_ptr<RectangleShapeComponent> spShape = GetRectangleShapeComponent(*spShield);

				spShieldCom->currentTime += deltaTime;
				spShieldCom->currentSoundTime -= deltaTime;

				if (spShieldCom->currentTime > spShieldCom->blinkingPeriod)
				{
					spShieldCom->currentTime -= spShieldCom->blinkingPeriod;
					spShieldCom->decreasingVisibility = !spShieldCom->decreasingVisibility;
				}

				//Blink shield
				sf::Color color = spShape->shape.getFillColor();
				if (spShieldCom->decreasingVisibility)
					color.a = gel::linearInterpolation(255, 0, spShieldCom->currentTime / spShieldCom->blinkingPeriod);
				else
					color.a = gel::linearInterpolation(0, 255, spShieldCom->currentTime / spShieldCom->blinkingPeriod);

				spShape->shape.setFillColor(color);

				//Play sound
				if (spShieldCom->currentSoundTime < 0.f)
				{
					spShieldCom->currentSoundTime += spShieldCom->soundPeriod;
					signals::onPlayWarningSound();
				}
			}
		}
	}
}


//Change enemies behaviour according to the game state
void ChangeEnemiesBehaviour()
{
	//Get enemies node
	std::weak_ptr<Entity> wpEntity = ECSGame::Instance().GetEntityManager().FindEntity("Enemies");
	std::shared_ptr<Entity> spEntity = wpEntity.lock();
	SceneNode* nodePtrEntity = ECSGame::Instance().GetSceneRoot().FindChild(*spEntity);
	//After change all enemies behaviour
	SceneNodeVisitorChangeEnemyBehaviour visitor;
	nodePtrEntity->AcceptVisitor(visitor);
	//Check if exploded at least one enemy the play explosion sounds many times
	if(visitor.explodedEnemy)
		signals::onPlayManyExplosionsSounds(static_cast<int>(gel::Randf(3.f, 6.f)));
}

//Remove all entities and start camera animation
void GameSystem::OnGameRestart()
{
	//Delete all entities
	signals::onDeleteEntitiesByName("Bullet");
	signals::onDeleteEntitiesByName("PowerUp1");
	signals::onDeleteEntitiesByName("PowerUp2");
	//Explode all enemies ship and then delete enemies
	ChangeEnemiesBehaviour();
	signals::onDeleteEntitiesByName("Enemy");
	//Destroy shield
	signals::onDeleteEntity(ECSGame::Instance().GetEntityManager().FindEntity("Shield"));
	//Get player
	std::weak_ptr<Entity> wpPlayer = ECSGame::Instance().GetEntityManager().FindEntity("Player");
	//Check that player exist then destroy it
	if (wpPlayer.lock() != nullptr)
	{
		CreateExplosion(wpPlayer.lock()->GetTransformable().getPosition());
		signals::onDeleteEntity(wpPlayer);
	}

	//Delete game UI
	RemoveGameUI();

	//Set camera to move again
	std::shared_ptr<Entity> spCamera = ECSGame::Instance().GetEntityManager().FindEntity("Camera").lock();
	std::shared_ptr<CameraComponent> spCameraCom = GetCameraComponent(*spCamera);
	spCameraCom->moveCamera = true;
	std::shared_ptr<MovementComponent> spMovementCom = GetMovementComponent(*spCamera);
	spMovementCom->velocity = {-400.f,0.f};

	//Reset statistics
	bulletsAvoided = 0;
	enemiesDestroyed = 0;
	bulletsFired = 0;
}

//It manages won animations
void GameSystem::OnGameWon() 
{
	signals::onPlayWonSound();
	//Delete game UI
	RemoveGameUI();
	//Create end game text
	InitializeGameEndText(enemiesDestroyed,bulletsAvoided,bulletsFired);
	ChangeEnemiesBehaviour();
	signals::onDeleteEntitiesByName("Enemy");
	RemovePlayerComponent();
}

//It manages lost animations
void GameSystem::OnGameLost() 
{
	RemoveGameUI();
	InitializeGameEndText(enemiesDestroyed, bulletsAvoided, bulletsFired);
	//Set camera movement to false
	std::shared_ptr<CameraComponent> spCameraCom = GetCameraFromCameraEntity();
	spCameraCom->moveCamera = false;
	ChangeEnemiesBehaviour();
}

//Increase statisctics variables functions
void GameSystem::OnBulletFired() { bulletsFired++; }
void GameSystem::OnBulletAvoided() { bulletsAvoided++; }
void GameSystem::OnEnemyDestroyed() { enemiesDestroyed++; }
