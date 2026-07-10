#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "UIPackerStyle.h"

class FUIPackerCommands : public TCommands<FUIPackerCommands>
{
public:

	FUIPackerCommands()
		: TCommands<FUIPackerCommands>(TEXT("UIPacker"), NSLOCTEXT("Contexts", "UIPacker", "UIPacker Plugin"), NAME_None, FUIPackerStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
