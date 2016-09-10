// main.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include "SDL.h"
#include "tiledmap.h"

// Initialize SDL and related subsystems
bool Initialize(
    const Uint16 cxScreen,              // Width of screen in pixels
    const Uint16 cyScreen,              // Height of screen in pixels
    const char * szWindowTitle,         // Window title
    const SDL_Color renderDrawColor,    // Primarily used to clear the renderer
    SDL_Window **ppSDLWindow,           // Returned window container
    SDL_Renderer **ppSDLRenderer)       // Returned renderer container
{
    bool fResult = true;
    *ppSDLWindow    = nullptr;
    *ppSDLRenderer  = nullptr;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) // SDL_INIT_EVERYTHING works too, but we only need video...init what you need
    {
        printf("SDL_Init() failed, error = %s\n", SDL_GetError());
        fResult = false;
    }
    else
    {
        // Creates the Window for the GUI
        *ppSDLWindow = SDL_CreateWindow(szWindowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, cxScreen, cyScreen, SDL_WINDOW_SHOWN);
        if (*ppSDLWindow == nullptr)
        {
            printf("SDL_CreateWindow() failed, error = %s\n", SDL_GetError());
            fResult = false;
        }
        else
        {
            // We now need a renderer to make use of textures, so create one based on the window and we'll use this to update what
            // the user sees rather than drawing to the SDL_Surface like last time
            *ppSDLRenderer = SDL_CreateRenderer(*ppSDLWindow, -1, SDL_RENDERER_ACCELERATED);
            if (*ppSDLRenderer == nullptr)
            {
                printf("SDL_CreateRender() failed, error = %s\n", SDL_GetError());
                fResult = false;
            }
            else
            {
                // Basically sets the color that will fill the screen when cleared
                if (SDL_SetRenderDrawColor(*ppSDLRenderer, renderDrawColor.r, renderDrawColor.g, renderDrawColor.b, renderDrawColor.a) < 0)
                {
                    printf("SDL_SetRenderDrawColor() failed, error = %s\n", SDL_GetError());
                    fResult = false;
                }
                else
                {
                    // This bit will allow us to load PNG files, which I am storing all my images assets as
                    const int cFlagsNeeded = IMG_INIT_PNG | IMG_INIT_JPG;
                    int iFlagsInitted = IMG_Init(cFlagsNeeded);
                    if ((iFlagsInitted & (cFlagsNeeded)) != (cFlagsNeeded))
                    {
                        printf("IMG_Init() failed, error = %s\n", IMG_GetError());
                        fResult = false;
                    }
                }
            }
        }
    }
    return fResult;
}

// Cleanup objects create in Initialize and shutdown SDL and related subsystems
void Cleanup(SDL_Window **ppSDLWindow, SDL_Renderer **ppSDLRenderer, SDL_Texture **ppSDLTexture)
{
    SDL_DestroyTexture(*ppSDLTexture);
    *ppSDLTexture = nullptr;

    SDL_DestroyRenderer(*ppSDLRenderer);
    *ppSDLRenderer = nullptr;

    SDL_DestroyWindow(*ppSDLWindow);
    *ppSDLWindow = nullptr;

    IMG_Quit();
    SDL_Quit();
}

SDL_Texture* LoadTexture(const char *szFileName, SDL_Renderer *pSDLRenderer)
{
    SDL_Texture* pTextureOut = nullptr;
    SDL_Surface* pSDLSurface = IMG_Load(szFileName);
    if (pSDLSurface == nullptr)
    {
        printf("IMG_Load() failed, error = %s\n", IMG_GetError());
    }
    else
    {
        pTextureOut = SDL_CreateTextureFromSurface(pSDLRenderer, pSDLSurface);
        SDL_FreeSurface(pSDLSurface);
    }
    return pTextureOut;
}

