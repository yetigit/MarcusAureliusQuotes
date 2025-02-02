
#pragma once
#include "Widgets/SWindow.h"

class SMAQWindow: public SWindow
{
public:
	SLATE_BEGIN_ARGS( SMAQWindow )
	{
	}
  SLATE_END_ARGS()


 bool IsRegularWindow() const;
 bool AppearsInTaskbar() const;


};
