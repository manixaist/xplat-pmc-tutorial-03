// main.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include "SDL.h"
#include "include/tiledmap.h"
#include "include/constants.h"
#include "include/utils.h"
#include "include/sprite.h"

using namespace XplatGameTutorial::PacManClone;

// Cleanup objects create in Initialize and shutdown SDL and related subsystems
void Cleanup(SDL_Window **ppSDLWindow, SDL_Renderer **ppSDLRenderer)
{
    SDL_DestroyRenderer(*ppSDLRenderer);
    *ppSDLRenderer = nullptr;

    SDL_DestroyWindow(*ppSDLWindow);
    *ppSDLWindow = nullptr;

    IMG_Quit();
    SDL_Quit();
}

// Simple enum to denote the 4 possible directions
// The sprites can move
enum class Direction
{
    Up = 0,
    Down,
    Left,
    Right
};

// Given a player's current state (location, direction, animation) check if the player can move in a given direction, and if
// so position the player on the new track at the new velocity
void DoPlayerInputCheck(Sprite *pSprite, TiledMap *pTiledMap, Direction direction, Uint16 row, Uint16 col, Uint16 animationIndex, double dx, double dy)
{
    // Helper lambda to check the map in a given direction.  I put it here instead of another helper
    // because it's only useful here now.  Plus I wanted to check the c++11 feature on both compilers :)
    auto CanMove = [](Direction direction, Uint16 row, Uint16 col) -> SDL_bool
    {
        SDL_bool fResult = SDL_FALSE;
        // Adjust the [row][col] to look at based on direction
        if (direction == Direction::Up)
        {
            row--;
        }
        else if (direction == Direction::Down)
        {
            row++;
        }
        else if (direction == Direction::Left)
        {
            col--;
        }
        else if (direction == Direction::Right)
        {
            col++;
        }

        // Check the map, 0s are legal free space
        if (Constants::CollisionMap[row * Constants::MapCols + col] == 0)
        {
            fResult = SDL_TRUE;
        }
        return fResult;
    };

    // If we can move and we're not already moving in the direction
    if ((CanMove(direction, row, col) == SDL_TRUE) &&
        (pSprite->CurrentAnimation() != animationIndex))
    {
        // Set a new animation and position the player with a new velocity
        pSprite->SetAnimation(animationIndex);
        SDL_Point tilePoint = pTiledMap->GetTileCoordinates(row, col);
        pSprite->ResetPosition(tilePoint.x, tilePoint.y);
        pSprite->SetVelocity(dx, dy);
    }
}

// Even if no input is pressed, the player may run into a wall, so we need to handle collisions
// after the player is moved
void DoPlayerBoundsCheck(Sprite *pSprite, TiledMap *pTiledMap)
{
    SDL_Point playerPoint = { static_cast<int>(pSprite->X()), static_cast<int>(pSprite->Y()) };

    // Need to check bounds in direction moving (account for width of half the sprite)
    // This is because the sprite is double the size of the tiles and placed along the centerline
    // in the direction of movement.  So 1/2 of its size in a given direction is the "edge" of the
    // sprite on the screen (minus a pixel or 2 of transparency)
    if (pSprite->DX() != 0) // If we're not moving in this axis, then don't bother
    {
        if (pSprite->DX() < 0)
        {
            playerPoint.x -= (Constants::PlayerSpriteWidth / 2) - Constants::TileWidth / 2;
        }
        else
        {
            playerPoint.x += (Constants::PlayerSpriteWidth / 2) - Constants::TileWidth / 2;
        }
    }
    else  // We cann't be moving in both directions at once
    {
        if (pSprite->DY() < 0)  // Same logic for y axis if moving
        {
            playerPoint.y -= (Constants::PlayerSpriteHeight / 2) - Constants::TileHeight / 2;
        }
        else
        {
            playerPoint.y += (Constants::PlayerSpriteHeight / 2) - Constants::TileHeight / 2;
        }
    }

    // Now get the row, col we're in
    Uint16 row = 0;
    Uint16 col = 0;
    pTiledMap->GetTileRowCol(playerPoint, row, col);

    // If we wandered into a bad cell, stop
    if (Constants::CollisionMap[row * Constants::MapCols + col] == 1)
    {
        pSprite->SetVelocity(0, 0);
    }
}

