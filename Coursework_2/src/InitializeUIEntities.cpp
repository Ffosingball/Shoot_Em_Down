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

//Setup text properties, where it should move. Also either xPos or yPos
//should be not null!
void SetupMoveTextProperties(const std::string textName, const float* xPos, const float* yPos, const sf::Vector2f velocity, const bool destroyAtTarget) 
{
	//Get text by name
	std::weak_ptr<Entity> wGText = ECSGame::Instance().GetEntityManager().FindEntity(textName);
	std::shared_ptr<Entity> sGText = wGText.lock();
	//Get components
	std::shared_ptr<UIPartComponent> sGTextCom = GetUIPartComponent(*sGText);
	std::shared_ptr<MovementComponent> sGMovCom = GetMovementComponent(*sGText);

	//Set animation properties for gameText
	sGTextCom->moveIt = true;
	sGTextCom->destroyAtTarget = destroyAtTarget;
	//Set target position
	if(xPos==nullptr)
		sGTextCom->targetPosition = {sGText->GetTransformable().getPosition().x, *yPos};
	else if(yPos == nullptr)
		sGTextCom->targetPosition = { *xPos, sGText->GetTransformable().getPosition().y };
	else
		sGTextCom->targetPosition = { *xPos, *yPos };
	//Set velocity
	sGMovCom->velocity = velocity;
}

//This function creates a text
std::shared_ptr<Entity> CreateGenericText(const std::string textName, const int fontSize) 
{
	//create entity
	std::shared_ptr<Entity> spUI = CreateNewEntityAt("UI", textName).lock();
	//Add component
	spUI->AddComponent(ComponentType::Text);
	spUI->AddComponent(ComponentType::Movement);
	spUI->AddComponent(ComponentType::UIPart);
	//Get component
	std::shared_ptr<TextComponent> spUICom = GetTextComponent(*spUI);
	//Get font from the resource manager
	std::shared_ptr<sf::Font> fontPtr = ResourceManager::Instance().LoadFont("media/fonts/PixelOperator8-Bold.ttf").lock();
	//Set text properties
	spUICom->text = std::make_shared<sf::Text>(*fontPtr);
	spUICom->text->setFillColor(sf::Color::White);
	spUICom->text->setOutlineColor(sf::Color(100,100,100));
	spUICom->text->setOutlineThickness(1.2f);
	spUICom->text->setCharacterSize(fontSize);

	return spUI;
}

//Creates text without moving animation
void InitializeText(const std::string name, const std::string text, const int fontSize, const sf::Vector2f position, const bool isBlinking=false, const bool isMoving = false, const float* targetX=nullptr, const float* targetY=nullptr, const sf::Vector2f velocity = {0.f,0.f}, const bool skipOriginReset=false)
{
	//Check if text exist then use existing one, otherwise create new one
	std::shared_ptr<Entity> spUI = ECSGame::Instance().GetEntityManager().FindEntity(name).lock();
	//Check if this text exist
	if (spUI == nullptr)
		spUI = CreateGenericText(name,fontSize);
	//Get component
	std::shared_ptr<UIPartComponent> spUICom = GetUIPartComponent(*spUI);
	std::shared_ptr<TextComponent> spTextCom = GetTextComponent(*spUI);
	std::shared_ptr<MovementComponent> spMovCom = GetMovementComponent(*spUI);
	//set text
	spTextCom->text->setString(text);
	//Set text position
	SetNewPosition(spUI, position);
	//Check if text should blink
	if (isBlinking)
	{
		//Set blinking properties
		spUICom->isBlinking = true;
		spUICom->decreasingVisibility = false;
		spUICom->blinkTime = spUICom->blinkingPeriod;
		spUICom->flatLine = true;
	}
	//Check if text should move
	if (isMoving)
	{
		//Set moving animation properties
		SetupMoveTextProperties(name, targetX, targetY, velocity, false);
	}
	//Reset text origin to center of the text if needed
	if(!skipOriginReset)
		gel::SetTextOrigin(*(spTextCom->text), position);
}

