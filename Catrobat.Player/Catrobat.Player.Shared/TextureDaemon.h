#pragma once

#include "CatrobatTexture.h"
#include "Common\DeviceResources.h"

#include <map>
#include <string>
#include <d2d1.h>

class TextureDaemon
{
public:
	static TextureDaemon *Instance();
    std::unique_ptr<CatrobatTexture> LoadTexture(const std::shared_ptr<DX::DeviceResources>& deviceResources, std::string textureKey);

private:
	TextureDaemon();
    ~TextureDaemon();
    //TextureDaemon(TextureDaemon &const);
    TextureDaemon& operator=(TextureDaemon const&);

	static TextureDaemon *__instance;
};

