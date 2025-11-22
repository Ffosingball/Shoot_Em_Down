#include "SceneNodeVisitors.h"
#include "ECSGame.h"
#include "Components.h"
#include "CommonGameCode.h"
#include "EntitiesFunctions.h"
#include <random>
#include <iostream>
#include <SFML/Audio.hpp>


//Check if it needs to delete bullet, then it signals it, otherwise just deletes object
void CheckIfDeletingBullet(std::shared_ptr<Entity> sEntity)
{
    //Check if it is a bullet
    if (sEntity->HasComponent(ComponentType::Bullet)) 
    {
        std::shared_ptr<BulletComponent> spBulletCom = GetBulletComponent(*sEntity);
        //Check if bullet fired by enemy
        if (spBulletCom->firedByEnemy)
            signals::onBulletAvoided();
    }
    //Delete entity
    signals::onDeleteEntity(sEntity);
}

//Check if entity reached target position or not
bool ReachedTargetPosition(const sf::Vector2f& currentPosition, const sf::Vector2f& targetPosition, const sf::Vector2f& velocity) 
{
    bool closeByY{ false };
    bool closeByX{ false };
    //Check by y axis
    if(velocity.y==0.f)
        closeByY = true;
    else if (velocity.y < 0.f) 
    {
        if (currentPosition.y < targetPosition.y)
            closeByY = true;
    }
    else 
    {
        if (currentPosition.y > targetPosition.y)
            closeByY = true;
    }

    //Check by x axis
    if(velocity.x == 0.f)
        closeByX = true;
    else if (velocity.x < 0.f)
    {
        if (currentPosition.x < targetPosition.x)
            closeByX = true;
    }
    else
    {
        if (currentPosition.x > targetPosition.x)
            closeByX = true;
    }

    //Return result by y and x axis
    return closeByY && closeByX;
}