// Handle any keyboard input.  The basic logic here is
// 1)  If a directional key is pressed
// 2)  Check the cell adjacent based on direction
// 3)  If the new direction is open, place the sprite along the centerline and
//     set its new velocity
// 4)  If ESC is hit, signal quit
//
// pInputSprite is the temporary graphical helper which will go away - it shows directions pressed
bool ProcessInput(Sprite *pSprite, Sprite* pInputSprite, TiledMap *pTiledMap)
{
    bool fResult = false;

    // All it takes to get the key states.  The array is valid within SDL while running
    const Uint8 *pCurrentKeyState = SDL_GetKeyboardState(nullptr);

    // Get the player's info before any input is taken
    SDL_Point playerPreInputPoint = { static_cast<int>(pSprite->X()), static_cast<int>(pSprite->Y()) };
    Uint16 playerPreInputRow = 0;
    Uint16 playerPreInputCol = 0;
    pTiledMap->GetTileRowCol(playerPreInputPoint, playerPreInputRow, playerPreInputCol);

    // LOGIC
    // Check if a direction key is down (or WASD) and then process it with our helper
    // which will handle collision, etc
    if (pCurrentKeyState[SDL_SCANCODE_UP] || pCurrentKeyState[SDL_SCANCODE_W])
    {
        pInputSprite->SetFrame(0);
        pInputSprite->SetVisible(SDL_TRUE);
        DoPlayerInputCheck(pSprite, pTiledMap, Direction::Up, playerPreInputRow, playerPreInputCol, Constants::AnimationIndexUp, 0, -1.5);
    }
    else if (pCurrentKeyState[SDL_SCANCODE_DOWN] || pCurrentKeyState[SDL_SCANCODE_S])
    {
        pInputSprite->SetFrame(1);
        pInputSprite->SetVisible(SDL_TRUE);
        DoPlayerInputCheck(pSprite, pTiledMap, Direction::Down, playerPreInputRow, playerPreInputCol, Constants::AnimationIndexDown, 0, 1.5);
    }
    else if (pCurrentKeyState[SDL_SCANCODE_LEFT] || pCurrentKeyState[SDL_SCANCODE_A])
    {
        pInputSprite->SetFrame(2);
        pInputSprite->SetVisible(SDL_TRUE);
        DoPlayerInputCheck(pSprite, pTiledMap, Direction::Left, playerPreInputRow, playerPreInputCol, Constants::AnimationIndexLeft, -1.5, 0);
    }
    else if (pCurrentKeyState[SDL_SCANCODE_RIGHT] || pCurrentKeyState[SDL_SCANCODE_D])
    {
        pInputSprite->SetFrame(3);
        pInputSprite->SetVisible(SDL_TRUE);
        DoPlayerInputCheck(pSprite, pTiledMap, Direction::Right, playerPreInputRow, playerPreInputCol, Constants::AnimationIndexRight, 1.5, 0);
    }
    else if (pCurrentKeyState[SDL_SCANCODE_X])
    {
        pSprite->SetAnimation(Constants::AnimationIndexDeath);
    }
    else if (pCurrentKeyState[SDL_SCANCODE_ESCAPE])
    {
        printf("ESC hit - exiting main loop...");
        fResult = true;
    }
    else
    {
        pInputSprite->SetVisible(SDL_FALSE);
    }
    return fResult;
}

// Helper to break out the sprite init code from main()
void InitializeSprites(TiledMap* pTiledMap, TextureWrapper* pSpriteTexture, Sprite **ppPlayerSprite, Sprite **ppInputSprite)
{
    *ppPlayerSprite = nullptr;
    *ppInputSprite = nullptr;

    // Declare and initialize sprite object(s)
    Sprite* pSprite = new Sprite(pSpriteTexture, Constants::PlayerSpriteWidth, Constants::PlayerSpriteHeight,
        Constants::PlayerTotalFrameCount, Constants::PlayerTotalAnimationCount);

    pSprite->LoadFrames(0, 0, 0, 10);
    pSprite->LoadFrames(10, 0, Constants::PlayerSpriteHeight, 10);
    pSprite->LoadAnimationSequence(Constants::AnimationIndexLeft, AnimationType::Loop, Constants::PlayerAnimation_LEFT, Constants::PlayerAnimationFrameCount, Constants::PlayerAnimationSpeed);
    pSprite->LoadAnimationSequence(Constants::AnimationIndexRight, AnimationType::Loop, Constants::PlayerAnimation_RIGHT, Constants::PlayerAnimationFrameCount, Constants::PlayerAnimationSpeed);
    pSprite->LoadAnimationSequence(Constants::AnimationIndexUp, AnimationType::Loop, Constants::PlayerAnimation_UP, Constants::PlayerAnimationFrameCount, Constants::PlayerAnimationSpeed);
    pSprite->LoadAnimationSequence(Constants::AnimationIndexDown, AnimationType::Loop, Constants::PlayerAnimation_DOWN, Constants::PlayerAnimationFrameCount, Constants::PlayerAnimationSpeed);
    pSprite->LoadAnimationSequence(Constants::AnimationIndexDeath, AnimationType::Once, Constants::PlayerAnimation_DEATH, Constants::PlayerAnimationDeathFrameCount, Constants::PlayerAnimationSpeed);
    pSprite->SetVelocity(1.5, 0);
    pSprite->SetAnimation(Constants::AnimationIndexRight);
    pSprite->SetFrameOffset(1 - (Constants::PlayerSpriteWidth / 2), 1 - (Constants::PlayerSpriteHeight / 2));

    SDL_Point playerStartCoord = pTiledMap->GetTileCoordinates(Constants::PlayerStartRow, Constants::PlayerStartCol);
    pSprite->ResetPosition(playerStartCoord.x, playerStartCoord.y);

    // Visual for detected input
    Sprite *pInputSprite = new Sprite(pSpriteTexture, Constants::PlayerSpriteWidth, Constants::PlayerSpriteHeight, 4, 4);
    pInputSprite->LoadFrames(0, 0, 64, 4);
    pInputSprite->SetVisible(SDL_FALSE);

    *ppPlayerSprite = pSprite;
    *ppInputSprite = pInputSprite;
}

