#include <SFML/Graphics.hpp>
#include <iostream>
#include<cstdlib>
#include <time.h>
#include <vector>
#include <array>
#include <memory>

const int SCREEN_HEIGHT_PER_BLOCK = 30;
const int SCREEN_WIDTH_PER_BLOCK = 20;
const int PIXELS_PER_BLOCK = 16;
const int SCREEN_WIDTH = (SCREEN_WIDTH_PER_BLOCK) * PIXELS_PER_BLOCK;
const int SCREEN_HEIGHT = (SCREEN_HEIGHT_PER_BLOCK) * PIXELS_PER_BLOCK;

// --- Figure  ---
//   0       1
//   2       3
//   4       5
//   6       7

std::array <char, 7> shapeSeq = {'I','Z','S','T','L','J','O'};
std::map< char, std::vector<int> > shapesMap = {       
    {'I',   {1,3,5,7}},
    {'Z',   {2,4,5,7}},
    {'S',   {3,5,4,6}},
    {'T',   {3,5,4,7}},
    {'L',   {2,3,5,7}},
    {'J',   {3,5,7,6}},
    {'O',   {2,3,4,5}}
};

enum direction {
    centre = 0,
    up,
    down,
    right,
    left
};

bool checkRight(const sf::Sprite& sprite, const sf::Sprite& otherSprite) {
    sf::FloatRect rightBlock= sprite.getGlobalBounds();
    rightBlock.left += PIXELS_PER_BLOCK;
    return otherSprite.getGlobalBounds().intersects(rightBlock);
}

bool checkLeft(const sf::Sprite& sprite, const sf::Sprite& otherSprite) {
    sf::FloatRect leftBlock= sprite.getGlobalBounds();
    leftBlock.left -= PIXELS_PER_BLOCK;
    return otherSprite.getGlobalBounds().intersects(leftBlock);
}

bool checkDown(const sf::Sprite& sprite, const sf::Sprite& otherSprite) {
    sf::FloatRect downBlock= sprite.getGlobalBounds();
    downBlock.top += 1;
    return otherSprite.getGlobalBounds().intersects(downBlock);
}

bool isOn(const sf::Sprite& sprite, const sf::Sprite& otherSprite) {
    return otherSprite.getGlobalBounds().intersects(sprite.getGlobalBounds());
}

bool isOn(const std::vector <sf::Sprite>& sprite, const std::vector <sf::Sprite>& otherSprite) {
    for (auto& osprite : otherSprite){
        for (auto& s:sprite){
            if (osprite.getGlobalBounds().intersects(s.getGlobalBounds())){
                return true;
            }
        }
    }
    return false;
}

bool isOn(const std::vector <sf::Sprite>& sprite, const sf::Sprite& otherSprite) {
    for (auto& s:sprite){
        if (otherSprite.getGlobalBounds().intersects(s.getGlobalBounds())){
            return true;
        }
    }
    return false;
}

direction checkCollision(sf::Sprite sourceMino, sf::Sprite targetMino){
    if(checkDown(sourceMino, targetMino)){
        return down;
    }
    if(checkLeft(sourceMino, targetMino)){
        return left;
    }
    if(checkRight(sourceMino, targetMino)){
            return right;
    }
    return centre;
}

