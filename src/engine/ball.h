
#ifndef BALL_BALL_H
#define BALL_BALL_H
#include "kernel.h"

class IBall : public IInterface
{
	MACRO_INTERFACE("ball", 0)
public:

	virtual ~IBall() {}
	virtual void Init() = 0;
	virtual void UpdateAndRender() = 0;
	//virtual bool HasUnsavedData() = 0;
	//virtual int Load(const char *pFilename, int StorageType) = 0;
	//virtual int Save(const char *pFilename) = 0;
};

extern IBall *CreateBall();
#endif


