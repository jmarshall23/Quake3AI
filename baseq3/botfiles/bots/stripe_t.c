
//===========================================================================
//
// Name:			Stripe
// Function:		chat lines for Stripe
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:			The Seven Swords & R.A. Salvatore
//Editor:			Paul Jaquays
// Last update:		October 13, 1999
// Tab Size:		3 (real tabs)
//===========================================================================

chat "stripe"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"Y'all got your boots on. Let's get busy.";
		"I lived in worse places than, ", 4, ".";
		"All the comforts of home and good company too.";
		"Y'all don't want to get on my bad side today.";
		HELLO6;
		HELLO3;
		HELLO11;
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"All y'all just got lucky, ", fighter, "s. Duty calls.";
		"No ~bugs to squash in ", 4, ".";
		GOODBYE5;
		// 0 = bot name
		// 4 = Level's title
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"Show me what y'll got, ~bug-lover.";
		"Come on junior, let's see what y'all got.";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"I feel like I just kissed my sista'.";
		"I might as well start raising pigeons.";
		LEVEL_END1;
		"You're excused, ", 3, ". But ", 2," , I'm gonna pay you back good.";
		// 0 = bot name
		// 2 = opponent in first place
		// 3 = opponent in last place
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"What else would ya'll expect from an elite killing machine?";
		"Move like a dragonfly, hit like a tank, I looked pretty, but y'all don't even rank.";
		"I expect you to salute your superiors, ", 3, ".";
		LEVEL_END_VICTORY4;
		LEVEL_END_VICTORY3;
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"I'm a disgrace to the Legion.";
		"Where were y'all when I was on Beta-3?";
		"Go hunt some ~bugs, ", 2, ".";
		LEVEL_END_LOSE0;
		// 0 = bot name
		// 2 = opponent in first place
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"Y'all wanna street fight punk? YOU GOT IT!"; 
		"All's fair in love and war, right?  Y'all are so WRONG, ", fighter, "!!";
		"Y'all don't go messing with ~Stripe!";  
		//0 = shooteron
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"Either I'm getting slow or y'all're using a sloppy weapon.";
		0, ", y'all are beginning to irritate me.";
		"I must have a tracer on me.";
		"Don't play games with me, ", 0, ".";
		"Y'all hit the man, he supposed to stay down. Y'all messed up.";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		TAUNT5;
		"I wish I had my .44.";
		"Look at ", 0, " go!  bet that ", fighter, " could outrun police dogs!";
		"Last time I shot something that ran as fast as y'all, I cooked it for dinner.";
		"Be nice if they give us some live ammo.";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Get out of my head!";
		"What is this?  A friggin' turf war?";
		DEATH_TELEFRAGGED1;
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"I wish I was wearing my game shoes.";
		"Double-pump, 360 slam!";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"And I thought Louisiana Hot Sauce was hot.";
		"Man! Almost as hot as a Texas summer.";
		DEATH_LAVA1;
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"AAAAAAAAH! ~Bug dip.";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"They didn't cover this in basic.";
		"Well that's just great. Who broke the hydrant?";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"That's not right.";
		DEATH_SUICIDE1;
		"I never was a good rebounder.";
		"Y'all are definitely outta bounds there, ", 0, ".";
		DEATH_SUICIDE4;
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"I might as well put on a dress and go back to the pen.";
		"Lean on the car and spread 'em, right?";
		"Do that again and I'll cut you, man!";
		DEATH_GAUNTLET1;
		DEATH_GAUNTLET2;
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"I didn't know I was within ~five feet of the fence.";
		"That gun don't mess around.";
		"Y'all put that thing down, ", 0, ". Don't go messing with what you can't handle.";
		DEATH_RAIL1;
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG	
	{
		"Now I know what a piece of toast feels like.";
		"Y'all don't got the training to use that piece of heat.";
		DEATH_BFG0;
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"Oh, so you want to play rough?";
		"You piece of crap! Are you wearing a rabbit's foot?";
		DEATH_INSULT2;
		DEATH_INSULT4;
		DEATH_INSULT6;
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"Consider yourself lucky, ", 0, ". You just offed a bonafide war ~hero.";
		"Just get my name right on the tombstone, ", fighter, ".";
		D_PRAISE2;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"Y'all give a new meaning to reach out and touch someone, ", 0, ".";
		KILL_RAIL0;
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"Gonna have me a weiner roast; no need to build a fire ...";
		"Y'all are ~one messed up ", fighter, " now, ", 0, ".";
		KILL_GAUNTLET2;
		KILL_GAUNTLET1;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Get off my turf!";
		"There's ~one monkey off my back.";
		"Y'all can't nap in the stall like that, ", 0, ".";
		"I figured y'all forgot to flush, ", 0, ". So I done it for y'all.";
		TELEFRAGGED1;
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		"Taking the easy way out, eh?";
		"Step TO the edge, brainchild. Not OVER it!";
		"First time with live ammo, ", fighter, "?";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"The ~bugs in the kitchen gave me more of a fight than y'all did, ", 0, ".";
		"Let's hope you never go to prison.";
		"Yo, ", 0, "! You take that target off when y'all sleep?";
		KILL_INSULT6;
		KILL_INSULT31;
		KILL_INSULT38;
		KILL_EXTREME_INSULT;
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"Y'all Consider yourself honored. I'm a trained professional.";
		"Y'all were a worthy opponent, for an amateur.";
		PRAISE3;
		PRAISE4;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"Are y'all gonna fire that thing or just put us on a fashion show?";
		"Does your momma know where y'all are, ", 1, "?";
		TAUNT4;
		"Does ", 4, " feel like home to y'all? Good, cuz they'll bury y'all here.";
		"Y'all needs to practice more with live ammo.";
		"Firing them video guns in the arcade don't count as field training.";
		TAUNT7;
		// 0 = name of randomly chosen player
		// 1 = bot name
		// 4 = Level's title
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"I can't believe I still have my safety on.";
		"I could go for a big plate of ", food, ", the way my momma made it.";
		MISC6;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end Stripe chat