int main(int argc, char* argv[])
{
    SDL_Renderer *pSDLRenderer              = nullptr;
    SDL_Window *pSDLWindow                  = nullptr;
    static const Uint16 c_cxScreen          = 800;
    static const Uint16 c_cyScreen          = 600;
    static const Uint32 c_framesPerSecond   = 60;                                   // 1000 ms      1 s
    static const Uint32 c_msPerSecond       = 1000;                                 // ------- x ---------
    static const Uint32 c_msPerFrame        = (c_msPerSecond / c_framesPerSecond);  //   1 s     60 frames
    static const Uint32 c_ticksPerFrame     = c_msPerFrame;                         // Ticks and ms are the same thing in SDL
    static const SDL_Color sc_sdlColorGrey  = { 128, 128, 128,255 };                // 
    static const char * const sc_szTitle    = "Tiled Map";
    static const Uint16 c_cMapRows          = 36;
    static const Uint16 c_cMapCols          = 28;
    static const Uint16 c_textureWidth      = 192;
    static const Uint16 c_textureHeight     = 192;
    static const Uint16 c_tileWidth         = 16;
    static const Uint16 c_tileHeight        = 16;

    // Init
    if (Initialize(c_cxScreen, c_cyScreen, sc_szTitle, sc_sdlColorGrey, &pSDLWindow, &pSDLRenderer))
    { 
        // Load our texture that holds all of the map tiles
        SDL_Texture* pTilesTexture =  LoadTexture("./grfx/tiles.png", pSDLRenderer);
        if (pTilesTexture == nullptr)
        {
            printf("LoadTexture() failed, error should be above\n");
        }
        else
        {
            int cxTexture;
            int cyTexture;
            if (SDL_QueryTexture(pTilesTexture, nullptr, nullptr, &cxTexture, &cyTexture) != 0)
            {
                printf("SDL_QueryTexture() failed, error = %s\n", SDL_GetError());
            }
            else
            {
                // This should be know, but it should also match what we just queried
                SDL_assert(cxTexture == c_textureWidth);
                SDL_assert(cyTexture == c_textureHeight);
                SDL_Rect textureRect{ 0, 0, c_textureWidth, c_textureHeight };

                // Init tiled map object
                TiledMap tiledMap(c_cMapRows, c_cMapCols);

                // Indices to tiles that make up the map - for your own sanity use a level editor (several free ones exist) or better
                // yet develop your own tool early in the design process
                //  We just have this one level we'll reuse, so just and paste as long as you don't change the order of the tiles.png
                static Uint16 sc_mapIndicies[c_cMapRows * c_cMapCols] = 
                {
                     49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
                     49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
                     49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
                      6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7, 40, 39,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  8,
                     18, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 15, 17, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 20,
                     18, 16,  0,  1,  1,  2, 16,  0,  1,  1,  1,  2, 16, 15, 17, 16,  0,  1,  1,  1,  2, 16,  0,  1,  1,  2, 16, 20,
                     18, 13, 12, 49, 49, 14, 16, 12, 49, 49, 49, 14, 16, 15, 17, 16, 12, 49, 49, 49, 14, 16, 12, 49, 49, 14, 13, 20,
                     18, 16, 24, 25, 25, 26, 16, 24, 25, 25, 25, 26, 16, 27, 29, 16, 24, 25, 25, 25, 26, 16, 24, 25, 25, 26, 16, 20,
                     18, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 20,
                     18, 16,  0,  1,  1,  2, 16,  0,  2, 16,  0,  1,  1,  1,  1,  1,  1,  2, 16,  0,  2, 16,  0,  1,  1,  2, 16, 20,
                     18, 16, 24, 25, 25, 26, 16, 12, 14, 16, 24, 25, 25,  5,  3, 25, 25, 26, 16, 12, 14, 16, 24, 25, 25, 26, 16, 20,
                     18, 16, 16, 16, 16, 16, 16, 12, 14, 16, 16, 16, 16, 12, 14, 16, 16, 16, 16, 12, 14, 16, 16, 16, 16, 16, 16, 20,
                     30, 31, 31, 31, 31, 11, 16, 12, 27,  1,  1,  2, 49, 12, 14, 49,  0,  1,  1, 29, 14, 16,  9, 31, 31, 31, 31, 32,
                     49, 49, 49, 49, 49, 23, 16, 12,  3, 25, 25, 26, 49, 24, 26, 49, 24, 25, 25,  5, 14, 16, 21, 49, 49, 49, 49, 49,
                     49, 49, 49, 49, 49, 23, 16, 12, 14, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 12, 14, 16, 21, 49, 49, 49, 49, 49,
                     49, 49, 49, 49, 49, 23, 16, 12, 14, 49, 36, 37, 22, 47, 47, 19, 37, 38, 49, 12, 14, 16, 21, 49, 49, 49, 49, 49,
                     34, 34, 34, 34, 34, 35, 16, 24, 26, 49, 48, 49, 49, 49, 49, 49, 49, 50, 49, 24, 26, 16, 33, 34, 34, 34, 34, 34,
                     49, 49, 49, 49, 49, 49, 16, 49, 49, 49, 48, 49, 49, 49, 49, 49, 49, 50, 49, 49, 49, 16, 49, 49, 49, 49, 49, 49,
                     10, 10, 10, 10, 10, 11, 16,  0,  2, 49, 48, 49, 49, 49, 49, 49, 49, 50, 49,  0,  2, 16,  9, 10, 10, 10, 10, 10,
                     49, 49, 49, 49, 49, 23, 16, 12, 14, 49, 60, 61, 61, 61, 61, 61, 61, 62, 49, 12, 14, 16, 21, 49, 49, 49, 49, 49,
                     49, 49, 49, 49, 49, 23, 16, 12, 14, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 12, 14, 16, 21, 49, 49, 49, 49, 49,
                     49, 49, 49, 49, 49, 23, 16, 12, 14, 49,  0,  1,  1,  1,  1,  1,  1,  2, 49, 12, 14, 16, 21, 49, 49, 49, 49, 49,
                      6, 34, 34, 34, 34, 35, 16, 24, 26, 49, 24, 25, 25,  5,  3, 25, 25, 26, 49, 24, 26, 16, 33,  7,  7,  7,  7,  8,
                     18, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 12, 14, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 20,
                     18, 16,  0,  1,  1,  2, 16,  0,  1,  1,  1,  2, 16, 12, 14, 16,  0,  1,  1,  1,  2, 16,  0,  1,  1,  2, 16, 20,
                     18, 16, 24, 25,  5, 14, 16, 24, 25, 25, 25, 26, 16, 12, 14, 16, 24, 25, 25, 25, 26, 16, 12,  3, 25, 26, 16, 20,
                     18, 13, 16, 16, 12, 14, 16, 16, 16, 16, 16, 16, 16, 24, 26, 16, 16, 16, 16, 16, 16, 16, 12, 14, 16, 16, 13, 20,
                     53, 25,  5, 16, 12, 14, 16,  0,  2, 16,  0,  1,  1,  1,  1,  1,  1,  2, 16,  0,  2, 16, 12, 14, 16,  3,  4, 54,
                     41, 28, 29, 16, 24, 26, 16, 12, 14, 16, 24, 25, 25,  5,  3, 25, 25, 26, 16, 12, 14, 16, 24, 26, 16, 27, 28, 42,
                     18, 16, 16, 16, 16, 16, 16, 12, 14, 16, 16, 16, 16, 12, 14, 16, 16, 16, 16, 12, 14, 16, 16, 16, 16, 16, 16, 20,
                     18, 16,  0,  1,  1,  1,  1, 29, 27,  1,  1,  2, 16, 12, 14, 16, 0,   1,  1, 29, 27,  1,  1,  1,  1,  2, 16, 20,
                     18, 16, 24, 25, 25, 25, 25, 25, 25, 25, 25, 26, 16, 24, 26, 16, 24, 25, 25, 25, 25, 25, 25, 25, 25, 26, 16, 20,
                     18, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 20,
                     30, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 32,
                     49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
                     49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49
                };
                
                tiledMap.Initialize(textureRect, { 0, 0, c_tileWidth, c_tileHeight }, pTilesTexture, sc_mapIndicies, c_cMapRows * c_cMapCols);

                // Game loop
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
                        SDL_RenderClear(pSDLRenderer);

                        tiledMap.Render(pSDLRenderer, c_cxScreen, c_cyScreen);
                        
                        SDL_RenderPresent(pSDLRenderer);

                        // Fix this at ~c_framesPerSecond
                        Uint32 endTicks = SDL_GetTicks();
                        Uint32 elapsedTicks = endTicks - startTicks;
                        if (elapsedTicks < c_ticksPerFrame)
                        {
                            SDL_Delay(c_ticksPerFrame - elapsedTicks);
                        }
                    }
                }
            }
        }

        // cleanup
        Cleanup(&pSDLWindow, &pSDLRenderer, &pTilesTexture);
    }
    return 0;
}
