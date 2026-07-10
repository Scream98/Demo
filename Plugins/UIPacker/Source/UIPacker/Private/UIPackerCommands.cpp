#include "UIPackerCommands.h"

#define LOCTEXT_NAMESPACE "FUIPackerModule"

void FUIPackerCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "UIPacker", "Execute UIPacker action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
