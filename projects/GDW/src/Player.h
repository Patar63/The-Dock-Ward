//class creation for the player entity

#pragma once
#include "NOU/Entity.h"

using namespace nou;

	class Player
	{
	public:
		//Creator functions
		Player();
		Player(float x, float y, float z);

		//destructor
		~Player();

		//update declaration
		void Update(float deltaTime);

		//function for movement animation loading
		void LoadFrames();

	private:

	protected:
	};

