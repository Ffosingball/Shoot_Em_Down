#include "SceneNodeVisitors.h"
#include "ECSGame.h"
#include "Components.h"
#include "CommonGameCode.h"
#include "EntitiesFunctions.h"
#include <random>
#include <iostream>
#include <SFML/Audio.hpp>

//Auxiliary visitors, are visitors which are not called by any system
//They are called by other visitors

//Find Collision
void SceneNodeVisitorFindCollision::ProcessNode(SceneNode& node)
{
    std::shared_ptr<Entity> spEntity = node.GetEntity().lock();
    //Check if pointer is valid
    if (spEntity != nullptr)
    {
        //Check that target do not references this current entity
        if (spEntity.get() != (&targetEntity))
        {
            //Check if entity has rectangleShape
            if (spEntity->HasComponent(ComponentType::RectangleShape))
            {
                //Get target component and transformable
                sf::Transformable targetTrans = targetEntity.GetTransformable();
                std::shared_ptr<RectangleShapeComponent> spTargetShape = GetRectangleShapeComponent(targetEntity);
                //Get current entity component and transformable
                sf::Transformable entityTrans = spEntity->GetTransformable();
                std::shared_ptr<RectangleShapeComponent> spEntityShape = GetRectangleShapeComponent(*spEntity);
                //If there is a collision then set this entity
                if (gel::DetectCollision(targetTrans.getPosition(), spTargetShape->shape.getSize(), entityTrans.getPosition(), spEntityShape->shape.getSize()))
                {
                    collidedWithEntity = spEntity;
                }
            }
        }
    }
}


//Change enemies behaviour
void SceneNodeVisitorChangeEnemyBehaviour::ProcessNode(SceneNode& node)
{
    std::shared_ptr<Entity> spEntity = node.GetEntity().lock();
    //Check if pointer is valid
    if (spEntity != nullptr)
    {
        //Check if entity has enemy component
        if (spEntity->HasComponent(ComponentType::Enemy))
        {
            //If player lost then fly out of the camera view
            if (ECSGame::Instance().GetGameState() == GameState::Lost)
            {
                //Get movement component
                std::shared_ptr<MovementComponent> spMovementCom = GetMovementComponent(*spEntity);
                //Get camera component
                std::shared_ptr<CameraComponent> sCameraCom = GetCameraFromCameraEntity();

                //Set all enemies in bootom part to fly to bottom of the screen
                //and all enemies at top part to fly to the top of the screen
                if (spEntity->GetTransformable().getPosition().y > sCameraCom->view.getCenter().y)
                    spMovementCom->velocity.y = -spMovementCom->velocity.x;
                else
                    spMovementCom->velocity.y = spMovementCom->velocity.x;
            }
            //If player won or restarted, then all enemies will blow up
            if (ECSGame::Instance().GetGameState() == GameState::Won || ECSGame::Instance().GetGameState() == GameState::Animation)
            {
                CreateExplosion(spEntity->GetTransformable().getPosition());
                explodedEnemy = true;
            }
        }
    }
}


//Render UI elements
void SceneNodeVisitorRenderUI::ProcessNode(SceneNode& node) 
{
    std::shared_ptr<Entity> spEntity = node.GetEntity().lock();
    //Check if pointer is valid
    if (spEntity != nullptr)
    {
        //Check if entity has UI component
        if (spEntity->HasComponent(ComponentType::UIPart))
        {
            //And now draw entities differently, depending on which other components they have
            if (spEntity->HasComponent(ComponentType::RectangleShape))
            {
                //Get component
                std::shared_ptr<RectangleShapeComponent> spEntityRecShape = GetRectangleShapeComponent(*spEntity);

                //Get absolute position of the entity in the world
                sf::RenderStates states;
                states.transform = node.GetCombinedTransform();
                //Draw entity
                renderWindow.draw(spEntityRecShape->shape, states);
            }

            if (spEntity->HasComponent(ComponentType::Text))
            {
                //Get component
                std::shared_ptr<TextComponent> spEntityUI = GetTextComponent(*spEntity);

                //Get absolute position of the entity in the world
                sf::RenderStates states;
                states.transform = node.GetCombinedTransform();
                //Draw entity
                renderWindow.draw(*(spEntityUI->text), states);
            }

            if (spEntity->HasComponent(ComponentType::Particles))
            {
                //Get component
                std::shared_ptr<ParticlesComponent> spParticles = GetParticlesComponent(*spEntity);
                //Draw entity
                spParticles->particleSystems.Draw(renderWindow, node.GetCombinedTransform());
            }
        }
    }
}