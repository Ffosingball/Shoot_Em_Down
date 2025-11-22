#pragma once
#include <SFML/Graphics.hpp>
#include "ParticlesAndConfigs.h"

//More specific particles configuration
class SimpleParticlesConfig :public ParticleConfig 
{
public:
    sf::Color color = sf::Color::Yellow;
    //Values and times when particle should change its transparency
    std::vector<int> alphaValues = { 0,255,255,0 };
    std::vector<float> alphaTimes = { 0,1.f,4.f,5.f };
    //Time which particle will exist
    float particleLifetime = 5.f;
    //Velocity of the particle
    float velocityMag = 40;
    //Size of the particles
    sf::Vector2f rangeSizeX{ 15.f,20.f };
    sf::Vector2f rangeSizeY{ 15.f,20.f };
    //Velocity
    sf::Vector2f rangeVelocityX{ -1.f,1.f };
    sf::Vector2f rangeVelocityY{ -1.f,1.f };
    //Position where particles will be emitted
    sf::Vector2f emmisionSurfaceX{ 1.f,-1.f };
    sf::Vector2f emmisionSurfaceY{ -1.f,1.f };

	~SimpleParticlesConfig() override = default;
    virtual void Init(Particle& p) const override;
    virtual void Update(Particle& p, float dt) const override;

protected:
    //This function returns index and number (from 0 to 1) where to get correct value 
    // for color or transparency
    std::pair<int, float> PiecewiseLinearInterpolation(float time, const std::vector<float>& timePoints) const;
};

//Fumes configurations
class Fumes : public SimpleParticlesConfig
{
public:
    Fumes(bool enemyPlane);
};

//Explosion configuration
class Explosion : public SimpleParticlesConfig
{
public:
    //Stores color values and when to change them
    std::vector<sf::Vector3i> colorValues;
    std::vector<float> colorTimes;

    Explosion();

    void Update(Particle& p, float dt) const override;
};