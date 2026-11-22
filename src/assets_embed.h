#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <cstddef>

// 查找已嵌入 exe 的资源。返回数据指针；outSize 输出字节数。
// 未找到（或 path 为空）返回 nullptr。
const unsigned char* FindEmbeddedAsset(const char* path, size_t* outSize);

// 以下函数优先从内嵌资源加载，失败时回退到文件系统路径。
// 这样开发期（assets 目录存在）和发布期（资源已内嵌）都能正常工作。
SDL_Texture* Asset_LoadTexture(SDL_Renderer* renderer, const char* path);
Mix_Music*   Asset_LoadMusic(const char* path);
Mix_Chunk*   Asset_LoadWAV(const char* path);
TTF_Font*    Asset_LoadFont(const char* path, int size);
