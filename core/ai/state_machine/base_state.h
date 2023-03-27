//
// Created by 7hube on 27.03.2023.
//

#ifndef SILENCE_BASE_STATE_H
#define SILENCE_BASE_STATE_H

#include <string>
#include <utility>

class BaseState {
private:
	std::string name;
public:
	BaseState(std::string name) : name(std::move(name)) {}
	virtual void enter() = 0;
	virtual void update() = 0;
	virtual void exit() = 0;
	virtual std::string get_name() { return name; }
};

#endif //SILENCE_BASE_STATE_H
