#pragma once

#include "System.h"
#include "Systems.h"
#include <SFML/Graphics.hpp>
#include "SceneNode.h"
#include <sigslot/signal.hpp>
#include "GameState.h"
#include <SFML/Audio.hpp>


//Input system processes inputs
class InputSystem :public System 
{
public:
	//To ensure that keys will call functions only once when they are pressed
	bool keySpacePressed{ false };
	bool keyQPressed{ false };
	bool keyRPressed{ false };

	virtual ~InputSystem() = default;
private:
	void Initialize() override;
	void Update(SceneNode& scene, float deltaTime) override;
	//Signals functions
	void OnKeyPressed(sf::Event::KeyPressed key);
	void OnKeyReleased(sf::Event::KeyReleased key);
};

//Processes all movement
class MovementSystem :public System
{
public:
	//Player direction
	sf::Vector2f direction;

	virtual ~MovementSystem() = default;
private:
	void Initialize() override;
	void Update(SceneNode& scene, float deltaTime) override;
	//Set direction to the new one
	void OnMovePlayer(sf::Vector2f direction) { this->direction = direction; }
};

//Processes all collisions
class CollisionSystem :public System
{
public:
	//Power up drop rates
	float powerUpDropRate = 0.f;
	float powerUp1Correlation = 0.f;

	virtual ~CollisionSystem() = default;
private:
	void Initialize() override;
	void Update(SceneNode& scene, float deltaTime) override;
	//Signals functions
	void OnDifficultyLevelChange(DifficultyLevel level);
};

//Processes UI
class UISystem :public System
{
public:
	DifficultyLevel level{DifficultyLevel::Medium};
	virtual ~UISystem() = default;
private:
	void Initialize() override;
	void Update(SceneNode& scene, float deltaTime) override;
	//Signals functions
	void OnGameRestart();
	void OnPlayerHurt(int hpLeft);
	void OnDurabilityUsed(int durabilityLeft);
	void OnDifficultyLevelChange(DifficultyLevel level);
	void OnDifficultyLevelShow();
};

//Manages all enemies
class EnemiesSystem :public System
{
public:
	//Enemies spawn and fire rates
	float spawnEnemyPeriod{1.f};
	float timeLeftToSpawnNextEnemy{ 1.f };
	float firePeriod{ 2.f };

	virtual ~EnemiesSystem() = default;
private:
	void Initialize() override;
	void Update(SceneNode& scene, float deltaTime) override;
	//Signals functions
	void OnDifficultyLevelChange(DifficultyLevel level);
};


//Manages Music
class MusicSystem :public System
{
public:
	//Pointer to sound and music
	std::shared_ptr<sf::Music> mainMusic;
	std::shared_ptr<sf::Sound> spExplosionSound;
	std::shared_ptr<sf::Sound> spHurtSound;
	std::shared_ptr<sf::Sound> spShotSound;
	std::shared_ptr<sf::Sound> spWarningSound;
	std::shared_ptr<sf::Sound> spPowerUpSound;
	std::shared_ptr<sf::Sound> spWonSound;
	//Variables to play some sounds several times
	int playExplosionSound{ 0 };
	float timeLeftToPlaySound{ 0.f };
	int playWonSound{ 0 };
	float timeLeftToPlayWonSound{0.f};

	virtual ~MusicSystem() = default;
private:
	void Initialize() override;
	void Update(SceneNode& scene, float deltaTime) override;
	//Signals functions
	void OnGameRestart();
	void OnPlayExplosionSound();
	void OnPlayHurtSound(int n);
	void OnPlayShotSound();
	void OnPlayWarningSound();
	void OnPlayPowerUpSound();
	void OnPlayManyExplosionsSounds(int numOfTimesToPlay);
	void OnPlayWonSound();
};


//Manages all entities with particles component
class ParticlesManager :public System
{
public:
	std::shared_ptr<sf::Music> mainMusic;

	virtual ~ParticlesManager() = default;
private:
	void Update(SceneNode& scene, float deltaTime) override;
};


//Manages clean up during restart, won-lost animations, game statistics
//and player shield
class GameSystem :public System
{
public:
	//Statistics variables
	int bulletsAvoided{0};
	int enemiesDestroyed{ 0 };
	int bulletsFired{ 0 };

	virtual ~GameSystem() = default;
private:
	void Initialize() override;
	void Update(SceneNode& scene, float deltaTime) override;
	//Signals functions
	void OnGameRestart();
	void OnGameWon();
	void OnGameLost();
	void OnBulletFired();
	void OnBulletAvoided();
	void OnEnemyDestroyed();
};


//List of all possible signals
namespace signals
{
	inline sigslot::signal<sf::Event::KeyPressed> onKeyPressed;
	inline sigslot::signal<sf::Event::KeyReleased> onKeyReleased;
	inline sigslot::signal<> onGameRestart;
	inline sigslot::signal<sf::Vector2f> onMovePlayer;
	inline sigslot::signal<> onGameWon;
	inline sigslot::signal<> onGameLost;
	inline sigslot::signal<> onPlayExplosionSound;
	inline sigslot::signal<int> onPlayerHurt;
	inline sigslot::signal<> onPlayShotSound;
	inline sigslot::signal<> onPlayPowerUpSound;
	inline sigslot::signal<> onPlayWarningSound;
	inline sigslot::signal<> onBulletAvoided;
	inline sigslot::signal<> onEnemyDestroyed;
	inline sigslot::signal<> onBulletFired;
	inline sigslot::signal<int> onPlayManyExplosionsSounds;
	inline sigslot::signal<int> onDurabilityUsed;
	inline sigslot::signal<> onPlayWonSound;
	inline sigslot::signal<DifficultyLevel> onDifficultyLevelChange;
	inline sigslot::signal<> onDifficultyLevelShow;
}