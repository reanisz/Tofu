#include "tofu/ball/game.h"
#include "tofu/ball/renderer_registerer.h"

void Main()
{
    Scene::SetBackground(ColorF(0.8, 0.9, 1.0));

	tofu::ball::Game game;
    game.initBaseSystems();

    // クライアントなので描画を有効化
    tofu::ball::RendererRegisterer::Attach(game);

    game.initEnitites();
	game.run();
}