//Initialize start menu text
void InitializeStartUI()
{
	float yOffset = 40.f;
	float xOffset = 100.f;
	int gameTextFont = 60;
	int startTextFont = 36;
	float textSpeed = 750.f;
	//Get camera
	std::shared_ptr<CameraComponent> sCameraCom = GetCameraFromUICameraEntity();
	//Initialize all ui entities
	float xPos = sCameraCom->view.getCenter().x;
	sf::Vector2f velocity{textSpeed,0.f};
	sf::Vector2f initPos = { -xOffset, sCameraCom->view.getCenter().y - yOffset };
	InitializeText("GameText","Shoot `Em Down", gameTextFont, initPos,true,true,&xPos,nullptr,velocity);
	velocity={ -textSpeed,0.f };
	initPos = { sCameraCom->view.getSize().x+xOffset, sCameraCom->view.getCenter().y + yOffset };
	InitializeText("StartText","Press <Q> to start the game", startTextFont, initPos,true, true, &xPos, nullptr, velocity);
	initPos = { sCameraCom->view.getSize().x+xOffset, sCameraCom->view.getCenter().y + (yOffset * 2) };
	InitializeText("ExitText", "Press <Escape> to exit the game", startTextFont, initPos,true, true, &xPos, nullptr, velocity);
}

//Creates move animation
void MoveText(const std::string name, const float* targetX, const float* targetY, const sf::Vector2f velocity, const bool destroyAtTarget)
{
	//Get text pointer
	std::shared_ptr<Entity> sText = ECSGame::Instance().GetEntityManager().FindEntity(name).lock();
	//Check if this text exist
	if (sText != nullptr)
	{
		//Setup movement properties
		SetupMoveTextProperties(name, targetX, targetY, velocity, destroyAtTarget);
	}
}

//Move start menu text
void MoveStartText() 
{
	float textSpeed = 200.f;
	float yOffset = 40.f;
	//Get camera
	std::shared_ptr<CameraComponent> sCameraCom = GetCameraFromUICameraEntity();
	//Get yPos for GameText
	float yPos = sCameraCom->view.getCenter().y - (sCameraCom->view.getSize().y / 2.f) - yOffset;
	SetupMoveTextProperties("GameText", nullptr, &yPos, { 0.f,-textSpeed },true);
	SetupMoveTextProperties("StartText", nullptr, &yPos, { 0.f,-textSpeed },true);
	SetupMoveTextProperties("ExitText", nullptr, &yPos, { 0.f,-textSpeed }, true);

	yPos = sCameraCom->view.getSize().y + yOffset;
	sf::Vector2f velocity = { 0.f,textSpeed };
	MoveText("MediumText", nullptr, &yPos, velocity, true);
	MoveText("MediumHintText", nullptr, &yPos, velocity, true);
	MoveText("EasyText", nullptr, &yPos, velocity, true);
	MoveText("EasyHintText", nullptr, &yPos, velocity, true);
	MoveText("HardText", nullptr, &yPos, velocity, true);
	MoveText("HardHintText", nullptr, &yPos, velocity, true);
}


void InitializePauseText()
{
	//std::cout << "Create pauseText!\n";
	int pauseTextFont = 50;
	//get camera position
	std::shared_ptr<CameraComponent> sCameraCom = GetCameraFromUICameraEntity();
	sf::Vector2f position = sCameraCom->view.getCenter();
	//Create text
	InitializeText("PauseText", "Press <Q> to continue", pauseTextFont, position, true);
}


void MovePauseText()
{
	float textSpeed = 400.f;
	float yOffset = 40.f;
	//Get camera
	std::shared_ptr<CameraComponent> sCameraCom = GetCameraFromUICameraEntity();
	float yPos = sCameraCom->view.getCenter().y + (sCameraCom->view.getSize().y / 2.f) + yOffset;
	SetupMoveTextProperties("PauseText", nullptr, &yPos, { 0.f,textSpeed },true);
}