void SceneNodeVisitorMovement::ProcessNode(SceneNode& node)
{
    std::shared_ptr<Entity> spEntity = node.GetEntity().lock();
    //Get deltatime
    float dt = ECSGame::Instance().GetDeltaTime();
    //Check if pointer not null
    if (spEntity != nullptr)
    {
        //Check if entity has a movement component
        if (spEntity->HasComponent(ComponentType::Movement))
        {
            //If yes, then check which other component it has
            if (spEntity->HasComponent(ComponentType::Camera)) 
            {
                //If it has camera component then move camera, not entity
                std::shared_ptr<CameraComponent> spCameraCom = GetCameraComponent(*spEntity);
                //Check if we can move camera or not
                if (spCameraCom->moveCamera)
                {
                    //Get movement component
                    std::shared_ptr<MovementComponent> spMovementCom = GetMovementComponent(*spEntity);

                    //Move camera
                    spCameraCom->view.move({ spMovementCom->velocity.x * dt,0 });

                    //Check if camera reached right end of the tilemap
                    if (spCameraCom->view.getCenter().x > spCameraCom->rightBorder)
                    {
                        //If it reached end, then player won
                        spCameraCom->view.move({ -spMovementCom->velocity.x * dt,0 });
                        ECSGame::Instance().SetGameState(GameState::Won);
                        spCameraCom->moveCamera = false;
                        signals::onGameWon();
                        signals::onPlayManyExplosionsSounds(static_cast<int>(gel::Randf(3.f,6.f)));
                    }//Check if camera reached left side
                    else if (spCameraCom->view.getCenter().x < spCameraCom->leftBorder)
                    {
                        //If camera reached left side, then it finished its animation
                        spCameraCom->view.move({ -spMovementCom->velocity.x * dt,0 });
                        spCameraCom->moveCamera = false;
                        ECSGame::Instance().SetGameState(GameState::Start);
                        //Create start menu
                        InitializeStartUI();
                        signals::onDifficultyLevelShow();

                        signals::onDeleteEntitiesByName("Explosion");
                        //Reset camera
                        ResetCamera();
                    }
                }
            }
            else if (spEntity->HasComponent(ComponentType::Player))
            {
                //Get component
                std::shared_ptr<MovementComponent> spMovementCom = GetMovementComponent(*spEntity);
                
                //If it is a player, then move player's entity in specific direction
                sf::Transformable transformable = spEntity->GetTransformable();
                sf::Vector2f position = transformable.getPosition();
                position.x += spMovementCom->velocity.x * movementSystem.direction.x * dt;
                position.y += spMovementCom->velocity.y * movementSystem.direction.y * dt;

                //Get player size
                std::shared_ptr<RectangleShapeComponent> spRecCom = GetRectangleShapeComponent(*spEntity);
                sf::Vector2f playerSize = spRecCom->shape.getSize();

                //Get camera entity
                std::shared_ptr<CameraComponent> sCameraCom = GetCameraFromCameraEntity();

                // Check that players position within the camera view
                position.x = gel::clamp(position.x, sCameraCom->view.getCenter().x - (sCameraCom->view.getSize().x / 2.f) + playerSize.x / 2.f, sCameraCom->view.getCenter().x + (sCameraCom->view.getSize().x / 2.f) - playerSize.x / 2.f);
                position.y = gel::clamp(position.y, sCameraCom->view.getCenter().y - (sCameraCom->view.getSize().y / 2.f) + playerSize.y / 2.f, sCameraCom->view.getCenter().y + (sCameraCom->view.getSize().y / 2.f) - playerSize.y / 2.f);

                //Check if it is animation state or not
                if (ECSGame::Instance().GetGameState() == GameState::Animation) 
                {
                    //If it is, then if player plane reached that position, 
                    //then end of the animation and game can start
                    if (position.x > ECSGame::Instance().GetWindowSize().x * 0.1f) 
                    {
                        InitializeGameUI();
                        ECSGame::Instance().SetGameState(GameState::Game);
                    }
                }

                //Apply movement
                transformable.setPosition(position);
                spEntity->SetTransformable(transformable);
            }
            else if (spEntity->HasComponent(ComponentType::UIPart)) 
            {
                //Get components
                std::shared_ptr<UIPartComponent> spTextCom = GetUIPartComponent(*spEntity);
                std::shared_ptr<MovementComponent> spMovementCom = GetMovementComponent(*spEntity);

                //Check if it have special movement logic
                if (spTextCom->moveIt) 
                {
                    sf::Transformable transformable = spEntity->GetTransformable();
                    sf::Vector2f position = transformable.getPosition();

                    //Move UIComponent
                    position.x += spMovementCom->velocity.x * dt;
                    position.y += spMovementCom->velocity.y * dt;
                    //Check if it reached target position
                    if (ReachedTargetPosition(position, spTextCom->targetPosition,spMovementCom->velocity))
                    {
                        //If it reached target position, then destroy it if needed
                        //or just stop moving this entity
                        if (spTextCom->destroyAtTarget)
                        {
                            signals::onDeleteEntity(spEntity);
                        }
                        else
                        {
                            position.x = spTextCom->targetPosition.x;
                            position.y = spTextCom->targetPosition.y;
                            spTextCom->moveIt = false;
                            spMovementCom->velocity = { 0.f,0.f };
                        }
                    }

                    //Apply movement
                    transformable.setPosition(position);
                    spEntity->SetTransformable(transformable);
                }
            }
            else 
            {
                //Get component
                std::shared_ptr<MovementComponent> spMovementCom = GetMovementComponent(*spEntity);

                //Move it
                sf::Transformable transformable = spEntity->GetTransformable();
                sf::Vector2f position = transformable.getPosition();
                position.x += spMovementCom->velocity.x * dt;
                position.y += spMovementCom->velocity.y * dt;

                //Get camera entity
                std::shared_ptr<CameraComponent> sCameraCom = GetCameraFromCameraEntity();

                //Check that entity position within the camera view
                //If not then delete it
                float allowedDistanceOutOfViewX = 40.f;
                float allowedDistanceOutOfViewY = 12.f;
                if (position.x > sCameraCom->view.getCenter().x + (sCameraCom->view.getSize().x / 2.f)+allowedDistanceOutOfViewX)
                    CheckIfDeletingBullet(node.GetEntity().lock());
                else if (position.x < sCameraCom->view.getCenter().x - (sCameraCom->view.getSize().x / 2.f)- allowedDistanceOutOfViewX)
                    CheckIfDeletingBullet(node.GetEntity().lock());
                else if (position.y > sCameraCom->view.getCenter().y + (sCameraCom->view.getSize().y / 2.f) + allowedDistanceOutOfViewY)
                    CheckIfDeletingBullet(node.GetEntity().lock());
                else if (position.y < sCameraCom->view.getCenter().y - (sCameraCom->view.getSize().y / 2.f) - allowedDistanceOutOfViewY)
                    CheckIfDeletingBullet(node.GetEntity().lock());
                else
                {
                    //Apply movement
                    transformable.setPosition(position);
                    spEntity->SetTransformable(transformable);
                }
            }
        }
    }
}


