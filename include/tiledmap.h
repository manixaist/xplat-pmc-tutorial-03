#pragma once
#include "SDL_image.h"

// Takes a texture divided evenly into tiles as well as a map size and a list of indices to the tiles
// to fill out the map.  When rendered, the map will center itself in the total window and iterate over
// the map, drawing the indexed tile
class TiledMap
{
public:
    TiledMap(const Uint16 rows, const Uint16 cols) : 
        _pTileRects(nullptr), 
        _pMapIndicies(nullptr),
        _cCols(cols),
        _cRows(rows),
        _tileSize(0),
        _pTileTexture(nullptr),
        _cTilesOnTexture(0)
    {
        SDL_memset(&_textureRect, 0, sizeof(SDL_Rect));
    }

    ~TiledMap()
    {
        delete[] _pMapIndicies;
        delete[] _pTileRects;
    }

    bool Initialize(SDL_Rect textureRect, SDL_Rect tileRect, SDL_Texture *pTexture, Uint16 *pMapIndices, Uint16 countOfIndicies);
    void Render(SDL_Renderer *pSDLRenderer, Uint16 cxScreen, Uint16 cyScreen);

private:
    SDL_Rect* _pTileRects;      // Will hold the list of tile source rects from the texture loaded
    Uint16 *_pMapIndicies;      // Will hold the indices to the map
    Uint16 _cCols;              // Cols in the map
    Uint16 _cRows;              // Rows in the map
    Uint16 _tileSize;           // Cached size of the tile (w == h in our implementation e.g. square tiles only)
    SDL_Rect _textureRect;      // Size of the texture
    SDL_Texture *_pTileTexture; // Texture that holds the tiles (must be evenly divisible by tile size)
    Uint16 _cTilesOnTexture;    // Total number of tiles on the texture
 };