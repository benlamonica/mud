/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

/***************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com   *
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this  * 
*  code is allowed provided you add a credit line to the effect of:        *
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest    *
*  of the standard diku/rom credits. If you use this or a modified version *
*  of this code, let me know via email: moongate@moongate.ams.com. Further *
*  updates will be posted to the rom mailing list. If you'd like to get    *
*  the latest version of quest.c, please send a request to the above add-  *
*  ress. Quest Code v2.00.                                                 *
***************************************************************************/



#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

DECLARE_DO_FUN( do_say );

/* Object vnums for Quest Rewards */

#define QUEST_ITEM1 30
#define QUEST_ITEM2 31
#define QUEST_ITEM3 32
#define QUEST_ITEM4 33
#define QUEST_ITEM5 34
#define QUEST_ITEM6 35
#define QUEST_ITEM7 36
#define QUEST_ITEM8 37
#define QUEST_ITEM9 38
#define QUEST_ITEM10 39
#define QUEST_ITEM11 40
#define QUEST_ITEM12 41
#define QUEST_ITEM13 42
#define QUEST_ITEM14 43

/* Object vnums for object quest 'tokens'. In Moongate, the tokens are
   things like 'the Shield of Moongate', 'the Sceptre of Moongate'. These
   items are worthless and have the rot-death flag, as they are placed
   into the world when a player receives an object quest. */

#define QUEST_OBJQUEST1 5
#define QUEST_OBJQUEST2 6
#define QUEST_OBJQUEST3 7
#define QUEST_OBJQUEST4 8
#define QUEST_OBJQUEST5 9

/* Local functions */

void generate_quest	args(( CHAR_DATA *ch, CHAR_DATA *questman ));
void quest_update	args(( void ));
bool chance		args(( int num ));

/* CHANCE function. I use this everywhere in my code, very handy :> */

bool chance(int num)
{
    if (number_range(1,100) <= num) return TRUE;
    else return FALSE;
}

/* The main quest function */