int main(){
    srand((unsigned) time(NULL));
    sf::Clock clock;
    sf::Time elapsedTime = sf::Time::Zero;
    sf::Time timeSinceLastFrame = sf::Time::Zero;
    const sf::Time TIME_PER_FRAME = sf::seconds(1.f/60.f);
    sf::Time deltaTime = TIME_PER_FRAME;

    float fallVelocity; 
    float minoFallDistanceTotal = 0;
    
    enum direction dir = down;

    std::vector<sf::Sprite> minoBuffer;
    minoBuffer.reserve(SCREEN_WIDTH_PER_BLOCK * SCREEN_HEIGHT_PER_BLOCK);
    
    // Init Render Window
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT) , "The Game!");

    // <- Wall Texture ->
    // textures for wall
    std::array<sf::Sprite, 4> wallSpriteArray;
    auto  wallTexture = std::make_unique<sf::Texture>();
    if(wallTexture->loadFromFile("assets/images/wall.png")) {
        wallTexture->setRepeated(1);
    }
    for (auto& wall : wallSpriteArray) {
        wall.setTexture(*(wallTexture.get()));
    }
    wallSpriteArray[0].setTextureRect({0, 0, SCREEN_WIDTH, PIXELS_PER_BLOCK});  // defining the texture for top wall
    wallSpriteArray[1].setTextureRect({0, 0, PIXELS_PER_BLOCK, SCREEN_HEIGHT});  // definfing the texture for left wall
    wallSpriteArray[2].setTextureRect({0, 0, PIXELS_PER_BLOCK, SCREEN_HEIGHT});  // defining the texture for right wall
    wallSpriteArray[2].setPosition({SCREEN_WIDTH-PIXELS_PER_BLOCK, 0});  // definfing position for right wall
    wallSpriteArray[3].setTextureRect({0, 0, SCREEN_WIDTH, PIXELS_PER_BLOCK});  // bottom wall
    wallSpriteArray[3].setPosition({0, SCREEN_HEIGHT-PIXELS_PER_BLOCK});  // definfing position for right wall

    // <- Mino Textures and Sprites ->
    // loading texture
    sf::Texture minoTexture;
    minoTexture.loadFromFile("assets/images/tiles.png");
    std::array <sf::Sprite, 8> minoColorSprites;
    std::array <sf::Sprite, 4> currentMino;
    for (int i=0; i<8; i++) {
        minoColorSprites[i].setTexture(minoTexture);
        minoColorSprites[i].setTextureRect(sf::IntRect( ((PIXELS_PER_BLOCK) + 2)*i, 0, PIXELS_PER_BLOCK, PIXELS_PER_BLOCK ) );
    }

    // setting texture to mino
    for (int i= 0; i<4; i++){
        currentMino[i] = minoColorSprites[0];
    }

    // setting the shape of the Mino:
    char shape= 'T';
    try {
        std::vector shapeVec = shapesMap.at(shape);
        for (int i=0; i<4; i++) {
            currentMino[i].setPosition(PIXELS_PER_BLOCK + (PIXELS_PER_BLOCK * (shapeVec[i]%2)), 
                                        PIXELS_PER_BLOCK + (PIXELS_PER_BLOCK * (shapeVec[i]/2)));
        }
    }
    catch (const std::out_of_range&) {
        std::cout << "Key \"" << shape << "\" not found" << std::endl;
        return -1;
    }

    bool continueGameFlag = true;
