#include <SFML/Graphics.hpp>
#include "ParticlesConfigurations.h"
#include "CommonGameCode.h"
#include <iostream>

void SimpleParticlesConfig::Init(Particle& p) const 
{
    //Set particle size
    float sizeX = gel::Randf(rangeSizeX.x, rangeSizeX.y);
    float sizeY = gel::Randf(rangeSizeY.x, rangeSizeY.y);
    p.shape = sf::RectangleShape(sf::Vector2f(sizeX, sizeY));

    //Set texture
    p.shape.setTexture(pTexture);

    //set velocity
    p.velocity.x = gel::Randf(rangeVelocityX.x, rangeVelocityX.y);
    p.velocity.y = gel::Randf(rangeVelocityY.x, rangeVelocityY.y);;
    p.velocity = p.velocity.normalized();
    p.velocity *= velocityMag;

    // Set position when particle will appear
    p.position.x = gel::Randf(emmisionSurfaceX.x, emmisionSurfaceX.y);
    p.position.y = gel::Randf(emmisionSurfaceY.x, emmisionSurfaceY.y);;

    //Set lifetime
    p.timeToLive = particleLifetime;
}


// Return the segment start index in "timePoints", and how far in we are, as a percentage
std::pair<int, float> SimpleParticlesConfig::PiecewiseLinearInterpolation(float time, const std::vector<float>& timePoints) const
{
    if (timePoints.size() >= 2)
    {
        // Ensure it's in range
        if (time < timePoints.front())
            time = timePoints.front();
        else if (time > timePoints.back())
            time = timePoints.back();

        //go through the loop until we find the correct time piece
        for (int i = 1; i < timePoints.size(); ++i)
        {
            //Check if current time is in this piece
            if (time <= timePoints[i])
            {
                float timeBound0 = timePoints[i - 1];
                float timeBound1 = timePoints[i];
                float t = (time - timeBound0) / (timeBound1 - timeBound0);
                //Return position of left time bound and percentage on how far we are in this piece
                return std::make_pair(i - 1, t);
            }
        }
    }
    return { 0,0 }; // We shouldn't reach this
}


void SimpleParticlesConfig::Update(Particle& p, float dt) const 
{
    //Update position
    p.position += p.velocity * dt;
    p.shape.setPosition(p.position);

    //Update transparency of the particle
    sf::Color newColor = color;
    float timeAlive = particleLifetime - p.timeToLive;
    auto indexAndPercent = PiecewiseLinearInterpolation(timeAlive, alphaTimes);
    int i0 = indexAndPercent.first;
    int i1 = i0 + 1;
    float t = indexAndPercent.second;
    newColor.a = gel::linearInterpolation(alphaValues[i0], alphaValues[i1], t);
    p.shape.setFillColor(newColor);
}

//Set fumes configuration
Fumes::Fumes(bool enemyPlane) 
{
    color = sf::Color(100,100,100);
    alphaValues = { 0,255,0 };
    alphaTimes = { 0,0.5f,4.f,};
    particleLifetime = 4.f;
    emissionRate = 13.0f;
    //Check if fumes is for enemy plane or for the player
    if(enemyPlane)
        velocityMag = 10;
    else
        velocityMag = -10;

    rangeSizeX = sf::Vector2f{ 20.f,30.f };
    rangeSizeY = sf::Vector2f{ 20.f,30.f };
    rangeVelocityX = sf::Vector2f{ 0.9f,1.f };
    rangeVelocityY = sf::Vector2f{ -0.1f,0.1f };
    emmisionSurfaceX = sf::Vector2f{ -1.f,1.f };
    emmisionSurfaceY = sf::Vector2f{ -10.f,10.f };
}

//Set explosion configuration
Explosion::Explosion()
{
    color = sf::Color(255, 255, 0);
    alphaValues = { 0,255,255,0 };
    alphaTimes = { 0,0.2f,1.f,1.4f };
    particleLifetime = 1.4f;
    emissionRate = 100.f;
    emissionDuration = 1.f;
    velocityMag = 25;
    rangeSizeX = sf::Vector2f{ 20.f,25.f };
    rangeSizeY = sf::Vector2f{ 20.f,25.f };
    rangeVelocityX = sf::Vector2f{ -1.f,1.f };
    rangeVelocityY = sf::Vector2f{ -1.f,1.f };
    emmisionSurfaceX = sf::Vector2f{ -1.f,1.f };
    emmisionSurfaceY = sf::Vector2f{ -1.f,1.f };

    colorValues.emplace_back(sf::Vector3i{ 255,255,0 });
    colorValues.emplace_back(sf::Vector3i{ 255,0,0 });
    colorValues.emplace_back(sf::Vector3i{ 0,0,0 });
    colorTimes = { 0.f,0.7f,1.4f };
}


void Explosion::Update(Particle& p, float dt) const
{
    //Call same function from the parent
    SimpleParticlesConfig::Update(p, dt);

    //Set new color
    sf::Color newColor = color;
    float timeAlive = particleLifetime - p.timeToLive;
    auto indexAndPercent = PiecewiseLinearInterpolation(timeAlive, colorTimes);
    int i0 = indexAndPercent.first;
    int i1 = i0 + 1;
    float t = indexAndPercent.second;
    newColor.r = gel::linearInterpolation(colorValues[i0].x, colorValues[i1].x, t);
    newColor.g = gel::linearInterpolation(colorValues[i0].y, colorValues[i1].y, t);
    newColor.b = gel::linearInterpolation(colorValues[i0].z, colorValues[i1].z, t);
    p.shape.setFillColor(newColor);
}