void do_quest(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *questman;
    OBJ_DATA *obj=NULL, *obj_next;
    OBJ_INDEX_DATA *questinfoobj;
    MOB_INDEX_DATA *questinfo;
    char buf [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];

    if(IS_SET(ch->act,PLR_IMMQUEST))
    {
	do_question(ch,argument);
	return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (!strcmp(arg1, "info"))
    {
	if (IS_SET(ch->act, PLR_QUESTOR))
	{
	    if (ch->questmob == -1 && ch->questgiver->short_descr != NULL)
	    {
		sprintf(buf, "Your quest is ALMOST complete!\n\rGet back to %s before your time runs out!\n\r",ch->questgiver->short_descr);
		send_to_char(buf, ch);
	    }
	    else if (ch->questobj > 0)
	    {
                questinfoobj = get_obj_index(ch->questobj);
		if (questinfoobj != NULL)
		{
		    sprintf(buf, "You are on a quest to recover the fabled %s!\n\r",questinfoobj->name);
		    send_to_char(buf, ch);
		}
		else send_to_char("You aren't currently on a quest.\n\r",ch);
		return;
	    }
	    else if (ch->questmob > 0)
	    {
                questinfo = get_mob_index(ch->questmob);
		if (questinfo != NULL)
		{
	            sprintf(buf, "You are on a quest to slay the dreaded %s!\n\r",questinfo->short_descr);
		    send_to_char(buf, ch);
		}
		else send_to_char("You aren't currently on a quest.\n\r",ch);
		return;
	    }
	}
	else
	    send_to_char("You aren't currently on a quest.\n\r",ch);
	return;
    }
    if (!strcmp(arg1, "points"))
    {
	sprintf(buf, "You have %d quest points.\n\r",ch->questpoints);
	send_to_char(buf, ch);
	return;
    }
    else if (!strcmp(arg1, "time"))
    {
	if (!IS_SET(ch->act, PLR_QUESTOR))
	{
	    send_to_char("You aren't currently on a quest.\n\r",ch);
	    if (ch->nextquest > 1)
	    {
		sprintf(buf, "There are %d minutes remaining until you can go on another quest.\n\r",ch->nextquest);
		send_to_char(buf, ch);
	    }
	    else if (ch->nextquest == 1)
	    {
		sprintf(buf, "There is less than a minute remaining until you can go on another quest.\n\r");
		send_to_char(buf, ch);
	    }
	}
        else if (ch->countdown > 0)
        {
	    sprintf(buf, "Time left for current quest: %d\n\r",ch->countdown);
	    send_to_char(buf, ch);
	}
	return;
    }

/* Checks for a character in the room with spec_questmaster set. This special
   procedure must be defined in special.c. You could instead use an 
   ACT_QUESTMASTER flag instead of a special procedure. */

    for ( questman = ch->in_room->people; questman != NULL; questman = questman->next_in_room )
    {
	if (!IS_NPC(questman)) continue;
        if (IS_SET(questman->act,ACT_QUESTMASTER)) break;
    }

    if (questman == NULL || !IS_SET(questman->act,ACT_QUESTMASTER))
    {
        send_to_char("You can't do that here.\n\r",ch);
        return;
    }

    if ( questman->fighting != NULL)
    {
	send_to_char("Wait until the fighting stops.\n\r",ch);
        return;
    }

    ch->questgiver = questman;

/* And, of course, you will need to change the following lines for YOUR
   quest item information. Quest items on Moongate are unbalanced, very
   very nice items, and no one has one yet, because it takes awhile to
   build up quest points :> Make the item worth their while. */

    if (!strcmp(arg1, "list"))
    {
        act( "$n asks $N for a list of quest items.", ch, NULL, questman, TO_ROOM); 
	act ("You ask $N for a list of quest items.",ch, NULL, questman, TO_CHAR);
	sprintf(buf, "Current Quest Items available for Purchase:\n\r\
10000qp.........A personal safe room!!!\n\r\
1000qp.........The Flail of Torture\n\r\
950qp..........The Flaming Mace\n\r\
900qp..........The Sword of Frost and Ice\n\r\
900qp..........The Frost Dagger\n\r\
800qp..........The Knuckles of Power\n\r\
800qp..........A token worth one creation point\n\r\
700qp..........The Light of Eternal Darkness\n\r\
600qp..........The Belt of Blood and Gore\n\r\
600qp..........The Bracer of Power\n\r\
600qp..........The Gloves of Fortitude\n\r\
600qp..........100 permanent movement points\n\r\
500qp..........350,000 gold pieces\n\r\
300qp..........The Decanter of Endless Drinking\n\r\
100qp..........A potion of Super Healing\n\r\
To buy an item, type 'QUEST BUY <item>'.\n\r");
	send_to_char(buf, ch);
	return;
    }


    else if (!strcmp(arg1, "buy"))
    {
	if (arg2[0] == '\0')
	{
	    send_to_char("To buy an item, type 'QUEST BUY <item>'.\n\r",ch);
	    return;
	}
	if (is_name(arg2, "room personal safe"))
	{
	    if (ch->questpoints >= 10000)
	    {
		ch->questpoints -= 10000;
	        obj = create_object(get_obj_index(QUEST_ITEM2),ch->level);
	    }
	    else
	    {
      		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else if (is_name(arg2, "token"))
	{
	    if (ch->questpoints >= 800)
	    {
		ch->questpoints -= 800;
	        obj = create_object(get_obj_index(QUEST_ITEM1),ch->level);
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else if (is_name(arg2, "decanter endless drinking"))
	{
	    if (ch->questpoints >= 300)
	    {
		ch->questpoints -= 300;
	        obj = create_object(get_obj_index(QUEST_ITEM3),ch->level);
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else if (is_name(arg2, "frost dagger"))
	{
	    if (ch->questpoints >= 900)
	    {
		ch->questpoints -= 900;
	        obj = create_object(get_obj_index(QUEST_ITEM4),ch->level);
	    }
	    else
	    {
  		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
        }
        else if (is_name(arg2, "flail torture"))
        {
            if (ch->questpoints >= 1000)
            {
                ch->questpoints -= 1000;
                obj = create_object(get_obj_index(QUEST_ITEM6),ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
        else if (is_name(arg2, "gloves fortitude"))
        {
            if (ch->questpoints >= 600)
            {
                ch->questpoints -= 600;
                obj = create_object(get_obj_index(QUEST_ITEM7),ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
        else if (is_name(arg2, "movement points 100 permanent"))
        {
            if (ch->questpoints >= 600)
            {
                ch->questpoints -= 600;
                obj = create_object(get_obj_index(QUEST_ITEM8),ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }

	}
	else if (is_name(arg2, "flaming mace"))
	{
	    if (ch->questpoints >= 950)
	    {
		ch->questpoints -= 950;
	        obj = create_object(get_obj_index(QUEST_ITEM5),ch->level);
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
/*	}
	else if (is_name(arg2, "practices pracs prac practice"))
	{
	    if (ch->questpoints >= 500)
	    {
		ch->questpoints -= 500;
	        ch->practice += 30;
    	        act( "$N gives 30 practices to $n.", ch, NULL, questman, TO_ROOM );
    	        act( "$N gives you 30 practices.",   ch, NULL, questman, TO_CHAR );
	        return;
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }*/
	}
	else if (is_name(arg2, "gold gp"))
	{
	    if (ch->questpoints >= 500)
	    {
		ch->questpoints -= 500;
	        ch->gold += 350000;
    	        act( "$N gives 350,000 gold pieces to $n.", ch, NULL, questman, TO_ROOM );
    	        act( "$N has 350,000 in gold transferred from $s Midgaard account to your balance.",   ch, NULL, questman, TO_CHAR );
	        return;
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
        }
        else if (is_name(arg2, "super heal potion"))
        {
            if (ch->questpoints >= 100)
            {
                ch->questpoints -= 100;
                obj = create_object(get_obj_index(QUEST_ITEM9),ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
        else if (is_name(arg2, "sword frost ice"))
        {
            if (ch->questpoints >= 900)
            {
                ch->questpoints -= 900;
                obj = create_object(get_obj_index(QUEST_ITEM10),ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
        else if (is_name(arg2, "knuckles power"))
        {
            if (ch->questpoints >= 800)
            {
                ch->questpoints -= 800;
                obj = create_object(get_obj_index(QUEST_ITEM11),ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
        else if (is_name(arg2, "light eternal darkness"))
        {
            if (ch->questpoints >= 700)
            {
                ch->questpoints -= 700;
                obj = create_object(get_obj_index(QUEST_ITEM12),ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
        else if (is_name(arg2, "belt blood gore"))
        {
            if (ch->questpoints >= 600)
            {
                ch->questpoints -= 600;
                obj = create_object(get_obj_index(QUEST_ITEM13),ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
        else if (is_name(arg2, "bracer power"))
        {
            if (ch->questpoints >= 600)
            {
                ch->questpoints -= 600;
                obj = create_object(get_obj_index(QUEST_ITEM14),ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }

	}
	else
	{
	    sprintf(buf, "I don't have that item, %s.",ch->name);
	    do_say(questman, buf);
	}
	if (obj != NULL)
	{
    	    act( "$N gives $p to $n.", ch, obj, questman, TO_ROOM );
    	    act( "$N gives you $p.",   ch, obj, questman, TO_CHAR );
	    obj_to_char(obj, ch);
	}
	return;
    }
    else if (!strcmp(arg1, "request"))
    {
        act( "$n asks $N for a quest.", ch, NULL, questman, TO_ROOM); 
	act ("You ask $N for a quest.",ch, NULL, questman, TO_CHAR);
	if (IS_SET(ch->act, PLR_QUESTOR))
	{
	    sprintf(buf, "But you're already on a quest!");
	    do_say(questman, buf);
	    return;
	}
	if (ch->nextquest > 0)
	{
	    sprintf(buf, "You're very brave, %s, but let someone else have a chance.",ch->name);
	    do_say(questman, buf);
	    sprintf(buf, "Come back later.");
	    do_say(questman, buf);
	    return;
	}

	sprintf(buf, "Thank you, brave %s!",ch->name);
	do_say(questman, buf);

	generate_quest(ch, questman);

        if (ch->questmob > 0 || ch->questobj > 0)
	{
            ch->countdown = number_range(10,30);
	    SET_BIT(ch->act, PLR_QUESTOR);
	    sprintf(buf, "You have %d minutes to complete this quest.",ch->countdown);
	    do_say(questman, buf);
	    sprintf(buf, "May the gods go with you!");
	    do_say(questman, buf);
	}
	return;
    }
    else if (!strcmp(arg1, "complete"))
    {
        act( "$n informs $N $e has completed $s quest.", ch, NULL, questman, TO_ROOM); 
	act ("You inform $N you have completed $s quest.",ch, NULL, questman, TO_CHAR);
	if (ch->questgiver != questman)
	{
	    sprintf(buf, "I never sent you on a quest! Perhaps you're thinking of someone else.");
	    do_say(questman,buf);
	    return;
	}

	if (IS_SET(ch->act, PLR_QUESTOR))
	{
	    if (ch->questmob == -1 && ch->countdown > 0)
	    {
		int reward, pointreward, pracreward;

		reward = number_range(1500,15000);
		pointreward = number_range(10,40);

		sprintf(buf, "Congratulations on completing your quest!");
		do_say(questman,buf);
		sprintf(buf,"As a reward, I am giving you %d quest points, and %d gold.",pointreward,reward);
		do_say(questman,buf);
		if (chance(15))
		{
		    pracreward = number_range(1,6);
		    sprintf(buf, "You gain %d practices!\n\r",pracreward);
		    send_to_char(buf, ch);
		    ch->practice += pracreward;
		}

	        REMOVE_BIT(ch->act, PLR_QUESTOR);
	        ch->questgiver = NULL;
	        ch->countdown = 0;
	        ch->questmob = 0;
		ch->questobj = 0;
	        ch->nextquest = 30;
		ch->gold += reward;
		ch->questpoints += pointreward;

	        return;
	    }
	    else if (ch->questobj > 0 && ch->countdown > 0)
	    {
		bool obj_found = FALSE;

    		for (obj = ch->carrying; obj != NULL; obj= obj_next)
    		{
        	    obj_next = obj->next_content;
        
		    if (obj != NULL && obj->pIndexData->vnum == ch->questobj)
		    {
			obj_found = TRUE;
            	        break;
		    }
        	}
		if (obj_found == TRUE)
		{
		    int reward, pointreward, pracreward;

		    reward = number_range(1500,25000);
		    pointreward = number_range(10,40);

		    act("You hand $p to $N.",ch, obj, questman, TO_CHAR);
		    act("$n hands $p to $N.",ch, obj, questman, TO_ROOM);

	    	    sprintf(buf, "Congratulations on completing your quest!");
		    do_say(questman,buf);
		    sprintf(buf,"As a reward, I am giving you %d quest points, and %d gold.",pointreward,reward);
		    do_say(questman,buf);
		    if (chance(15))
		    {
		        pracreward = number_range(1,6);
		        sprintf(buf, "You gain %d practices!\n\r",pracreward);
		        send_to_char(buf, ch);
		        ch->practice += pracreward;
		    }

	            REMOVE_BIT(ch->act, PLR_QUESTOR);
	            ch->questgiver = NULL;
	            ch->countdown = 0;
	            ch->questmob = 0;
		    ch->questobj = 0;
	            ch->nextquest = 30;
		    ch->gold += reward;
		    ch->questpoints += pointreward;
		    extract_obj(obj);
		    return;
		}
		else
		{
		    sprintf(buf, "You haven't completed the quest yet, but there is still time!");
		    do_say(questman, buf);
		    return;
		}
		return;
	    }
	    else if ((ch->questmob > 0 || ch->questobj > 0) && ch->countdown > 0)
	    {
		sprintf(buf, "You haven't completed the quest yet, but there is still time!");
		do_say(questman, buf);
		return;
	    }
	}
	if (ch->nextquest > 0)
	    sprintf(buf,"But you didn't complete your quest in time!");
	else sprintf(buf, "You have to REQUEST a quest first, %s.",ch->name);
	do_say(questman, buf);
	return;
    }

    send_to_char("QUEST commands: POINTS INFO TIME REQUEST COMPLETE LIST BUY.\n\r",ch);
    send_to_char("For more information, type 'HELP QUEST'.\n\r",ch);
    return;
}

void generate_quest(CHAR_DATA *ch, CHAR_DATA *questman)
{
    CHAR_DATA *victim;
    MOB_INDEX_DATA *vsearch;
    ROOM_INDEX_DATA *room;
    OBJ_DATA *questitem;
    char buf [MAX_STRING_LENGTH];
    long mcounter;
    int level_diff, mob_vnum;

    /*  Randomly selects a mob from the world mob list. If you don't
	want a mob to be selected, make sure it is immune to summon.
	Or, you could add a new mob flag called ACT_NOQUEST. The mob
	is selected for both mob and obj quests, even tho in the obj
	quest the mob is not used. This is done to assure the level
	of difficulty for the area isn't too great for the player. */

    for (mcounter = 0; mcounter < 99999; mcounter ++)
    {
	mob_vnum = number_range(50, 32600);

	if ( (vsearch = get_mob_index(mob_vnum) ) != NULL )
	{
	    level_diff = vsearch->level - ch->level;

		/* Level differences to search for. Moongate has 350
		   levels, so you will want to tweak these greater or
		   less than statements for yourself. - Vassago */
		
if (vsearch->pShop != NULL)
	continue;

	    if (((level_diff < 15 && level_diff > -5)
		|| (ch->level > 90 && vsearch->level > 91 && vsearch->level < 150))
//		&& IS_EVIL(vsearch)
		&& !IS_SET(vsearch->imm_flags, IMM_SUMMON)
    		&& !IS_SET(vsearch->act,ACT_TRAIN)
		&& !IS_SET(vsearch->act,ACT_PET)
		&& !IS_SET(vsearch->act,ACT_GAIN)
    		&& !IS_SET(vsearch->act,ACT_PRACTICE)
    		&& !IS_SET(vsearch->act,ACT_IS_HEALER)
                && !IS_SET(vsearch->act,ACT_NO_QUEST)
		&& chance(35)) break;
		else vsearch = NULL;
	}
    }

    if ( vsearch == NULL || ( victim = get_char_world( ch, 
vsearch->player_name ) ) == NULL )
    {
	sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.");
	do_say(questman, buf);
	sprintf(buf, "Try again later.");
	do_say(questman, buf);
	ch->nextquest = 5;
        return;
    }

    if ( ( room = find_location( ch, victim->name ) ) == NULL )
    {
	sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.");
	do_say(questman, buf);
	sprintf(buf, "Try again later.");
	do_say(questman, buf);
	ch->nextquest = 5;
        return;
    }

    /*  40% chance it will send the player on a 'recover item' quest. */

    if (chance(40))
    {
	int objvnum = 0;

	switch(number_range(0,4))
	{
	    case 0:
	    objvnum = QUEST_OBJQUEST1;
	    break;

	    case 1:
	    objvnum = QUEST_OBJQUEST2;
	    break;

	    case 2:
	    objvnum = QUEST_OBJQUEST3;
	    break;

	    case 3:
	    objvnum = QUEST_OBJQUEST4;
	    break;

	    case 4:
	    objvnum = QUEST_OBJQUEST5;
	    break;
	}

        questitem = create_object( get_obj_index(objvnum), 100 );
	obj_to_room(questitem, room);
	ch->questobj = questitem->pIndexData->vnum;

	sprintf(buf, "Vile pilferers have stolen %s from the royal treasury!",questitem->short_descr);
	do_say(questman, buf);
	do_say(questman, "My court wizardess, with her magic mirror, has pinpointed its location.");

	/* I changed my area names so that they have just the name of the area
	   and none of the level stuff. You may want to comment these next two
	   lines. - Vassago */

	sprintf(buf, "Look in the general area of %s for %s!",room->area->name, room->name);
	do_say(questman, buf);
	return;
    }

    /* Quest to kill a mob */

    else 
    {
    switch(number_range(0,1))
    {
	case 0:
        sprintf(buf, "An enemy of mine, %s, is making vile threats against the crown.",victim->short_descr);
        do_say(questman, buf);
        sprintf(buf, "This threat must be eliminated!");
        do_say(questman, buf);
	break;

	case 1:
	sprintf(buf, "Our most heinous criminal, %s, has escaped from the dungeon!",victim->short_descr);
	do_say(questman, buf);
	sprintf(buf, "Since the escape, %s has murdered %d civilians!",victim->short_descr, number_range(2,20));
	do_say(questman, buf);
	do_say(questman,"The penalty for this crime is death, and you are to deliver the sentence!");
	break;
    }

    if (room->name != NULL)
    {
        sprintf(buf, "Seek %s out somewhere in the vicinity of %s!",victim->short_descr,room->name);
        do_say(questman, buf);

	/* I changed my area names so that they have just the name of the area
	   and none of the level stuff. You may want to comment these next two
	   lines. - Vassago */

	sprintf(buf, "That location is in the general area of %s.",room->area->name);
	do_say(questman, buf);
    }
    ch->questmob = victim->pIndexData->vnum;
    }
    return;
}

/* Called from update_handler() by pulse_area */

void quest_update(void)
{
    CHAR_DATA *ch, *ch_next;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
        ch_next = ch->next;

	if (IS_NPC(ch)) continue;

	if (ch->nextquest > 0) 
	{
	    ch->nextquest--;

	    if (ch->nextquest == 0)
	    {
	        send_to_char("You may now quest again.\n\r",ch);
	        return;
	    }
	}
        else if (IS_SET(ch->act,PLR_QUESTOR))
        {
	    if (--ch->countdown <= 0)
	    {
    	        char buf [MAX_STRING_LENGTH];

	        ch->nextquest = 30;
	        sprintf(buf, "You have run out of time for your quest!\n\rYou may quest again in %d minutes.\n\r",ch->nextquest);
	        send_to_char(buf, ch);
	        REMOVE_BIT(ch->act, PLR_QUESTOR);
                ch->questgiver = NULL;
                ch->countdown = 0;
                ch->questmob = 0;
	    }
	    if (ch->countdown > 0 && ch->countdown < 6)
	    {
	        send_to_char("Better hurry, you're almost out of time for your quest!\n\r",ch);
	        return;
	    }
        }
    }
    return;
}
