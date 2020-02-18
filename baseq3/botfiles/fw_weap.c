//===========================================================================
//
// Name:			fw_weap.c
// Function:
// Programmer:		Mr Elusive (MrElusive@idsoftware.com)
// Last update:		1999-09-08
// Tab Size:		4 (real tabs)
//===========================================================================



weight "Gauntlet"
{
	switch(INVENTORY_GAUNTLET)
	{
		case 1: return 0;
		default: return W_GAUNTLET;
	} //end switch
} //end weight

weight "Shotgun"
{
	switch(INVENTORY_SHOTGUN)
	{
		case 1: return 0;
		default:
		{
			switch(INVENTORY_SHELLS)
			{
				case 1: return 0;
				default:
				{
					return W_SHOTGUN;
				} //end default
			} //end switch
		} //end default
	} //end switch
} //end weight

weight "Machinegun"
{
	switch(INVENTORY_MACHINEGUN)
	{
		case 1: return 0;
		default:
		{
			switch(INVENTORY_BULLETS)
			{
				case 1: return 0;
				default:
				{
					return W_MACHINEGUN;
				} //end default
			} //end switch
		} //end default
	} //end switch
} //end weight

weight "Grenade Launcher"
{
	switch(INVENTORY_GRENADELAUNCHER)
	{
		case 1: return 0;
		default:
		{
			switch(INVENTORY_GRENADES)
			{
				case 1: return 0;
				default:
				{
					return W_GRENADELAUNCHER;
				} //end default
			} //end switch
		} //end default
	} //end switch
} //end weight

weight "Rocket Launcher"
{
	switch(INVENTORY_ROCKETLAUNCHER)
	{
		case 1: return 0;
		default:
		{
			switch(INVENTORY_ROCKETS)
			{
				case 1: return 0;
				default:
				{
					return W_ROCKETLAUNCHER;
				} //end default
			} //end switch
		} //end default
	} //end switch
} //end weight

weight "Lightning Gun"
{
	switch(INVENTORY_LIGHTNING)
	{
		case 1: return 0;
		default:
		{
			switch(INVENTORY_LIGHTNINGAMMO)
			{
				case 1: return 0;
				default:
				{
					//the lightning gun has a limited range
					switch(ENEMY_HORIZONTAL_DIST)
					{
						case 768: return W_LIGHTNING;
						default: return $evalint(W_LIGHTNING*0.1);
					} //end switch
				} //end default
			} //end switch
		} //end default
	} //end switch
} //end weight

weight "Railgun"
{
	switch(INVENTORY_RAILGUN)
	{
		case 1: return 0;
		default:
		{
			switch(INVENTORY_SLUGS)
			{
				case 1: return 0;
				default:
				{
					return W_RAILGUN;
				} //end default
			} //end switch
		} //end default
	} //end switch
} //end weight

weight "Plasma Gun"
{
	switch(INVENTORY_PLASMAGUN)
	{
		case 1: return 0;
		default:
		{
			switch(INVENTORY_CELLS)
			{
				case 1: return 0;
				default:
				{
					return W_PLASMAGUN;
				} //end default
			} //end switch
		} //end default
	} //end switch
} //end weight

weight "BFG10K"
{
	switch(INVENTORY_BFG10K)
	{
		case 1: return 0;
		default:
		{
			switch(INVENTORY_BFGAMMO)
			{
				case 1: return 0;
				default:
				{
					return W_BFG10K;
				} //end default
			} //end switch
		} //end default
	} //end switch
} //end weight

weight "Grappling Hook"
{
	switch(INVENTORY_GRAPPLINGHOOK)
	{
		case 1: return 0;
		default:
		{
			return W_GRAPPLE;
		} //end default
	} //end switch
} //end weight

