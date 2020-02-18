//===========================================================================
//
// Name:			sarge_c.c
// Function:		chat lines for Sarge
// Programmer:		Jan Paul & J. Cash
// Scripter:		Paul Jaquays
// Last update:		08/24/99
// Tab Size:		3 (real tabs)
//===========================================================================

chat "sarge"
{
	#include "teamplay.h"
	//
	type "game_enter"
	{
		"Mmmmm...  Smells like fresh meat.";
		"~TEN-SHUN! ~Sarge is in the House.";  
		"Alright, ~Hero. It's time for some Pain."; 
		"*SNIFF* I smell victory in the air today.";
		"You think you got what it takes to beat me, ", 1, "?";
		HELLO5;
	} //end type

	type "game_exit"
	{
		"Keep up the pace soldiers. I'm up for some ~R and ~R.";
		"Duty calls. Try to keep from fragging yourselves.";
		"Duty calls.";
		GOODBYE;
		GOODBYE5;
	} //end type

	type "level_start"
	{
		LEVEL_START0;
		"Alright, Heroes. It's time for some ^1Pain."; 
		"Everyone drop and give me 20!";
		"Let's get this over with.";
		"It's time to separate the warriors from the kiddies.";
	} //end type

	type "level_end"
	{
		"Well fought troops. Well fought.";
		"I think we all earned a medal for that ~one.";
		"You call that a good match? Sloppy. Very sloppy.";
	} //end type

	type "level_end_victory"
	{
		"You call that effort? You're not worthy of my ordinance, pissant.";
		"Yeah! Better luck next time, ", 3, ".";
		"You gave it a good shot, ", 3, ", but next time I won't be so gentle.";
		"I'm feeling mighty fine right now. Mighty fine indeed.";
		"Yeah! The sweet smell of victory.";
		"I wish my momma could've been here to see this day.";
		LEVEL_END_VICTORY2;
	} //end type

	type "level_end_lose"
	{
		"Frag off. I'll see you again, Jumping Junior.";
		"Nice shootin' Fragbait. Just don't be getting' cocky.";
		"Little Pig, you just won by the hair of your chinny chin, chin.";
		"Fragbait.";
		"There will be some serious payback for this.";
		"Gloat while you can, fragbait. Gloat while you can.";
		"Looks like ol' ~Sarge's tactics could use some re-evaluation.";
	} //end type

	//======================================================
	//======================================================
	type "hit_talking" //bot is hit while chat balloon is visible;
	{
		"What is it that you don't understand about a fair fight, fragbait?";
		"You've just moved to the top of my hit list, ", 0, ".";
		"Shootin' a man while he's talkin' just rubs me raw, ", 0, ".";
		//0 = shooter
	} //end type

	type "damaged_nokill" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"You comin' on to me, Private?";
		"Bring it on!  Bring it on!"; //
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"It'd be quicker if you just hold still.";
		"That's just me bein' friendly.";
		//0 = opponent
	} //end type
	type "enemy_suicide"
	{
		"This your first day with that weapon, ", 0, "?";
		"Oooo.  Don'tcha just hate it when that happens.";
		"Get that ", fighter, " to the infirmary!";
		"Barely enough to send home to Momma.";
		//0 = enemy
	} //end type

	//================================================================
	//================================================================

	type "death_telefrag"
	{
		"Rrrrrr."; //
		"Well I'll be a son-of-a ... gun.";
		"That's gonna cost you a month of K-P duty, ", 0, "!";
		DEATH_TELEFRAGGED1;
		DEATH_TELEFRAGGED2;
	} //end type 

	type "death_lava"
	{
		"I didn't come here to be barbequed.";
		"Blast! Was it my turn to bring the marshmallows?";
		DEATH_LAVA0;
	} //end type

	type "death_slime"
	{
		"Horrible stuff.";
		"Rrrrrr."; //
		"I think I had that slime crap in an MRE once.";
		DEATH_SUICIDE1;
	} //end type

	type "death_drown"
	{
		"I'm Army, not some waterlogged ~Marine.";
		DEATH_SUICIDE0;
	} //end type

	type "death_suicide"  //includes weapon, cratering, & trigger_hurt
	{
		"Oooo. Don'tcha just hate it when that happens.";
		"Accidents happen.";
		"This battlefield is booby-trapped!";
		"I did that to make the odds more fair.";
		"That's no way for a warrior to die.";
		"There are worse ways to die ...";
		"No ~one said ANYTHING about land mines!";
		DEATH_SUICIDE0;
		DEATH_SUICIDE1;
	} //end type

	type "death_gauntlet"
	{
		"Rrrrrr."; //VOICE RANDOM_2
		"You comin' on to me, ", fighter, "?";
		"You're allowed to do that just once, soldier.";
		"That's an ugly way to die.";
		DEATH_GAUNTLET0;
		DEATH_GAUNTLET1;
	} //end type

	type "death_rail"
	{
		"Everyone gets lucky, once in awhile, ", 0, ".";
		"Dirty, rotten, lousy sniper.";
		"A lucky shot, ", 0, ".";
		DEATH_RAIL1;
	} //end type

	type "death_bfg"
	{
		"A real soldier can't depend on weapons of mass destruction.";
		"Next time, Private, face to face like real soldiers.";
		"Worthless fragbait.";
		"Your mommy say you could use that big gun, ", 0, "?";
		DEATH_BFG1;
	} //end type

	type "death_insult"
	{
		"Wipe that smile off your face, ", 0, ". You ~ain't out of the woods yet.";
		"Anybody can be lucky once, fragbait. Do you understand me?";
		"You can dish it out, ", 0, ". But can you take it?";
		"My dog could have done that better and quicker.";
		"Sloppy, soldier. Very sloppy.";
		"That was my favorite cigar back there. Now I'm angry.";
		"Finally! Is someone else doing the aiming for you now, ", 0,"?";
		"You've had your lucky shot for the day, ", 0, ".";
		"I'll bet you expect to get a medal for doing that, eh ", 0, "?";
//		DEATH_INSULT5;
	} //end type

	type "death_praise"
	{
		"Nice shootin', Fragbait. Just don't be getting' cocky.";
		"Well, ", 0, ". There's hope for you yet, soldier.";
		"Good shooting.";
		"You see, ", 0, ", practice does pay off in the long run.";
		"I see you've been practicing again, ", 0, ".";
		"We'll make a soldier out of you yet, ",0, ".";
	} //end type
	
	//================================================================
	//================================================================

	type "kill_rail"
	{
		"Alright!"; //VOICE RANDOM_1
		"Remind me to show you my sharpshooter ribbon.";
		"This is a truly fine weapon.";
		KILL_RAIL1;
	} //end type

	type "kill_gauntlet"
	{
		"Lesson Number ~One: Never let the enemy get that close.";
		"That's just me bein' friendly, ", 0, ".";
		KILL_GAUNTLET2;
	} //end type

	type "kill_telefrag"
	{
		TELEFRAGGED5;
		"Alright!"; //VOICE RANDOM_1
		"Let that be a lesson to you soldier ... move your butt next time.";
	} //end type 

	type "kill_insult"
	{
		"It's just like a bad dream ain't it.";
		"Get used to it, fragbait.";
		"Alright. That's more like it.";
		"Whole lot more where that came from.";
		"Heheh. Now you come in wallet size, ", 0, ".";
		"This your first day with that weapon?";
		"Let that be a lesso, ", 0, "... move your butt next time.";
		"It'd be quicker, ", 0, ", if you just hold still.";
		"Move around more, fragbait! You're too easy a target.";
		"You'd last about ~two seconds in a real shootin' war, ", 0, ".";
		0, ", you are making this far too easy.";
//		KILL_INSULT22;
//		KILL_INSULT25;
//		KILL_INSULT36;
	} //end type

	type "kill_praise"
	{
		"Fancy move, ", 0, ".  But it still got you killed.";
		"Finally, a challenge worthy of my skills.";
		"You may lose, ", 0, ", but you don't have to like it.";
		"I see you've been practicing again, ", 0, ".";
		"We'll make a soldier out of you yet, ", 0, ".";
	} //end type
	
	//================================================================
	//================================================================

	type "random_insult"
	{
		TAUNT1;
		"Rrrrrr."; 
		"It'd be quicker if you just hold still, ", 1, ".";
		"Does your mommy know you're here, ", 0, "?";
		"You're not naturally bad, ", 0, ". What you got takes practice.";
		"You want a piece of me?  You gotta take it the hard way!";
		"Come and get me fragbait.";
		"My old granny would be opening a can of whoopass here.";
		"Smells like a latrine. Or is that ", 0, "'s cologne?";
		"What?  You waitin' for your nails to dry, ", 0, "?";
		TAUNT4;
		TAUNT7;
	} //end type

	type "random_misc"
	{
		"Alright!"; 
		"Rrrrrr.";  
		"Yeah!"; 
		"Bring it on! Bring it on!"; 
		MISC8;
		MISC2;
		MISC10;
		MISC14;
		"You bring any cigars with you ", 0, "? I'm almost out.";
		"This is all some kind of game to you, isn't it?";
		"It's time to wrap this up and move on.";
		"Move your sorry butt around more, ", 1, "!  You're too easy a target.";
		1, ", you'd last about ~two seconds in a real shootin' war.";
	} //end type
	
} //end chat sarge