//UI processing function
void SceneNodeVisitorUI::ProcessNode(SceneNode& node) 
{
    std::shared_ptr<Entity> spEntity = node.GetEntity().lock();
    //Get deltatime
    float dt = ECSGame::Instance().GetDeltaTime();
    //Check that pointer is valid
    if (spEntity != nullptr)
    {
        //Check if entity has UI component
        if (spEntity->HasComponent(ComponentType::UIPart))
        {
            //Get UI component
            std::shared_ptr<UIPartComponent> spEntityText = GetUIPartComponent(*spEntity);

            //Check if this text blinks or not
            if (spEntityText->isBlinking) 
            {
                //Process blink time
                spEntityText->blinkTime += dt;
                //Check if it is flatline or not
                if(spEntityText->flatLine)
                {
                    if (spEntityText->blinkTime > spEntityText->flatLinePeriod)
                    {
                        //If flat period finishd than start change visibility
                        spEntityText->blinkTime -= spEntityText->flatLinePeriod;
                        spEntityText->flatLine = false;
                    }
                }
                else
                {
                    if (spEntityText->blinkTime > spEntityText->blinkingPeriod)
                    {
                        //If blinkPeriod finished then show same visibility for some
                        //period of time
                        spEntityText->decreasingVisibility = !spEntityText->decreasingVisibility;
                        spEntityText->blinkTime -= spEntityText->blinkingPeriod;
                        spEntityText->flatLine = true;
                    }
                    //Get component
                    std::shared_ptr<TextComponent> spEntityTextCom = GetTextComponent(*spEntity);

                    //Get color values
                    sf::Color color = spEntityTextCom->text->getFillColor();
                    sf::Color outlineColor = spEntityTextCom->text->getOutlineColor();
                    uint8_t alphaValue{255};
                    //Check which state the text is and get correct alpha value
                    if (spEntityText->decreasingVisibility)
                        alphaValue = gel::linearInterpolation(255, 90, spEntityText->blinkTime / spEntityText->blinkingPeriod);
                    else
                        alphaValue = gel::linearInterpolation(90, 255, spEntityText->blinkTime / spEntityText->blinkingPeriod);

                    //Set new alphaValue
                    color.a = alphaValue;
                    outlineColor.a = alphaValue;
                    spEntityTextCom->text->setFillColor(color);
                    spEntityTextCom->text->setOutlineColor(outlineColor);
                }
            }
        }
    }
}

//Render processing function
void SceneNodeVisitorRender::ProcessNode(SceneNode& node)
{
    std::shared_ptr<Entity> spEntity = node.GetEntity().lock();
    //Get deltatime
    float dt = ECSGame::Instance().GetDeltaTime();
    //Check if pointer is valid
    if (spEntity != nullptr)
    {
        //Check that entity does not have UI component
        if (!spEntity->HasComponent(ComponentType::UIPart))
        {
            //Now check which type of components entity has, becuase
            //different components will be displayed differently
            if (spEntity->HasComponent(ComponentType::TileMap))
            {
                //Get component
                std::shared_ptr<Component> spEntityTileMapBase = spEntity->FindComponent(ComponentType::TileMap).lock();
                std::shared_ptr<TileMapComponent> spEntityTileMap = std::static_pointer_cast<TileMapComponent>(spEntityTileMapBase);

                //Draw entity
                spEntityTileMap->tileMap.Render(renderWindow, node.GetCombinedTransform());
            }

            if (spEntity->HasComponent(ComponentType::RectangleShape))
            {
                //Get component
                std::shared_ptr<Component> spEntityRecShapeBase = spEntity->FindComponent(ComponentType::RectangleShape).lock();
                std::shared_ptr<RectangleShapeComponent> spEntityRecShape = std::static_pointer_cast<RectangleShapeComponent>(spEntityRecShapeBase);

                //Get absolute position of the entity in the world
                sf::RenderStates states;
                states.transform = node.GetCombinedTransform();
                //Draw entity
                renderWindow.draw(spEntityRecShape->shape, states);
            }

            if (spEntity->HasComponent(ComponentType::Particles))
            {
                //Get component
                std::shared_ptr<Component> spEntityParticlesBase = spEntity->FindComponent(ComponentType::Particles).lock();
                std::shared_ptr<ParticlesComponent> spParticles = std::static_pointer_cast<ParticlesComponent>(spEntityParticlesBase);

                //Draw entity
                spParticles->particleSystems.Draw(renderWindow, node.GetCombinedTransform());
            }
        }
    }
}

