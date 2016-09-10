#include "tiledmap.h"

// The main goals here are to 
// 1) Divide up the texture into src rects
// 2) Copy the index data
// 3) Cache some calculated values we'll reuse rendering
bool TiledMap::Initialize(
    SDL_Rect textureRect,           // Size of the texture
    SDL_Rect tileRect,              // size of the tile - the texture should be a multiple of this size...
    SDL_Texture *pTexture,          // texture holding the tiles
    Uint16 *pMapIndices,            // array of indicies to the tiles, should match in size to map
    Uint16 countOfIndicies)         // again should match, but here to be explicit in the code
{
    // Validate some assumptions
    SDL_assert((textureRect.w % tileRect.w) == 0);
    SDL_assert((textureRect.h % tileRect.h) == 0);
    SDL_assert(countOfIndicies == (_cRows * _cCols));

    // Copy the map indicies data
    _pMapIndicies = new Uint16[countOfIndicies] { };
    SDL_memcpy(_pMapIndicies, pMapIndices, countOfIndicies * sizeof(Uint16));

    // Copy the texture data
    _pTileTexture = pTexture;
    SDL_memcpy(&_textureRect, &textureRect, sizeof(SDL_Rect));

    // Calculate the total available tiles on the texture and allocate space for the source rects
    // this is all just dividing the coordinate space into even squares
    _tileSize = tileRect.w;         //Pick either, we assuming they are the same so far
    Uint16 textureTilesPerWidth  = (_textureRect.w / _tileSize);    // The texture itself does not need to be square
    Uint16 textureTilesPerHeight = (_textureRect.h / _tileSize);
    _cTilesOnTexture = ((_textureRect.w / _tileSize) * textureTilesPerHeight);
    _pTileRects = new SDL_Rect[_cTilesOnTexture] {};
    
    // Loop through the tiles and set the source rects
    for (int r = 0; r < textureTilesPerHeight; r++)
    {
        for (int c = 0; c < textureTilesPerWidth; c++)
        {
            _pTileRects[((r * textureTilesPerHeight) + c)].h = _tileSize;
            _pTileRects[((r * textureTilesPerHeight) + c)].w = _tileSize;
            _pTileRects[((r * textureTilesPerHeight) + c)].x = _tileSize * c;
            _pTileRects[((r * textureTilesPerHeight) + c)].y = _tileSize * r;
        }
    }
    return true;
}

// Loop through the map of indicies and render each tile in order.  Center the map on the screen
void TiledMap::Render(SDL_Renderer *pSDLRenderer, Uint16 cxScreen, Uint16 cyScreen)
{
    SDL_assert(_cRows * _pTileRects[0].w <= cxScreen); // Every tile is the same size in this implementation
    SDL_assert(_cCols * _pTileRects[0].h <= cyScreen);

    // Center the map, so calculate the offsets
    Uint16 cxOffset = (cxScreen - (_cCols * _tileSize)) / 2;
    Uint16 cyOffset = (cyScreen - (_cRows * _tileSize)) / 2;

    SDL_Rect targetRect = {0, 0, _tileSize, _tileSize }; // The size won't change, so we'll update the x, y
    for (int r = 0; r < _cRows; r++)
    {
        for (int c = 0; c < _cCols; c++) 
        {
            targetRect.x = (c * _tileSize) + cxOffset;
            targetRect.y = (r * _tileSize) + cyOffset;
            int currentTileIndex = _pMapIndicies[r * _cCols + c];

            SDL_RenderCopy(
                pSDLRenderer,                   // Our renderer - everything goes here that draws
                _pTileTexture,                  // texture that holds the source tiles
                &_pTileRects[currentTileIndex], // rect in our map indicies list that tells us which tile to draw
                &targetRect);                   // dest rect on the screen for the tile indexed above
        }
    }
}