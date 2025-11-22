#pragma once
#include "SceneNodeVisitor.h"
#include <memory>
#include "Systems.h"
#include "Components.h"
#include <string>
#include <SFML/Graphics.hpp>

//Processes entities with movement components
class SceneNodeVisitorMovement : public SceneNodeVisitor
{
public:
    SceneNodeVisitorMovement(MovementSystem& mSystem) : movementSystem{ mSystem } {}

    void ProcessNode(SceneNode& node) override;

private:
    //References to movement system
    MovementSystem& movementSystem;
};

//To render entities with rectangleShape, tileMap and particles components
class SceneNodeVisitorRender : public SceneNodeVisitor
{
public:
    SceneNodeVisitorRender(sf::RenderWindow& zRenderWindow) :renderWindow(zRenderWindow) {}

    void ProcessNode(SceneNode& node) override;

private:
    sf::RenderWindow& renderWindow;
};

//To process entities with UI components
class SceneNodeVisitorUI : public SceneNodeVisitor
{
public:
    SceneNodeVisitorUI(UISystem& system) : uiSystem(system) {}

    void ProcessNode(SceneNode& node) override;
private:
    UISystem& uiSystem;
};

//Processes entities with enemy components
class SceneNodeVisitorEnemy : public SceneNodeVisitor
{
public:
    SceneNodeVisitorEnemy(EnemiesSystem& eSystem) : enemiesSystem{ eSystem } {}

    void ProcessNode(SceneNode& node) override;

private:
    //References to movement system
    EnemiesSystem& enemiesSystem;
};

//Processes entities with rectangleShape components
class SceneNodeVisitorCollision : public SceneNodeVisitor
{
public:
    SceneNodeVisitorCollision(CollisionSystem& cSystem) : collisionSystem{ cSystem } {}

    void ProcessNode(SceneNode& node) override;

private:
    //References to movement system
    CollisionSystem& collisionSystem;
};

//Returns whether this entity collided with someone else or not
class SceneNodeVisitorFindCollision : public SceneNodeVisitor
{
public:
    SceneNodeVisitorFindCollision(Entity& entity) : targetEntity{ entity } {}

    void ProcessNode(SceneNode& node) override;

    //Stores pointer to entity with each it collided
    std::weak_ptr<Entity> collidedWithEntity;
private:
    //References to the target entity
    Entity& targetEntity;
};

//Manages particles components
class SceneNodeVisitorParticles : public SceneNodeVisitor
{
public:
    SceneNodeVisitorParticles() {}

    void ProcessNode(SceneNode& node) override;
};

//Change enemies behaviour according to the game state
class SceneNodeVisitorChangeEnemyBehaviour : public SceneNodeVisitor
{
public:
    bool explodedEnemy{false};
    SceneNodeVisitorChangeEnemyBehaviour() {}

    void ProcessNode(SceneNode& node) override;
};

//To render entities with UI component
class SceneNodeVisitorRenderUI : public SceneNodeVisitor
{
public:
    SceneNodeVisitorRenderUI(sf::RenderWindow& zRenderWindow) :renderWindow(zRenderWindow) {}

    void ProcessNode(SceneNode& node) override;

private:
    sf::RenderWindow& renderWindow;
};