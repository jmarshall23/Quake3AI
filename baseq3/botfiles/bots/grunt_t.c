//===========================================================================
//
// Name:			grunt_c.c
// Function:		chat lines for Grunt
// Programmer:		Mrelusive
// Author:			Paul Jaquays
// Last update:		Oct. 8, 1999
// Tab Size:		3 (real tabs)
//===========================================================================

chat "grunt"
{
	#include "teamplay.h"
	//
	type "game_enter"
	{
		"Anyone standing in my way is gonna be dogmeat.";
		"A ~Marine laughs at danger. And I see nothing here to laugh about.";
		"I can see that the words 'no contest' mean something here.";
		"I am jacked in and good to go.";
		"You ", fighter, "s are stinkin' the place up.";
		"I see ", 1, " is stinkin' up the place again.";
		"Outstanding!";
		4, " is my kind of place!";
		"How's it hangin'?";
		"Nice place you got here. Could use a few gibs though.";
		HELLO5;
		HELLO6;
	} //end type

	type "game_exit"
	{
		"I am so outta here.";
		"Gotta run. Happy fraggin'.";
		"It's time for me to ~bug on out of here.";
		GOODBYE5;
	} //end type

	type "level_start"
	{
		LEVEL_START0;
		"OK you rubber monkeys, make me look good.";
		"OK, time for you ", fighter, "s to make me look good.";
		"Let's rumble.";
		"I'm psyched up. Let's get it on.";
	} //end type

	type "level_end"
	{
		"At least I'm not in the basement on this ~one.";
		"Next time, let's all hammer on ", 2, ".";
		LEVEL_END0;
	} //end type

	type "level_end_victory"
	{
		"Winning ain't everything ... but losing sure stinks, right, ", 3, "?"; 
		"Wasn't my turn to win today ... so I took someone else's.";
		"My momma didn't raise me to be a loser.";
		LEVEL_END_VICTORY3;
	} //end type

	type "level_end_lose"
	{
		"Losing make me mad. Real mad. Sort of homicidal.";
		"Losing sucks big time.";
		"Next time, let's all hammer on ", 2, ".";
		LEVEL_END_LOSE1;
	} //end type

	//================================================================
	//================================================================

	type "death_telefrag"
	{
		DEATH_TELEFRAGGED0;
		DEATH_TELEFRAGGED1;
	} //end type

	type "death_lava"
	{
		"I normally like the hot stuff.";
		"Flip me over when I'm about medium rare.";
	} //end type

	type "death_slime"
	{
		"Bah! I drink coffee stronger than that.";
		"Reminds me of the ", liquid, " we used to get at the mess hall.";
	} //end type

	type "death_drown"
	{
		"Guess I was having too good a time down there.";
		"There's Pirate Treasure down here!";
		DEATH_DROWN1;
		DEATH_SUICIDE1;
	} //end type

	type "death_suicide"
	{
		"I ought to get hazard pay for this.";
		"I'm better than that. Honest.";
		"I swear somebody rearranged the arena.";
		"Heh. Had me a terminal case of hangnail there.";
		"That toe fungus just sneaked up and grabbed me.";
		DEATH_SUICIDE1;
	} //end type

	type "death_gauntlet"
	{
		"You gotta have a lotta guts to come here and do that, ", 0, ".";
		"I'm impressed. Now it's my turn.";
		0, " You are in for a beating now.";
		"Thanks for sharing that with me, ", 0, ".";
		DEATH_GAUNTLET0;
		DEATH_GAUNTLET1;
	} //end type

	type "death_rail"
	{
		"OK, that was a good shot.";
		"That just ruins my day. Now I gotta ruin yours, ", 0, ".";
		DEATH_RAIL1;
	} //end type

	type "death_bfg"
	{
		0, ", I'm tempted to grab that thing and shove it ...";
		"Use that thing once more and you're gonna eat it ", 0, ".";
		ponder, " I remember that weapon being a lot scarier.";
		DEATH_BFG0;	
	} //end type

	type "death_insult"
	{
		"I gave you an easy shot there ", 0, ".";
		"OK, ", 0, ", now try that on a moving target.";
		"I'm gonna wrap that stinkin' ", 1, " around your neck ", 0, "!";
		"Nobody does that to Grunt and just walks away from it.";
		"I don't take that kind of crap from ", fighter, "s like you, ", 0, "!";
		"Lousy stinking .... Camper!";
		DEATH_INSULT4;
	} //end type

	type "death_praise"
	{
		"Not half bad there ", 0, ".";
		"This could be more difficult than I thought.";
		"You been taking lessons, ", 0, "?";
		"Looks like you can give some pointers about the ", 1, ", ", 0, ".";
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"Gahhhh. It figures that ", 0, " would be the kind of ~jerk who'd do that.";
		"Yeah, shoot me while I'm an easy target why don't ya, ", 0, ".";
		"I was on the phone!";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		0, ", buddy, you are going to regret not fragging me in ~one shot.";
		"I've had worse cuts shaving, ", 0, ".";
		"Nice poke there, ", 0, "Too bad you didn't finish the job.";
		"Looks like you just failed target practice today, ", 0, ".";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Yeah!! I could feel that ~one hit all the way over here.";
		"You survived that, ", 0, "? I must be gettin' old.";
		"I think I found my range with that hit.";
		//0 = opponent
	} //end type

	type "enemy_suicide"
	{
		"Dang it, ", 0, ". I don't need your help to win here.";
		"That can't be good for your complexion ", 0, ".";
		"Dr. Death would be so proud of you ", 0, ".";
		//0 = enemy
	} //end type

	//================================================================
	//================================================================

	type "kill_rail" 
	{
		"'Old Betsy' still works like a charm.";
		"I just love modern technology.";
		0, ", consider yourself terminated with extreme prejudice.";
		KILL_RAIL2;
	} //end type

	type "kill_gauntlet"
	{
		"Feel the ^1POWER!";	
		"Ow. My hand hurts now.";
		"Beats a hand blaster any day.";
		KILL_GAUNTLET1;
	} //end type

	type "kill_telefrag"
	{
		"Blood and gore all over the floor and me without a spoon.";
		TELEFRAGGED0;
		TELEFRAGGED6;
	} //end type

	type "kill_insult"
	{
		"That's another score for the good guys.";
		"Of course I'm picking on you, ", 0, ". You're an easy target.";
		"I'm trying to cure you of the uglies, but it ain't working.";
		KILL_INSULT0;
		KILL_INSULT4;
		KILL_INSULT6;
		KILL_INSULT14;
		KILL_INSULT32;
		KILL_INSULT38;
	} //end type

	type "kill_praise"
	{
		"You're a hard foe to pin down, ", 0, ".";
		"Dang, you're good, ", 0, "! That frag should count double.";
		"For a moment there, I thought you were a ~Marine, ", 0, ".";
		PRAISE4;
		D_PRAISE6;
	} //end type

	//================================================================
	//================================================================
	
	type "random_insult"
	{
		"If only the good die young, I think you just might live forever ", 0, ".";
		0, ", I don't think you can handle me.";
		"And here I thought this match was going to be challenging."; 
		TAUNT2;
		TAUNT6;
		TAUNT3;
	} //end type

	type "random_misc"
	{
		MISC8;
		MISC2;		
		MISC10;
		MISC14;
		"I'm too hot for my own good.";
		"I've got a grenade with the protofiend's name on it.";
		"I've got a grenade with ", 0, "'s name on it.";
		"I'm ready to dance when you are ", 0, "."; 
	} //end type

} //end chat grunt
