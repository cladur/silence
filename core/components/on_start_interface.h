#ifndef SILENCE_ON_START_INTERFACE_H
#define SILENCE_ON_START_INTERFACE_H

class IOnStart {
public:
	virtual ~IOnStart() = default;
	virtual void on_start() = 0;
};

#endif //SILENCE_ON_START_INTERFACE_H
