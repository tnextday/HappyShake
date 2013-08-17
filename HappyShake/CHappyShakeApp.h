#pragma once

// Include MZFC header file
#include <mzfc_inc.h>

#include "CHappyShakeWnd.h"

// Application class derived from CMzApp
class CHappyShakeApp: public CMzApp
{
public:
  CHappyShakeApp(void);
  ~CHappyShakeApp(void);

  // Application main form
  CHappyShakeMainWnd* m_MainWnd;

  // Initialize application
  virtual BOOL Init();
  virtual int Done();
};