//Create game end UI
void InitializeGameEndText(int shipsDestroyed, int bulletsAvoided, int bulletsFired)
{
	float textSpeed = 400.f;
	float textStatSpeed = 1000.f;
	float yOffset = 40.f;
	float xOffset = 240.f;
	float distanceBetweenLines = 50.f;
	int endTextFont = 50;
	int statTextFont = 30;
	//get camera position
	std::shared_ptr<CameraComponent> sCameraCom = GetCameraFromUICameraEntity();
	sf::Vector2f position = sCameraCom->view.getCenter();

	//Create LOSE/WON text
	//Check whether player won or lost to set correct text
	std::string text{" "};
	if(ECSGame::Instance().GetGameState()==GameState::Won)
		text = "You won! Congratulations!";
	else if (ECSGame::Instance().GetGameState() == GameState::Lost)
		text = "Game Over!";

	sf::Vector2f initPos = { sCameraCom->view.getCenter().x,-yOffset };
	position.y += -2.f * distanceBetweenLines;
	sf::Vector2f velocity = { 0.f,textSpeed };
	InitializeText("GameEndText", text, endTextFont, initPos, false, true, &position.x, &position.y, velocity);

	//Create hint 1 text
	initPos = { sCameraCom->view.getCenter().x,sCameraCom->view.getCenter().y + (sCameraCom->view.getSize().y / 2.f) + yOffset };
	position = sCameraCom->view.getCenter();
	position.y += 2.f * distanceBetweenLines;
	velocity = { 0.f,-textSpeed };
	InitializeText("ResetText", "Press <R> to start again!", statTextFont, initPos, true, true, &position.x, &position.y, velocity);

	//Create hint 2 text
	initPos = { sCameraCom->view.getCenter().x,sCameraCom->view.getCenter().y + (sCameraCom->view.getSize().y / 2.f) + (yOffset * 2) };
	position = sCameraCom->view.getCenter();
	position.y += 3.f * distanceBetweenLines;
	velocity = { 0.f,-textSpeed };
	InitializeText("ExitGameText", "Press <Escape> to exit game!", statTextFont, initPos, true, true, &position.x, &position.y, velocity);

	//statistics velocity
	velocity = { -textStatSpeed,0.f };

	//Create STATISTICS text 1
	initPos = { sCameraCom->view.getCenter().x + (sCameraCom->view.getSize().x / 2.f) + xOffset,sCameraCom->view.getCenter().y - distanceBetweenLines };
	position = sCameraCom->view.getCenter();
	position.y += -1.f * distanceBetweenLines;
	InitializeText("StatText1", "Ships destroyed: " + std::to_string(shipsDestroyed), statTextFont, initPos, false, true, &position.x, &position.y, velocity);

	//Create STATISTICS text 2
	initPos = { sCameraCom->view.getCenter().x + (sCameraCom->view.getSize().x / 2.f) + xOffset,sCameraCom->view.getCenter().y };
	position = sCameraCom->view.getCenter();
	InitializeText("StatText2", "Bullets evaded: " + std::to_string(bulletsAvoided), statTextFont, initPos, false, true, &position.x, &position.y, velocity);

	//Create STATISTICS text 3
	if (bulletsFired != 0)
		text = "Accuracy: " + std::to_string(static_cast<int>(round((static_cast<float>(shipsDestroyed) / static_cast<float>(bulletsFired)) * 100))) + "%";
	else
		text = "Accuracy: None";

	initPos = { sCameraCom->view.getCenter().x + (sCameraCom->view.getSize().x / 2.f) + xOffset,sCameraCom->view.getCenter().y + distanceBetweenLines };
	position = sCameraCom->view.getCenter();
	position.y += 1.f * distanceBetweenLines;
	InitializeText("StatText3", text, statTextFont, initPos, false, true, &position.x, &position.y, velocity);
}