// Game Loop:
    while (window.isOpen() && continueGameFlag)
    {
        timeSinceLastFrame += clock.restart();
        sf::Event e;
        dir = down; // reseting the direction of 
        fallVelocity = 300;
         
        // <- Drawing -> // 
        // clearing the screen
        window.clear(sf::Color::Black);
                
        // drawing the wall
        for (auto& wall: wallSpriteArray) {
            window.draw(wall);
        }

        // drawing all previous minos 
        for (auto& mino: minoBuffer)
            window.draw(mino);
        
        // draw current mino
        for (auto& mino : currentMino)
            window.draw(mino);

        window.display();

        while (timeSinceLastFrame > TIME_PER_FRAME)
        {
            timeSinceLastFrame -= TIME_PER_FRAME;
            elapsedTime += deltaTime;

            if(elapsedTime.asSeconds() > 0.07) {
                elapsedTime = sf::Time::Zero;
                
                // <- Check Input -> //
                window.pollEvent(e);
                if (e.type == sf::Event::Closed)    // if window is closed
                    window.close();
                if (e.type == sf::Event::KeyPressed){   // if keyboard is pressed
                    if (e.key.code == sf::Keyboard::Up) dir = up;
                    else if (e.key.code == sf::Keyboard::Left) dir = left;
                    else if (e.key.code == sf::Keyboard::Right) dir = right;
                    else if (e.key.code == sf::Keyboard::Down) {dir = down; fallVelocity *= 3;}; //!! Bug! goes throught the wall
                }

                // <- Collision check ->
                // for key pressed 
                for(int i =0; i<4; i++) {
                    switch (dir) {
                        case right: {
                            if(checkRight(currentMino[i], wallSpriteArray[2])){
                            }
                            break;
                        }
                        case left: {
                            if(checkLeft(currentMino[i], wallSpriteArray[1])){
                            }
                            break;
                        }
                        case down: {
                            if(checkDown(currentMino[i], wallSpriteArray[3])){
                                for (auto& m : currentMino) {  // save the sprite location in buffer
                                    m.setPosition(m.getPosition().x, m.getPosition().y);
                                    minoBuffer.push_back(m);
                                }

                                // and set an new sprite from the top
                                int randNum = rand()% (sizeof(minoColorSprites)/sizeof(minoColorSprites[0]));
                                for(int i=0; i<4; i++){
                                    currentMino[i] = minoColorSprites[randNum];
                                }                                
                                // setting the shape and position of the Mino:
                                shape = shapeSeq[ rand() % (sizeof(shapeSeq)/sizeof(shapeSeq[0])) ];
                                try {
                                    std::vector shapeVec = shapesMap.at(shape);
                                    for (int i=0; i<4; i++) {
                                        currentMino[i].setPosition(PIXELS_PER_BLOCK + (PIXELS_PER_BLOCK * (shapeVec[i]%2)), 
                                                                    PIXELS_PER_BLOCK + (PIXELS_PER_BLOCK * (shapeVec[i]/2)));
                                    }
                                }
                                catch (const std::out_of_range&) {
                                    std::cout << "Key \"" << shape << "\" not found" << std::endl;
                                    return -1;
                                }
                            }
                            break;
                        }
                        case up: {
                            break;
                        }
                        default:{
                            dir = down;
                            break;
                        }
                    }
                }
                // // <- Falling Down -> //
                // minoFallDistanceTotal += (fallVelocity * deltaTime.asSeconds()); // 

                 // <- Move -> //
                switch (dir) {
                    case right: {
                        for(int i =0; i<4; i++) {
                            currentMino[i].setPosition(currentMino[i].getPosition().x + PIXELS_PER_BLOCK, 
                                                    currentMino[i].getPosition().y);
                        }
                        break;
                    }
                    case left: {
                        for(int i =0; i<4; i++) {
                            currentMino[i].setPosition(currentMino[i].getPosition().x - PIXELS_PER_BLOCK, 
                                                    currentMino[i].getPosition().y);
                        }
                        break;
                    }
                    case down: {
                        for(int i =0; i<4; i++) {
                            currentMino[i].setPosition(currentMino[i].getPosition().x, 
                                                    currentMino[i].getPosition().y + (fallVelocity * deltaTime.asSeconds()) );
                        }
                        break;
                    }
                    case up: {
                    // Rotate
                        for(int i =0; i<4; i++) {
                            sf::Vector2f p = currentMino[1].getPosition(); // center of rotation
                            int x = currentMino[i].getPosition().y -  p.y;
                            int y = currentMino[i].getPosition().x - p.x;
                            currentMino[i].setPosition(p.x - x, p.y + y);
                        }
                        break;
                    }
                    default:
                        break;
                }
            }


            // // Collision with previous Minos
            // for (auto& currMino : currentMino) {
            //     for(auto& savedMino : minoBuffer){
            //         if (isOn(currMino, savedMino)) {
            //             if(dir == right){
            //             };
            //             if(dir == down || dir == centre){
            //                 // save the sprite location in buffer
            //                 for (auto& m : currentMino) {
            //                     m.setPosition(m.getPosition().x, m.getPosition().y - PIXELS_PER_BLOCK);
            //                     // check losing condition
            //                     if (m.getPosition().y < 3 * PIXELS_PER_BLOCK) {
            //                         continueGameFlag = false;
            //                         break;                           
            //                     }
            //                     minoBuffer.push_back(m);
            //                 }



            


        }

    }
    return 0;
}