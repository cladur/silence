//
// Created by 7hube on 27.03.2023.
//

#ifndef SILENCE_TEST_STATE_H
#define SILENCE_TEST_STATE_H

#include "../base_state.h"

class TestState : public BaseState {
public:
	TestState(std::string name);
	void enter() override;
	void update() override;
	void exit() override;
};

#endif //SILENCE_TEST_STATE_H