//Move game end UI
void MoveGameEndText()
{
	float textSpeed = 400.f;
	float textStatSpeed = 1200.f;
	float yOffset = 40.f;
	float xOffset = 180.f;
	//Get camera
	std::shared_ptr<CameraComponent> sCameraCom = GetCameraFromUICameraEntity();

	//Game end text animation properties
	float yPos = sCameraCom->view.getCenter().y - (sCameraCom->view.getSize().y / 2.f) - yOffset;
	sf::Vector2f velocity = { 0.f,-textSpeed };
	MoveText("GameEndText",nullptr,&yPos,velocity,true);

	//Reset and exitGame text animation properties
	yPos = sCameraCom->view.getCenter().y + (sCameraCom->view.getSize().y / 2.f) + yOffset;
	velocity = { 0.f,textSpeed };
	MoveText("ResetText", nullptr, &yPos, velocity, true);
	MoveText("ExitGameText", nullptr, &yPos, velocity, true);

	//Stat1, Stat2 and Stat3 text animation properties
	float xPos = sCameraCom->view.getCenter().x + (sCameraCom->view.getSize().x * 0.5f) + xOffset;
	velocity = { textStatSpeed,0.f };
	MoveText("StatText1", &xPos, nullptr, velocity, true);
	MoveText("StatText2", &xPos, nullptr, velocity, true);
	MoveText("StatText3", &xPos, nullptr, velocity, true);
}

//Create game UI
void InitializeGameUI()
{
	//HP CIRCLES SIZE
	sf::Vector2f size{ 60.f,60.f };
	float textSpeed = 300.f;
	float xOffset = 700.f;
	float yOffset = 100.f;
	float distanceBetweenLines = 30.f;
	float textDistanceToPass = 40.f;
	int hintTextFont = 25;

	//Get player`s component
	std::weak_ptr<Entity> wPlayer = ECSGame::Instance().GetEntityManager().FindEntity("Player");
	std::shared_ptr<Entity> sPlayer = wPlayer.lock();
	std::shared_ptr<PlayerComponent> sPlayerCom = GetPlayerComponent(*sPlayer);
	//Get camera component
	std::shared_ptr<CameraComponent> sCameraCom = GetCameraFromUICameraEntity();

	//Create the same amount of hp circles, as player has
	for (int i = 0; i < sPlayerCom->hp; i++)
	{
		//Create hp entity
		std::shared_ptr<Entity> spHP = CreateNewEntityAt("UI", "HP"+std::to_string(i)).lock();
		// Add components
		spHP->AddComponent(ComponentType::Movement);
		spHP->AddComponent(ComponentType::UIPart);
		spHP->AddComponent(ComponentType::RectangleShape);
		//Get components
		std::shared_ptr<MovementComponent> spHPMovement = GetMovementComponent(*spHP);
		std::shared_ptr<RectangleShapeComponent> spHPShape = GetRectangleShapeComponent(*spHP);
		std::shared_ptr<UIPartComponent> spUIPart = GetUIPartComponent(*spHP);
		//set ptoperties
		SetupRectangleShape(spHPShape, size, "media/textures/hp_icons.png");
		spHPShape->shape.setTextureRect(sf::IntRect({ 0,0 }, {20,20}));
		SetNewPosition(spHP, { sCameraCom->view.getCenter().x - (sCameraCom->view.getSize().x / 2.f) + size.x * (i + 1) * 1.25f,-yOffset });
		//Set animation properties
		spUIPart->moveIt = true;
		spUIPart->destroyAtTarget = false;
		spUIPart->targetPosition = { sCameraCom->view.getCenter().x - (sCameraCom->view.getSize().x / 2.f) + size.x * (i + 1) * 1.25f,size.y };
		spHPMovement->velocity = {0.f,textSpeed};
	}

	//Hints velocity
	sf::Vector2f velocity = { textSpeed,0.f };
	float posX = sCameraCom->view.getCenter().x - (sCameraCom->view.getSize().x / 2.f) + textDistanceToPass;
	
	//Create hint 1
	sf::Vector2f initPos = { sCameraCom->view.getCenter().x - (sCameraCom->view.getSize().x / 2.f) - xOffset, size.y + (distanceBetweenLines * 2) };
	InitializeText("HintText1", "<ARROWS> or <WASD> move your airplane", hintTextFont, initPos, false, true, &posX, &initPos.y, velocity,true);

	//Create hints 2
	initPos = { sCameraCom->view.getCenter().x - (sCameraCom->view.getSize().x / 2.f) - xOffset, size.y + (distanceBetweenLines * 3) };
	InitializeText("HintText2", "<Q> pause game", hintTextFont, initPos, false, true, &posX, &initPos.y, velocity, true);

	//Create hints 3
	initPos = { sCameraCom->view.getCenter().x - (sCameraCom->view.getSize().x / 2.f) - xOffset, size.y + (distanceBetweenLines * 4) };
	InitializeText("HintText3", "<R> restart game", hintTextFont, initPos, false, true, &posX, &initPos.y, velocity, true);

	//Create hints 4
	initPos = { sCameraCom->view.getCenter().x - (sCameraCom->view.getSize().x / 2.f) - xOffset, size.y + (distanceBetweenLines * 5) };
	InitializeText("HintText4", "<Space> fire bullets", hintTextFont, initPos, false, true, &posX, &initPos.y, velocity, true);
}