//Process Enemies
void SceneNodeVisitorEnemy::ProcessNode(SceneNode& node)
{
    std::shared_ptr<Entity> spEntity = node.GetEntity().lock();
    //Get deltatime
    float dt = ECSGame::Instance().GetDeltaTime();
    //Check if pointer is valid
    if (spEntity != nullptr)
    {
        //Check if it has enemy component
        if (spEntity->HasComponent(ComponentType::Enemy))
        {
            //Get component
            std::shared_ptr<EnemyComponent> spEnemyCom = GetEnemyComponent(*spEntity);
            std::shared_ptr<MovementComponent> spMovementCom = GetMovementComponent(*spEntity);

            spEnemyCom->timeLeftToFire -= dt;

            //If time passed then fire a bullet
            if (spEnemyCom->timeLeftToFire < 0)
            {
                CreateBullet(spEntity.get());
                spEnemyCom->timeLeftToFire += enemiesSystem.firePeriod;
            }

            //Now check if enemy on the y borders of the camera view, then change 
            // its direction, so player always will be able to destroy it
            std::shared_ptr<CameraComponent> sCameraCom = GetCameraFromCameraEntity();
            sf::Vector2f position = spEntity->GetTransformable().getPosition();
            if ((position.y > sCameraCom->view.getCenter().y + (sCameraCom->view.getSize().y / 2.f) || position.y < sCameraCom->view.getCenter().y - (sCameraCom->view.getSize().y / 2.f)) && ECSGame::Instance().GetGameState()==GameState::Game)
            {
                spMovementCom->velocity.y = -spMovementCom->velocity.y;
            }
        }
    }
}

//This function destroys player and change game state to LOST
void GameOver(std::weak_ptr<Entity> player) 
{
    ECSGame::Instance().SetGameState(GameState::Lost);
    CreateExplosion(player.lock()->GetTransformable().getPosition());
    signals::onPlayExplosionSound();
    signals::onDeleteEntity(player);
    signals::onGameLost();
}

//Processes entity collision with enemy entity
void ProcessEntityCollissionWithEnemy(std::shared_ptr<Entity> spEntity,float powerUpDropRate,float powerUp1Correlation)
{
    //Find node which contains all enemies
    std::weak_ptr<Entity> wSpEntity = ECSGame::Instance().GetEntityManager().FindEntity("Enemies");
    std::shared_ptr<Entity> sSpEntity = wSpEntity.lock();
    SceneNode* nodePtr = ECSGame::Instance().GetSceneRoot().FindChild(*sSpEntity);

    //Find any collision 
    SceneNodeVisitorFindCollision visitor(*spEntity);
    nodePtr->AcceptVisitor(visitor);
    //Check if entity collided with enemy
    std::shared_ptr<Entity> ptr = visitor.collidedWithEntity.lock();
    if (ptr != nullptr)
    {
        //Create power up with certain chance
        if (gel::Randf01() < powerUpDropRate)
        {
            //Choose which type of power up to create
            if (gel::Randf01() < powerUp1Correlation)
                CreatePowerUp(1, ptr->GetTransformable().getPosition());
            else
                CreatePowerUp(2, ptr->GetTransformable().getPosition());
        }

        //Then destroy enemy
        CreateExplosion(ptr->GetTransformable().getPosition());
        signals::onDeleteEntity(visitor.collidedWithEntity);
        signals::onPlayExplosionSound();
        signals::onEnemyDestroyed();

        //If player collided with enemy
        if (spEntity->GetName() == "Player")
        {
            std::shared_ptr<PlayerComponent> spPlayerCom = GetPlayerComponent(*spEntity);
            //Check if player still has durability left, then just decrease durability left
            //Otherwise gameover
            if (spPlayerCom->durability > 0)
            {
                spPlayerCom->durability--;
                signals::onDurabilityUsed(spPlayerCom->durability);
            }
            else
                GameOver(spEntity);
        }
        else
            signals::onDeleteEntity(spEntity);
    }
}

