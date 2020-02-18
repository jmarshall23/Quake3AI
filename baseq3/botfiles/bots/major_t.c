//===========================================================================
//
// Name:			major_c.c
// Function:		chat lines for Major
// Programmer:		Jan Paul & J. Cash
// Scripter:		Paul Jaquays
// Last update:		09/20/99
// Tab Size:		3 (real tabs)
//===========================================================================

chat "major"
{
	#include "teamplay.h"
	//
	type "game_enter"
	{
		HELLO;
		HELLO4;
		"I hope you're ready to lose today ", 1, ".";
		"You do know that I'm a VERY poor loser.";
		"Did you miss me, ", 1, "?";
		"If you quit now, ", 1, ", it will save a lot of bother.";
	} //end type

	type "game_exit"
	{
		"Just look at the time. I'm late already.";
		"Next time, don't bring the geeks ... they make the place look all trashy.";
	} //end type

	type "level_start"
	{
		LEVEL_START1;
		"I know this place. You are so seriously dead now, ", fighter, ".";
		"I'm good to go. Let's do it.";
	} //end type

	type "level_end"
	{
		LEVEL_END2;
		"So, I went easy on you this time. No biggie.";
		"It won't go the same way next time. Count on that.";
	} //end type

	type "level_end_victory"
	{
		"Uh-huh. A girl could really get used to this.";
		"Someone had to win. Today, I'm most definitely someone.";
		"Awwwww. Did mean ol' me pop a hole in someone's pretty little ego?";
		LEVEL_END_VICTORY0;
		LEVEL_END_VICTORY2;
	} //end type

	type "level_end_lose"
	{
		"I can't believe I let you do that.";
		"Do you guys play harder when you think a girl might beat you.";
		"If this were just a game, I wouldn't be so pissed right now.";
		LEVEL_END_LOSE1;
	} //end type

	//================================================================
	//================================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"Have you no honor, ", 0, "?";
		"Just because you can shoot me while I'm talking doesn't make it right, ", 0, ".";
		"~Two can play that game, ", 0, ".";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"You should have taken me out when you had the chance, ", 0, ".";
		"You know the rules, ", 0, ". Anything less than a frag doesn't count.";
		"Only a flesh wound. I'll live.  But YOU won't, ", 0, ".";
		"Don't blame the gun, ", 0, ". That was YOUR fault.";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Shoot! Something jiggled my arm!";
		"I hate wasting ammo like that.";
		"You could have done me the courtesy of dying, ", 0, ".";
		//0 = opponent
	} //end type

	type "enemy_suicide"
	{
		"Ugh. That's no way for a warrior to die, ", 0, ".";
		"Oh, I get it now, ", 0, ". You're trying to make it more challenging for yourself.";
		0, ", my mother always said, 'Stupid is as stupid does.'  Whatever that means.";
		"Victors don't let anyone kill them... including themselves.";
		//0 = enemy
	} //end type
	//================================================================
	//================================================================

	type "death_telefrag"
	{
		DEATH_TELEFRAGGED1;
		DEATH_TELEFRAGGED3;
		"Thanks for ruining my day, ", 0, ".";
	} //end type

	type "death_lava"
	{
		"What is it with all this lava?";
		"Figures. I missed the lava dancing class.";
		DEATH_LAVA1;
	} //end type

	type "death_slime"
	{
		"So, which ~one of you slugs left this slime trail?";
		"Well, ", 0, ". I can see you left your slime trail here.";
		DEATH_SLIME0;
	} //end type

	type "death_drown"
	{
		"Hey, at least I didn't get sand in my swimsuit. I hate that.";
		"Forgot to pack my floaties.";
		"*[sigh]* I miss my rebreather.";
		DEATH_DROWN0;
	} //end type

	type "death_suicide"
	{
		"Well that was friggin' spectacular. Guess I win the prize.";
		"Oh ha ha. What am I ... comic relief?";
		DEATH_SUICIDE1;
	} //end type

	type "death_gauntlet"
	{
		0, ", I suppose your mommy never said it was wrong to hit girls?";
		"I just love it when you do that ", 0, ". I dare you to try it again.";
		DEATH_GAUNTLET2;
	} //end type

	type "death_rail"
	{
		"Well, so much for having a quiet day at home.";
		"Camping AGAIN ", 0, "? Didn't your therapist tell you to stop?";
		DEATH_RAIL1;
	} //end type

	type "death_bfg"
	{
		"You're developing a really sick relationship with that thing, ", 0, ".";
		"C'mon ", 0, ". Don't you feel cheap and trashy when you do that?";
		DEATH_BFG0;
	} //end type

	type "death_insult"
	{
		"Let's see, ", 0, "; was that skill or just luck? I'm guessing ... luck.";
		"Did that make you feel like a big Warrior ", 0, ".";
		"Mmmmmmm. Was it good for you too, ", 0, "?";
		"That's right ", 0, ", pick on the girl.";
		DEATH_INSULT4;
	} //end type

	type "death_praise"
	{
		D_PRAISE2;
		PRAISE3;
		"That was definitely ... um ... pretty good, ", 0, ". Ok?";
		"I've seen better ", 0, ", but not many.";
	} //end type

	//================================================================
	//================================================================

	type "kill_rail" 
	{
		"Pow!! Right in the kisser.";
		"Quick. Clean. Efficient.";
		"Point and squirt. There's something very primal about that.";
		"Hurry back, ", 0, ". I can use the points.";
		KILL_RAIL1;
	} //end type

	type "kill_gauntlet"
	{
		"That left me breathless.";
		"Whoooooaaa. That felt great ", 0, ". Mind if I do it again?";
		"Drat, I broke a nail. Heh. Looks like a fair trade.";
		KILL_GAUNTLET0;
		KILL_GAUNTLET1;
		KILL_GAUNTLET2;
	} //end type

	type "kill_telefrag"
	{
		"A frag's a frag, ", 0, ", but I feel so ... cheap.";
		"Heh. I think you look better this way ",0, ".";
		TELEFRAGGED3;
	} //end type

	type "kill_insult"
	{
		"Oh. Did I ruin your day, ", 0, "?";
		"I came here expecting a challenge.";
		"C'mon, ", 0, ". Try harder. I'm not breaking a sweat here yet.";
		KILL_INSULT6;
		KILL_INSULT15;
		KILL_INSULT33;
	} //end type

	type "kill_praise"
	{
		PRAISE2;
		D_PRAISE4;
		"You made that really hard, ", 0, ". I like a challenge.";
		"That was so sweet of you, ", 0, ", giving me a clear shot like that.";
		"Your gore color coordinates so nicely with this arena, ", 0, ".";
	} //end type

	//================================================================
	//================================================================

	type "random_insult"
	{
		TAUNT_FEM;
		TAUNT_FEM7;
		TAUNT9;
		DEATH_INSULT5;
		"You know, ", 1, ", I've decided I like you better when you're dead.";
		"By the way, ", 1, ", I've decided I like you better when you're dead.";
		"Oh, was that your ego I deflated back there ", 1, "?, ";
		"You know, ", 0, ", bathing regularly isn't considered a crime.";
	} //end type

	type "random_misc"
	{
		TAUNT_FEM6;
		MISC3;
		MISC5;
		"What am I doing here? I promised myself no more hanging with losers.";
		"What? Are you trying to come on to me, ", 1, "?";
		"What was that you said about my mother, ", 0, ".";
		"Can we speed this up? I'm falling asleep out here.";
	} //end type
	
} //end chat major