//Start animation to remove game UI
void RemoveGameUI()
{
	float textSpeed = 140.f;
	float yOffset = 80.f;

	int count = 0;
	//For all hp entities while they exist
	while (ECSGame::Instance().GetEntityManager().FindEntity("HP" + std::to_string(count)).lock() != nullptr)
	{
		//Get entity
		std::shared_ptr<Entity> spHP = ECSGame::Instance().GetEntityManager().FindEntity("HP" + std::to_string(count)).lock();
		//Get components
		std::shared_ptr<MovementComponent> spHPMovement = GetMovementComponent(*spHP);
		std::shared_ptr<UIPartComponent> spUIPart = GetUIPartComponent(*spHP);
		//Set new properties
		spHPMovement->velocity = {0.f,-textSpeed};
		spUIPart->moveIt = true;
		spUIPart->destroyAtTarget = true;
		spUIPart->targetPosition = {spHP->GetTransformable().getPosition().x,-yOffset};

		count++;
	}

	count = 0;
	//For all durability entities while they exist
	while (ECSGame::Instance().GetEntityManager().FindEntity("Durability" + std::to_string(count)).lock() != nullptr)
	{
		//Get entity
		std::shared_ptr<Entity> spHP = ECSGame::Instance().GetEntityManager().FindEntity("Durability" + std::to_string(count)).lock();
		//Get components
		std::shared_ptr<MovementComponent> spHPMovement = GetMovementComponent(*spHP);
		std::shared_ptr<UIPartComponent> spUIPart = GetUIPartComponent(*spHP);
		//Set new properties
		spHPMovement->velocity = { 0.f,-textSpeed };
		spUIPart->moveIt = true;
		spUIPart->destroyAtTarget = true;
		spUIPart->targetPosition = { spHP->GetTransformable().getPosition().x,-yOffset };

		count++;
	}

	//All hints text animation properties
	float yPos = -yOffset;
	sf::Vector2f velocity = { 0.f,-textSpeed };
	MoveText("HintText1", nullptr, &yPos, velocity, true);
	MoveText("HintText2", nullptr, &yPos, velocity, true);
	MoveText("HintText3", nullptr, &yPos, velocity, true);
	MoveText("HintText4", nullptr, &yPos, velocity, true);
}

