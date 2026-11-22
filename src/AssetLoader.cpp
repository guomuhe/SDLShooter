#include "assets_embed.h"

// 资源加载器：优先读取内嵌进 exe 的资源，找不到再回退到文件系统。
// freesrc=1 让 SDL 在加载完成后释放 SDL_RWops，但不会释放静态数据本身，安全。

SDL_Texture* Asset_LoadTexture(SDL_Renderer* renderer, const char* path)
{
    size_t size = 0;
    const unsigned char* data = FindEmbeddedAsset(path, &size);
    if (data != nullptr) {
        SDL_RWops* rw = SDL_RWFromConstMem(data, static_cast<int>(size));
        if (rw != nullptr) {
            return IMG_LoadTexture_RW(renderer, rw, 1);
        }
    }
    return IMG_LoadTexture(renderer, path);
}

Mix_Music* Asset_LoadMusic(const char* path)
{
    size_t size = 0;
    const unsigned char* data = FindEmbeddedAsset(path, &size);
    if (data != nullptr) {
        SDL_RWops* rw = SDL_RWFromConstMem(data, static_cast<int>(size));
        if (rw != nullptr) {
            return Mix_LoadMUS_RW(rw, 1);
        }
    }
    return Mix_LoadMUS(path);
}

Mix_Chunk* Asset_LoadWAV(const char* path)
{
    size_t size = 0;
    const unsigned char* data = FindEmbeddedAsset(path, &size);
    if (data != nullptr) {
        SDL_RWops* rw = SDL_RWFromConstMem(data, static_cast<int>(size));
        if (rw != nullptr) {
            return Mix_LoadWAV_RW(rw, 1);
        }
    }
    return Mix_LoadWAV(path);
}

TTF_Font* Asset_LoadFont(const char* path, int size)
{
    size_t sizeBytes = 0;
    const unsigned char* data = FindEmbeddedAsset(path, &sizeBytes);
    if (data != nullptr) {
        SDL_RWops* rw = SDL_RWFromConstMem(data, static_cast<int>(sizeBytes));
        if (rw != nullptr) {
            return TTF_OpenFontRW(rw, 1, size);
        }
    }
    return TTF_OpenFont(path, size);
}
