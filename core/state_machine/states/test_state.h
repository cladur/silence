#ifndef SILENCE_TEST_STATE_H
#define SILENCE_TEST_STATE_H

#include "state.h"

class TestState : public State {
public:
	TestState(std::string name);
	void enter() override;
	void update(float dt) override;
	void exit() override;
};

#endif //SILENCE_TEST_STATE_H