//Create durability bars
void InitializeDurabilityBars(int durability)
{
	//Durability bar size
	sf::Vector2f size{ 24.f,50.f };
	float xOffset = 20.f;
	float yOffset = 10.f;
	float durSpeed = 300.f;

	//Get last hp circle position
	std::shared_ptr<Entity> sHPBar;
	int counter{ 0 };
	while (true)
	{
		if (ECSGame::Instance().GetEntityManager().FindEntity("HP" + std::to_string(counter)).lock() != nullptr)
			sHPBar = ECSGame::Instance().GetEntityManager().FindEntity("HP" + std::to_string(counter)).lock();
		else
			break;

		counter++;
	}
	sf::Vector2f hpPos = sHPBar->GetTransformable().getPosition();
	//Get camera component
	std::shared_ptr<CameraComponent> sCameraCom = GetCameraFromUICameraEntity();

	//Create the same amount of durability lines, as player has
	for (int i = 0; i < durability; i++)
	{
		//Check if durability lines exist, if not then create it
		if (ECSGame::Instance().GetEntityManager().FindEntity("Durability" + std::to_string(i)).lock() == nullptr)
		{
			//Create durability entities
			std::shared_ptr<Entity> spDur = CreateNewEntityAt("UI", "Durability" + std::to_string(i)).lock();
			// Add components
			spDur->AddComponent(ComponentType::Movement);
			spDur->AddComponent(ComponentType::RectangleShape);
			spDur->AddComponent(ComponentType::UIPart);
			//Get components
			std::shared_ptr<MovementComponent> spDurMovement = GetMovementComponent(*spDur);
			std::shared_ptr<RectangleShapeComponent> spDurShape = GetRectangleShapeComponent(*spDur);
			std::shared_ptr<UIPartComponent> spUIPart = GetUIPartComponent(*spDur);
			//set ptoperties
			SetupRectangleShape(spDurShape, size, "media/textures/kenney_shmup_durability.png");
			SetNewPosition(spDur, { (size.x * (i + 1.5f) * 1.25f)+hpPos.x+xOffset, -(yOffset*2.f) });
			spUIPart->moveIt = true;
			spUIPart->destroyAtTarget = false;
			spUIPart->targetPosition = { (size.x * (i + 1.5f) * 1.25f) + hpPos.x + xOffset, size.y + yOffset };
			spDurMovement->velocity = { 0.f,durSpeed };
		}
	}
}

//Highligh current difficulty choosed
void HighlightText(const std::string name, sf::Color color) 
{
	std::shared_ptr<Entity> spText = ECSGame::Instance().GetEntityManager().FindEntity(name).lock();
	std::shared_ptr<UIPartComponent> spUI = GetUIPartComponent(*spText);
	std::shared_ptr<TextComponent> spTextCom = GetTextComponent(*spText);
	spUI->isBlinking = true;
	spTextCom->text->setOutlineColor(color);
	spTextCom->text->setOutlineThickness(spTextCom->text->getOutlineThickness() * 2.f);
}

//Remove highlighting of the text
void RemoveTextHighlighting(const std::string name)
{
	std::shared_ptr<Entity> spText = ECSGame::Instance().GetEntityManager().FindEntity(name).lock();
	std::shared_ptr<UIPartComponent> spUI = GetUIPartComponent(*spText);
	std::shared_ptr<TextComponent> spTextCom = GetTextComponent(*spText);
	spUI->isBlinking = false;
	spTextCom->text->setOutlineColor(sf::Color(100, 100, 100));
	spTextCom->text->setOutlineThickness(spTextCom->text->getOutlineThickness() / 2.f);
	//Get colors
	sf::Color color = spTextCom->text->getFillColor();
	sf::Color outlineColor = spTextCom->text->getOutlineColor();
	//Set new alphaValue
	color.a = 255;
	outlineColor.a = 255;
	spTextCom->text->setFillColor(color);
	spTextCom->text->setOutlineColor(outlineColor);
}