int main(int argc, char* /*argv*/[])
{
    SDL_Renderer *pSDLRenderer = nullptr;
    SDL_Window *pSDLWindow     = nullptr;
    
    // Lots of things are controlled by XplatGameTutorial::PacManClone::Constants, 
    // e.g. screen dimensions, title, etc
    if (InitializeSDL(&pSDLWindow, &pSDLRenderer))
    { 
        {   // We'd like the enclosed objects to go out of scope before Cleanup is called

            // Load our textures
            SDL_Color colorKey = Constants::SDLColorMagenta;
            TextureWrapper tilesTexture("./grfx/tiles.png", SDL_strlen("./grfx/tiles.png"), pSDLRenderer, nullptr);
            TextureWrapper spriteTexture("./grfx/spritesheet.png", SDL_strlen("./grfx/spritesheet.png"), pSDLRenderer, &colorKey);
            
            if (tilesTexture.IsNull() || spriteTexture.IsNull())
            {
                printf("Failed to load one or more textures\n");
            }
            else
            {
                // This should be know, but it should also match what we just queried
                SDL_assert(tilesTexture.Width() == Constants::TileTextureWidth);
                SDL_assert(tilesTexture.Height() == Constants::TileTextureHeight);
                SDL_Rect textureRect{ 0, 0, Constants::TileTextureWidth, Constants::TileTextureHeight };

                // Initialize our tiled map object
                TiledMap tiledMap(Constants::MapRows, Constants::MapCols, Constants::ScreenWidth, Constants::ScreenHeight);

                tiledMap.Initialize(textureRect, { 0, 0,  Constants::TileWidth,  Constants::TileHeight }, tilesTexture.Ptr(),
                    Constants::MapIndicies, Constants::MapRows *  Constants::MapCols);

                // Initialize our sprites
                Sprite* pSprite = nullptr;
                Sprite* pInputSprite = nullptr;
                InitializeSprites(&tiledMap, &spriteTexture, &pSprite, &pInputSprite);

                // GAME LOOP -----
                bool fQuit = false;
                SDL_Event eventSDL;

                Uint32 startTicks;
                while (!fQuit)
                {
                    startTicks = SDL_GetTicks();
                    while (SDL_PollEvent(&eventSDL) != 0)
                    {
                        if (eventSDL.type == SDL_QUIT)
                        {
                            fQuit = true;
                        }
                    }

                    if (!fQuit)
                    {
                        // INPUT
                        fQuit = ProcessInput(pSprite, pInputSprite, &tiledMap);
                        if (!fQuit)
                        {
                            // UPDATE
                            pSprite->Update();

                            // BOUNDS CHECK
                            // We still need to check if the player has wandered into a wall
                            DoPlayerBoundsCheck(pSprite, &tiledMap);

                            // RENDERING
                            SDL_RenderClear(pSDLRenderer);
                            tiledMap.Render(pSDLRenderer);
                            pSprite->Render(pSDLRenderer);
                            pInputSprite->Render(pSDLRenderer);
                            SDL_RenderPresent(pSDLRenderer);

                            // TIMING
                            // Fix this at ~c_framesPerSecond
                            Uint32 endTicks = SDL_GetTicks();
                            Uint32 elapsedTicks = endTicks - startTicks;
                            if (elapsedTicks < Constants::TicksPerFrame)
                            {
                                SDL_Delay(Constants::TicksPerFrame - elapsedTicks);
                            }
                        }
                    }
                }
            }
        }

        // cleanup
        Cleanup(&pSDLWindow, &pSDLRenderer);
    }
    return 0;
}
