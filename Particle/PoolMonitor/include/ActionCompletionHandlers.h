#include "application.h"

class ActionCompletionHandlers {
private:
  String _outputState = "unknown";
  String _sourceState = "unknown";
  String _spaModeState = "unknown";

  String _currentAction = "idle";

  void resetOutputPostion();
  void resetSourcePosition();
  void resetSpaModePosition();

public:
  void init();

  String getOutputState();
  String getSourceState();
  String getSpaModeState();

  String getCurrentAction();

  void setPoolAsSource();
  void setSpaAsSource();
  void setBothAsSource();

  void setPoolAsOutput();
  void setSpaAsOutput();
  void setBothAsOutput();

  void setSpaModeNormal();
  void setSpaModeJets();
};