//Creates difficulty text
void CreateDifficultyLevelText(const DifficultyLevel level) 
{
	//Text movement properties
	float textSpeed = 120.f;
	float xOffset = 200.f;
	float yOffset = 50.f;
	float textDistanceToPass = 40.f;
	int hintTextFont = 40;

	//Get camera component
	std::shared_ptr<CameraComponent> sCameraCom = GetCameraFromUICameraEntity();

	//Movement values
	sf::Vector2f velocity = { 0.f,-textSpeed };
	float posY1 = sCameraCom->view.getCenter().y + (sCameraCom->view.getSize().y / 2.f) - yOffset;
	float posY2 = sCameraCom->view.getCenter().y + (sCameraCom->view.getSize().y / 2.f) - (yOffset*2.f);

	//Create medium text
	sf::Vector2f initPos = { sCameraCom->view.getCenter().x, sCameraCom->view.getSize().y+textDistanceToPass };
	InitializeText("MediumText", "Medium", hintTextFont, initPos, false, true, &initPos.x, &posY2, velocity);
	InitializeText("MediumHintText", "<2>", hintTextFont, initPos, false, true, &initPos.x, &posY1, velocity);

	//Create easy text
	initPos = { sCameraCom->view.getCenter().x-xOffset, sCameraCom->view.getSize().y + textDistanceToPass };
	InitializeText("EasyText", "Easy", hintTextFont, initPos, false, true, &initPos.x, &posY2, velocity);
	InitializeText("EasyHintText", "<1>", hintTextFont, initPos, false, true, &initPos.x, &posY1, velocity);

	//Create hard text
	initPos = { sCameraCom->view.getCenter().x + xOffset, sCameraCom->view.getSize().y + textDistanceToPass };
	InitializeText("HardText", "Hard", hintTextFont, initPos, false, true, &initPos.x, &posY2, velocity);
	InitializeText("HardHintText", "<3>", hintTextFont, initPos, false, true, &initPos.x, &posY1, velocity);

	//Change outline according to the game level
	switch (level) 
	{
	case DifficultyLevel::Easy:
		HighlightText("EasyText",sf::Color::Green);
		HighlightText("EasyHintText", sf::Color::Green);
		break;
	case DifficultyLevel::Medium:
		HighlightText("MediumText", sf::Color::Yellow);
		HighlightText("MediumHintText", sf::Color::Yellow);
		break;
	case DifficultyLevel::Hard:
		HighlightText("HardText", sf::Color::Red);
		HighlightText("HardHintText", sf::Color::Red);
		break;
	}
}

//Creates difficulty text
void ChangeDifficultyLevelText(const DifficultyLevel currentLevel, const DifficultyLevel previousLevel) 
{
	//If current and previous levels are the same then do nothing
	if (currentLevel != previousLevel) 
	{
		//Change outline according to the game level
		switch (currentLevel)
		{
		case DifficultyLevel::Easy:
			HighlightText("EasyText", sf::Color::Green);
			HighlightText("EasyHintText", sf::Color::Green);
			break;
		case DifficultyLevel::Medium:
			HighlightText("MediumText", sf::Color::Yellow);
			HighlightText("MediumHintText", sf::Color::Yellow);
			break;
		case DifficultyLevel::Hard:
			HighlightText("HardText", sf::Color::Red);
			HighlightText("HardHintText", sf::Color::Red);
			break;
		}

		//Change remove highlighting according to previous level
		switch (previousLevel)
		{
		case DifficultyLevel::Easy:
			RemoveTextHighlighting("EasyText");
			RemoveTextHighlighting("EasyHintText");
			break;
		case DifficultyLevel::Medium:
			RemoveTextHighlighting("MediumText");
			RemoveTextHighlighting("MediumHintText");
			break;
		case DifficultyLevel::Hard:
			RemoveTextHighlighting("HardText");
			RemoveTextHighlighting("HardHintText");
			break;
		}
	}
}