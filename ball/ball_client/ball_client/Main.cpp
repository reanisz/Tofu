#include <Siv3D.hpp>
#include <cassert>
#include <optional>

#include <entt\entt.hpp>
#include <box2d/box2d.h>

#include "Tofu.Utils.h"
#include "Tofu.Ecs.Core.h"
#include "Tofu.Ecs.Physics.h"
#include "Tofu.Ecs.Box2DPrimitiveRenderer.h"
#include "Tofu.Ball.Game.h"

void Main()
{
	Scene::SetBackground(ColorF(0.8, 0.9, 1.0));

	tofu::ball::Game game;
	game.run();
}