//Process Collisions
void SceneNodeVisitorCollision::ProcessNode(SceneNode& node)
{
    std::shared_ptr<Entity> spEntity = node.GetEntity().lock();
    //Check if pointer is valid
    if (spEntity != nullptr)
    {
        //Check if entity has rectangleShape
        if (spEntity->HasComponent(ComponentType::RectangleShape))
        {
            if (spEntity->GetName() == "Player")
            {
                //Check collision of player with enemies or powerUps
                ProcessEntityCollissionWithEnemy(spEntity,collisionSystem.powerUpDropRate,collisionSystem.powerUp1Correlation);

                //Process collision with powerUps
                //Find node which contains all powerUps
                std::weak_ptr<Entity> wSpEntity = ECSGame::Instance().GetEntityManager().FindEntity("PlayerNode");
                std::shared_ptr<Entity> sSpEntity = wSpEntity.lock();
                SceneNode* nodePtr = ECSGame::Instance().GetSceneRoot().FindChild(*sSpEntity);

                //Find if player collided with powerUp or not
                SceneNodeVisitorFindCollision visitor(*spEntity);
                nodePtr->AcceptVisitor(visitor);
                //Check if entity collided with powerUp
                std::shared_ptr<Entity> ptr = visitor.collidedWithEntity.lock();
                if (ptr != nullptr)
                {
                    //Get component
                    std::shared_ptr<PlayerComponent> spPlayer = GetPlayerComponent(*spEntity);
                    //Check if player has player component
                    if (spPlayer != nullptr)
                    {
                        signals::onPlayPowerUpSound();
                        //Check what the type of power up this is
                        if (ptr->GetName() == "PowerUp1")
                        {
                            //Checkif shield already exist
                            std::weak_ptr<Entity> wSh = ECSGame::Instance().GetEntityManager().FindEntity("Shield");
                            std::shared_ptr<Entity> sSh = wSh.lock();
                            if (sSh == nullptr)
                                CreateShield();
                            else
                                ResetShield();
                            //Make player invulnerable
                            signals::onDeleteEntity(ptr);
                            spPlayer->invulnerable = true;
                            spPlayer->invulnerabilityTimeLeft = 15.f;
                        }
                        else if (ptr->GetName() == "PowerUp2")
                        {
                            //Add durability
                            signals::onDeleteEntity(ptr);
                            InitializeDurabilityBars(6);
                            spPlayer->durability = 6;
                        }
                    }
                }
            }
            else if (spEntity->GetName() == "Bullet")
            {
                //Check collision of bullet with enemy or player
                //Get component
                std::shared_ptr<BulletComponent> spBulletCom = GetBulletComponent(*spEntity);

                if(spBulletCom->firedByEnemy)
                {
                    //Get player entity
                    std::weak_ptr<Entity> wSpEntity = ECSGame::Instance().GetEntityManager().FindEntity("Player");
                    std::shared_ptr<Entity> sSpEntity = wSpEntity.lock();
                    //Check that player exist
                    if (sSpEntity != nullptr)
                    {
                        //Get player component and transformable
                        sf::Transformable playerTrans = sSpEntity->GetTransformable();
                        std::shared_ptr<RectangleShapeComponent> spPlayerShape = GetRectangleShapeComponent(*sSpEntity);
                        //Get current entity component and transformable
                        sf::Transformable entityTrans = spEntity->GetTransformable();
                        std::shared_ptr<RectangleShapeComponent> spEntityShape = GetRectangleShapeComponent(*spEntity);
                        //If there is a collision then decrease player hp
                        float divideSizeBy = 1.3f;
                        if (gel::DetectCollision(playerTrans.getPosition(), spPlayerShape->shape.getSize()/divideSizeBy, entityTrans.getPosition(), spEntityShape->shape.getSize()/divideSizeBy))
                        {
                            //Destrioy bullet
                            signals::onDeleteEntity(spEntity);

                            std::shared_ptr<PlayerComponent> spPlayerCom = GetPlayerComponent(*sSpEntity);

                            //Check if player has player component
                            if (spPlayerCom != nullptr)
                            {
                                //Check if player is invulnerable
                                if (!spPlayerCom->invulnerable)
                                {
                                    spPlayerCom->hp--;
                                    signals::onPlayerHurt(spPlayerCom->hp);
                                    //Check if hp equal or less than 0 then gameover
                                    if (spPlayerCom->hp <= 0)
                                        GameOver(wSpEntity);
                                }
                            }
                        }
                    }
                }
                else 
                {
                    //Collision of bullets fired by player with enemies
                    ProcessEntityCollissionWithEnemy(spEntity, collisionSystem.powerUpDropRate, collisionSystem.powerUp1Correlation);
                }
            }
        }
    }
}


//Process particles
void SceneNodeVisitorParticles::ProcessNode(SceneNode& node)
{
    std::shared_ptr<Entity> spEntity = node.GetEntity().lock();
    //Get deltatime
    float dt = ECSGame::Instance().GetDeltaTime();
    //Check that pointer is valid
    if (spEntity != nullptr)
    {
        //Check if entity has UI component
        if (spEntity->HasComponent(ComponentType::Particles))
        {
            //Get component
            std::shared_ptr<ParticlesComponent> spParticlesCom = GetParticlesComponent(*spEntity);
            //Update particles
            spParticlesCom->particleSystems.Update(dt);

            //if it is explosion, check that it is finished and then destroy it
            if (spEntity->GetName() == "Explosion") 
            {
                if (spParticlesCom->particleSystems.IsFinished()) 
                {
                    signals::onDeleteEntity(spEntity);
                }
            }
        }
    }
}
