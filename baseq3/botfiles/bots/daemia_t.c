

//===========================================================================
//
// Name:			Daemia_c.c
// Function:		chat lines for Daemia
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:			Seven Swords, R.A. Salvataore, Paul Jaquays
// Editor:			Paul Jaquays
// Last update:		Oct. 8, 1999
// Tab Size:		3 (real tabs)
//===========================================================================

//example initial chats
chat "daemia"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"
	//======================================================
	//======================================================
	type "game_enter" //initiated when the bot enters the game
	{
		"Daemia enters the Arena, stage left. You ", fighter, "s can all cower.";
		"I think I'm gonna like this place.";
		"Name's ~Daemia, and I'm hauling all of you in.";
		1, ", I got a warrant for your destruction.";
		1, "'s here again? What am I ... loser bait?";
		"I love ", 4, "!";
		"Buenos Dias.";
		HELLO4;
		HELLO6;
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"Better worlds to conquer than this ", fighter, " heaven.";
		"I got contracts to catch up on.";
		"Hasta la vista.";
		"Te hablo luego.";
		"Adios.";
		GOODBYE5;
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"Do I make you ", fighter, "s nervous? If you are smart, you'd be nervous.";
		"It was a dark and rainy night in ", 4, ". Suddenly, A shot rang out!";
		LEVEL_START1;
		LEVEL_START2;
		// 0 = bot name
		// 4 = Level's title
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"Ay de mi'! It smells like purgatory. Huele malo.";
		"Hmphh, ", 3, ", looks like you've got a lot of work to do.";
		"You're so good, ", 2, ", we're promoting you to Xaero's punching bag.";
		"As far as I'm concerned, you're all a bunch of numbers and I'm about to win the lottery.";
		// 0 = bot name
		// 3 = opponent in last place
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"Superior training, superior tactics, superior everything.";
		"I'm surprised ... that it took me so long.";
		"And just how many times did you fall for the 'your-shoes-are-untied' bit, eh ", 3, "?";
		"Do I have to lead you around by the nose, ", 3, "?  If you had ~one left, I mean.";
		"La venganza es dulce.";
		"Te gane'.";
		LEVEL_END_VICTORY1;
		LEVEL_END_VICTORY4;
		// 0 = bot name
		// 3 = opponent in last place
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"Caramba! When did hell freeze over?";
		"Look at you ", fighter, "s! Now I know what's below the bottom of the barrel!";
		"Ay de mi'! This is a very bad thing. Que' malo.";
		"Me ganaste, ", 2, ".";
		"Este el fin.";
		LEVEL_END_LOSE0;
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"If you're shooting, you're not listening.";
		"~Jerk!";
		"I spit on you, ", 0, "!";
		TAUNT_FEM2;
		TAUNT_FEM5;
		// 0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"Ow! Don't make me come over there, ", 0, ".";
		"You are SO gonna wish you could take that back.";
		"I was gonna go easy on you ... but no more, ~jerk.";
		"Score ~one for ", 0, ". We'll put that on ", 0, "'s tombstone.";
		"Gracias, ", 0, ". I had an itch I couldn't reach there.";
		TAUNT_FEM5;
		PRAISE3;
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Cha-ching!";
		"That's like putting money in the bank.";
		"Keep running away, ", 0, ". I can hit a moving target, no problem.";
		"I'm a pinball wizard, ", fighter, ".";
		"Es suficiente o quieres mas?";
		"No lo creo.";
		TAUNT_FEM;
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"You could've knocked first, you know.";
		DEATH_TELEFRAGGED1;
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"Caramba! I've just enrolled in the WILE E. School of Stupidity.";
		"It's not how you fall, ", 0, ". It's how you land that coun ... ";
		"No, ~jerk, I can't see my house from up there.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"To me, Hell is like tropic resort. A vacation.";
		"No big deal. I had that stuff in my back yard.";
		DEATH_LAVA0;
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"At least the goo that killed me was brighter than ", 0, ".";
		"Revenge of the lime gelatin.";
		"The slime, he reminds me of my ex-husband, only cuter.";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"Well, this pool's going to need a good brushing down.";
		"You could've marked the deep end, ", 0, ".";
		"Ay chihuahua! ... and my gear's dry-clean only. Figures.";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"You know you're getting too old for this when ...";
		"Someone had to do it. I was getting tired of you being last every time, ", 3, ".";
		"Madre de dios!";
		"Ay chihuahua!";
		DEATH_SUICIDE2;
		DEATH_SUICIDE4;
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"Scratch a little lower, ", 0 ,".";
		"I don't do 'patty-cake' on a first date, ~jerk.";
		DEATH_GAUNTLET0;
		DEATH_TELEFRAGGED3;
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"Way to go, ", fighter, ". Let's get us a 'pic-a-nic' basket and go camping.";
		"How did you fire that thing? Chickens don't got fingers.";
		DEATH_RAIL1;
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"It's not how big your gun is ~jerk ... well, maybe for you it is.";
		"No skill needed there ", 0, ".";
		"You liked that, si?";
		DEATH_BFG1;
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"You got me because I aimed for your brain, ", 0, ". Impossible shot.";
		"Te veo en el infierno!";
		"Asi' se hace!";
		DEATH_INSULT2;
		DEATH_INSULT4;
		// 0 = enemy name
	} //end type


	type "death_praise" //praise initiated when the bot died
	{
		"You're all right, ", 0, ". You just might be around for a while.";
		"Nice job, ", 0, ".  Maybe next time, they let you carry a real gun.";
		"Ay caramba! You do that pretty good, ", 0, ".";
		D_PRAISE2;
		D_PRAISE4;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"Train leaving on track ~four! All aboard!";
		"Splat. Nothing to take back for a bounty.";
		"Orale!";
		"Me gusta asi!";
		KILL_RAIL1;
		KILL_RAIL2;
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"Give me that! You'll hurt someone with it.";
		"Coochie-coochie-coo!";
		"Es suficiente castigo?";
		KILL_GAUNTLET0;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"You know when those little hairs on the back of your neck stand up ...";
		"Did you eat something that didn't agree with you, ", 0, "?";
		"Con ganas!";
		":->";
		TELEFRAGGED0;
		TELEFRAGGED5;
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		"Well, I can see you don't need me, ", 0, ".";
		"Typical low-life. Always looking for the easy way out.";
		"You really shouldn't look down the barrel to see if it's loaded.";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"Don't worry, that toe tag will let your mommy know who you are.";
		"Who's your Mommy?";
		"It's never too late for a change of career, ", 0, ".";
		"Trust me when I say this: '", 0, ", you suck.'";
		"Te das por vencido ", 0, ".";
		"Te gane'.";
		KILL_INSULT0;
		KILL_INSULT1;
		KILL_INSULT5;
		KILL_INSULT9;
		KILL_INSULT35;
		KILL_INSULT33;
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"Well ", 0, ", you move better than a mannequin. There's that.";
		"You did your best, ", 0, ",...that's what should really bother you.";
		"Bueno! I didn't hear you whine once when I killed you, ", 0, "!";
		"Gracias! You die pretty good for a ", fighter, ".";
		PRAISE4;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		1, ", anyone ever mistake you for someone dangerous?";
		"So, ", 0, " is supposed to be the best. Yeah, sure.";
		"Is that your face, ", 0, ", or are you mooning me?";
		"Hey, ", 0, ", I'm salt and you're a slug. Bad combination." ;
		"Let me guess, ", 0, ". You ate a lot of paint chips when you were a kid?";
		"So you got brain damage, ", 0, "? Looks like an improvement to me.";
		"El mas fuerte no es siempre el mejor.";
		KILL_INSULT25;
		KILL_INSULT30;
		TAUNT_FEM7;
		// 0 = name of randomly chosen player
		// 1 = last victim name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"El que rie de ultimo rie mejor.";
		"The bounty on ", 0, " is worth the extra effort.";
		"See the universe. Frag interesting people.";
		"I'm hauling all you, ", fighter, "s in.";
		"Hey, ", 0, ". Your Mom did a great job dressing you this morning.";
		"Medic! Quick! I'm laughing so hard I'm choking.";
		"Orale!";
		one_liners;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end chat


