#ifndef SILENCE_ON_DESTROY_INTERFACE_H
#define SILENCE_ON_DESTROY_INTERFACE_H

class IOnDestroy {
public:
	virtual ~IOnDestroy() = default;
	virtual void on_destroy() = 0;
};

#endif //SILENCE_ON_DESTROY_INTERFACE_H
