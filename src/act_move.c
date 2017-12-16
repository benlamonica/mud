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

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_stand		);
DECLARE_DO_FUN(do_home          );

void make_corpse args((CHAR_DATA *victim));
char *	const	dir_name	[]		=
{
    "north", "east", "south", "west", "up", "down"
};

const	sh_int	rev_dir		[]		=
{
    2, 3, 0, 1, 5, 4
};

const	sh_int	movement_loss	[SECT_MAX]	=
{
    1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6
};



/*
 * Local functions.
 */
int	find_door	args( ( CHAR_DATA *ch, char *arg ) );
bool	has_key_to_door	args( ( CHAR_DATA *ch, int key ) );


void move_char( CHAR_DATA *ch, int door, bool follow )
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    bool fry=FALSE;
    OBJ_DATA *obj;
    CHAR_DATA *attacker;
    int ndoor;
    AFFECT_DATA af;
    bool web=FALSE;

    

    if ( door < 0 || door > 5 )
    {
	bug( "Do_move: bad door %d.", door );
	return;
    }

    for(obj = ch->in_room->contents; obj!=NULL; obj=obj->next)
    {
	if(obj->item_type==ITEM_TRAP && (obj->level-7 <= ch->level) &&
	  (obj->level+7 >= ch->level))
        {
	    if(obj->value[0]==door)
            {
		fry=TRUE;
		break;
            }
	}
    }

    if(IS_NPC(ch)) fry = FALSE;

    in_room = ch->in_room;
    if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->u1.to_room   ) == NULL 
    ||	 !can_see_room(ch,pexit->u1.to_room))
    {
	send_to_char( "Alas, you cannot go that way.\n\r", ch );
	return;
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    &&   !IS_AFFECTED(ch, AFF_PASS_DOOR) )
    {
	act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master != NULL
    &&   in_room == ch->master->in_room )
    {
	send_to_char( "What?  And leave your beloved master?\n\r", ch );
	return;
    }

    /*Don't let them leave if they are webbed.*/
    if ( IS_AFFECTED(ch, AFF_WEB) )
    {
	if( number_percent() < (int) get_curr_stat(ch,STAT_STR)*1.25 )
        {
	    send_to_char("You break out of your webs!\n\r",ch);
	    act("$n breaks out of the webs holding $m in the room.",ch,NULL,NULL,TO_ROOM);
	    affect_strip(ch,skill_lookup("web"));
	    REMOVE_BIT(ch->affected_by,AFF_WEB);
        }
	else
	{
	    send_to_char("You struggle in vain as you try to free yourself from the webs.\n\r",ch);
	    act("$n struggles in vain to free $mself from the webs surrounding $m.",ch,NULL,NULL,TO_ROOM);
	    return;
        }
    }

    if ( room_is_private( to_room ) )
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) )
    {
	int move;

/*	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    for ( iGuild = 0; iGuild < MAX_GUILD; iGuild ++)	
	    {
	    	if ( iClass != ch->class
	    	&&   to_room->vnum == class_table[iClass].guild[iGuild] )
	    	{
		    send_to_char( "You aren't allowed in there.\n\r", ch );
		    return;
		}
	    }
	}
*/
	if ( in_room->sector_type == SECT_AIR
	||   to_room->sector_type == SECT_AIR )
	{
	    if ( !IS_AFFECTED(ch, AFF_FLYING) && !IS_IMMORTAL(ch))
	    {
		send_to_char( "You can't fly.\n\r", ch );
		return;
	    }
	}

	if (( in_room->sector_type == SECT_WATER_NOSWIM
	||    to_room->sector_type == SECT_WATER_NOSWIM )
  	&&    !IS_AFFECTED(ch,AFF_FLYING))
	{
	    OBJ_DATA *obj;
	    bool found;

	    /*
	     * Look for a boat.
	     */
	    found = FALSE;

	    if (IS_IMMORTAL(ch))
		found = TRUE;

	    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	    {
		if ( obj->item_type == ITEM_BOAT )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
	    {
		send_to_char( "You need a boat to go there.\n\r", ch );
		return;
	    }
	}

	move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
	     + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)]
	     ;

        move /= 2;  /* i.e. the average */

	if ( ch->move < move )
	{
	    send_to_char( "You are too exhausted.\n\r", ch );
	    return;
	}

	WAIT_STATE( ch, 1 );
	ch->move -= move;
    }

    if ( IS_AFFECTED(ch, AFF_HIDE) )
	REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if( fry == TRUE )
    {
	attacker = get_char_world2(ch,obj->name);
	if(attacker != ch)
	{
	    if(attacker != NULL)
            {
	    	send_to_char("You have triggered a trap!\n\r",ch);
	    	act("$n has triggered a trap!",ch,NULL,NULL,TO_ROOM);
		if(obj->value[2] != skill_lookup("web"))
	    	    damage2( attacker, ch, (obj->level+10)*2, obj->value[2], obj->value[1]);
		else
		{
		    send_to_char("Webs fall on you and hold you to the floor.\n\r",ch);
		    af.type      = skill_lookup("web");
       	 	    af.level     = attacker->level;
        	    af.duration  = attacker->level / 2;
       	 	    af.location  = APPLY_DEX;
        	    af.modifier  = -1;
        	    af.bitvector = AFF_WEB;
        	    affect_to_char( ch, &af );
		    web = TRUE;
                }
            }
	
	    ndoor = door;
	    switch(ndoor)
            {
		case 0: ndoor = 2; break;
        	case 1: ndoor = 3; break;
        	case 2: ndoor = 0; break;
        	case 3: ndoor = 1; break;
        	case 4: ndoor = 5; break;
        	case 5: ndoor = 4; break;
            }

	    obj_from_room(obj);
	    extract_obj(obj);

	    for(obj = to_room->contents; obj!=NULL; obj=obj->next)
    	    {
	    	if(obj->item_type==ITEM_TRAP)
            	{
	    	    if(obj->value[0]==ndoor)
                    {
		    	obj_from_room(obj);
		    	extract_obj(obj);
		    	break;
            	    }
	        }
     	    }
	}
    }

    if(web == TRUE)
	return;
	
    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    && ( IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS) ) )
	act( "$n leaves $T.", ch, NULL, dir_name[door], TO_ROOM );

    char_from_room( ch );
    char_to_room( ch, to_room );
    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    && ( IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS) ) )
	act( "$n has arrived.", ch, NULL, NULL, TO_ROOM );

    do_look( ch, "auto" );

    if (in_room == to_room) /* no circular follows */
	return;

    for ( fch = in_room->people; fch != NULL; fch = fch_next )
    {
	fch_next = fch->next_in_room;

	if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM) 
	&&   fch->position < POS_STANDING)
	    do_stand(fch,"");

	if ( fch->master == ch && fch->position == POS_STANDING )
	{

	    if (IS_SET(ch->in_room->room_flags,ROOM_LAW)
	    &&  (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE)))
	    {
		act("You can't bring $N into the city.",
		    ch,NULL,fch,TO_CHAR);
		act("You aren't allowed in the city.",
		    fch,NULL,NULL,TO_CHAR);
		return;
	    }

	    act( "You follow $N.", fch, NULL, ch, TO_CHAR );
	    move_char( fch, door, TRUE );
	}
    }

    return;
}

void do_smash( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Smash what?\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_smash].beats );

    /* look for guards */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level )
	{
	    act( "$N is standing too close. $E is protecting that.",
		ch, NULL, gch, TO_CHAR );
	    return;
	}
    }

    if ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[gsn_smash])
    {
	send_to_char( "You run try to get a running start, but fail miserably.\n\r", ch);
	check_improve(ch,gsn_smash,FALSE,2);
	return;
    }
    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'pick object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "You can't smash solid objects.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed. Try just getting whatever you need from it.\n\r", ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "You fail miserable in your attempt to open this unbreakable object.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked. Try opening it.\n\r", ch ); return; }
	if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
	    { send_to_char( "You fail miserable in your attempt to open this unbreakable object.\n\r", ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	act( "You run full speed into $p smashing it open.\n\r", ch,obj,NULL,TO_CHAR );
	check_improve(ch,gsn_smash,TRUE,2);
	act( "$n runs and smashes into the $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'pick door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch))
	    { send_to_char( "It's not closed. Try walking through it.\n\r", ch ); return; }
	if ( pexit->key < 0 && !IS_IMMORTAL(ch))
	    { send_to_char( "You eye the door carefully, but it appears to be made of steel. You rethink your plan.\n\r", ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked. Try opening it!\n\r", ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_PICKPROOF))
	    { send_to_char( "You run full force into the door, and fall backwards onto your butt. OUCH!\n\r", ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	REMOVE_BIT(pexit->exit_info, EX_CLOSED);

	send_to_char( "You smash the door to splinters, leaving only the hinges on the frame.\n\r", ch );
	act( "$n runs full speed and smashes the $d down and flies from the room.", ch, NULL, pexit->keyword, TO_ROOM );
	check_improve(ch,gsn_smash,TRUE,2);

	/* pick the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	    REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
	}
	char_from_room(ch);
	char_to_room(ch,pexit->u1.to_room);
    }

    return;
}

/*
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    argument = one_argument(argument, arg);

    if(((obj = get_obj_room(ch,arg))!=NULL) || ((obj = get_obj_char(ch,arg))!=NULL)
    {
	if(obj->type==ITEM_CONTAINER)
        {
	    act("You try to smash $p, but can't seem to crush the solid object.",ch,obj,NULL,TO_CHAR);
	    return;
        }

	
    
    if
    if(IS_SET(ch->in_room->exit[direction]->exit_info,EX_CLOSED) && !IS_AFFECTED(victim,AFF_PASS_DOOR))
    {
	
    if(ch->in_room*/


void do_enter( CHAR_DATA *ch, char *argument, bool follow )
{
    OBJ_DATA *portal;
    ROOM_INDEX_DATA *to_room,*in_room;
    CHAR_DATA *fch, *fch_next;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    
     argument = one_argument(argument, arg);
    portal = get_obj_list(ch, arg, ch->in_room->contents);
    in_room = ch->in_room;
        
    if(portal==NULL)
    {
        send_to_char("You don't see that here.\n\r",ch);
        return;
    }

    if(portal->item_type != ITEM_PORTAL){
	sprintf(buf,"You cannot enter a %s.\n\r",portal->name);
	send_to_char(buf,ch);
	return;
   }
    if ( IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master != NULL
    &&   in_room == ch->master->in_room )
    {
	send_to_char( "What?  And leave your beloved master?\n\r", ch );
	return;
    }

   if(IS_AFFECTED(ch,AFF_WEB))
   {
	act("You can't enter $p because of the webs holding you down.",ch,portal,NULL,TO_CHAR);
	return;
   }

   to_room = get_room_index(portal->value[3]);

   if(to_room==NULL)
   {
	act("You attempt to enter $p, but discover that it leads no where.",ch,portal,NULL,TO_CHAR);
	return;
   }

   act( "$n enters $p and vanishes.", ch, portal, NULL, TO_ROOM );
   act( "You enter $p and find yourself elsewhere...",ch, portal, NULL, TO_CHAR );

    char_from_room( ch );
    char_to_room( ch, to_room );

    act( "$n has arrived through $p.", ch, portal, NULL, TO_ROOM );
   
    do_look( ch, "auto" );
                
    if (in_room == to_room) 
        return;
    
    for ( fch = in_room->people; fch != NULL; fch = fch_next )
    {
        fch_next = fch->next_in_room;
        
        if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM)
        &&   fch->position < POS_STANDING)
            do_stand(fch,""); 
         
        if ( fch->master == ch && fch->position == POS_STANDING )
        {
         
            if (IS_SET(ch->in_room->room_flags,ROOM_LAW)
            &&  (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE)))
            {
                act("You can't bring $N into the city.",
                    ch,NULL,fch,TO_CHAR);
                act("You aren't allowed in the city.",
                    fch,NULL,NULL,TO_CHAR);
                return;
            }
    
            act( "You follow $N.", fch, NULL, ch, TO_CHAR );
            do_enter( fch, arg, TRUE );
        }
    }
}



void do_north( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_NORTH, FALSE );
    return;
}



void do_east( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_EAST, FALSE );
    return;
}



void do_south( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_SOUTH, FALSE );
    return;
}



void do_west( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_WEST, FALSE );
    return;
}



void do_up( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_UP, FALSE );
    return;
}



void do_down( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_DOWN, FALSE );
    return;
}



int find_door( CHAR_DATA *ch, char *arg )
{
    EXIT_DATA *pexit;
    int door;

	 if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = 5;
    else
    {
	for ( door = 0; door <= 5; door++ )
	{
	    if ( ( pexit = ch->in_room->exit[door] ) != NULL
	    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
	    &&   pexit->keyword != NULL
	    &&   is_name( arg, pexit->keyword ) )
		return door;
	}
	act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	act( "I see no door $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return -1;
    }

    return door;
}

void do_push( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH]="\0";
    char arg1[MAX_INPUT_LENGTH]="\0";
    char arg2[MAX_INPUT_LENGTH]="\0";
    CHAR_DATA *victim;
    int direction=-1;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if(arg1[0]=='\0')
    {
	send_to_char("Push who what where?\n\r",ch);
	return;
    }

    if((victim = get_char_room(ch,arg1)) == NULL)
    {
	send_to_char("They aren't there.\n\r",ch);
	return;
    }

    if(victim==ch)
    {
	send_to_char("You push yourself in vain.\n\r",ch);
	return;
    }

    if( IS_NPC(victim) && (IS_SET(victim->act,ACT_SENTINEL) || IS_SET(victim->act,ACT_TRAIN) || IS_SET(victim->act,ACT_PRACTICE) || IS_SET(victim->act,ACT_IS_HEALER) ))
    {
	send_to_char("They cannot be pushed!\n\r",ch);
	return;
    }

    if(victim->position == POS_RESTING)
    {
	sprintf(buf,"%s is resting. Try dragging them.\n\r",victim->short_descr[0]=='\0'?victim->name:victim->short_descr);
	send_to_char(buf,ch);
	return;
    }

    if(victim->position == POS_FIGHTING)
    {
	sprintf(buf,"%s is fighting! Wait until they finish\n\r",victim->short_descr[0]=='\0'?victim->name:victim->short_descr);
	return;
    }

    if(victim->position == POS_SLEEPING)
    {
	sprintf(buf,"%s is sleeping. Try dragging them.\n\r",victim->short_descr[0]=='\0'?victim->name:victim->short_descr);
	send_to_char(buf,ch);
	return;
    }

    if(IS_AFFECTED(ch, AFF_WEB))
    {
	send_to_char("You can't leave the room! You're covered in webbing!\n\r",ch);
	return;
    }

    if(IS_AFFECTED(victim, AFF_WEB))
    {
	act("The webs holding $N prevent you from pushing $M from the room.",ch,NULL,victim,TO_CHAR);
	act("The webs holding you prevent $n from pushing you from the room.",ch,NULL,victim,TO_VICT);
	return;
    }


    if(victim->level < 16 && !IS_NPC(victim))
    {
	send_to_char("You may not push characters under the level of 16.\n\r",ch);
	return;
    }

    if(ch->level < 16 && !IS_NPC(victim))
    {
	send_to_char("Pushing a player is an aggressive action. Wait until you are level 16.\n\r",ch);
	return;
    }

    if(arg2[0]=='\0')
    {
	send_to_char("Push which direction?\n\r",ch);
	return;
    }

    if(get_curr_stat(ch,STAT_STR)*32 < victim->carry_weight)
    {
	sprintf(buf,"You run and slam into %s, but they stand firm.\r\n",victim->short_descr[0]=='\0'?victim->name:victim->short_descr);
	send_to_char(buf,ch);
	sprintf(buf,"%s tries to push you out of the room, but is too weak.\n\r",ch->name);
	send_to_char(buf,victim);
	act("$n slams into $N but fails to push $M from the room.",ch,victim,NULL,TO_NOTVICT);
	return;
    }
    else
    {
        switch(UPPER(arg2[0]))
	{
	    case 'N': direction=DIR_NORTH; break;
	    case 'S': direction=DIR_SOUTH; break;
	    case 'E': direction=DIR_EAST; break;
	    case 'W': direction=DIR_WEST; break;
	    case 'U': direction=DIR_UP; break;
	    case 'D': direction=DIR_DOWN; break;
	    default : direction=-1;
	}

	if(direction==-1)
	{
	    send_to_char("Push them where?\n\r",ch);
	    return;
	}

        if(!IS_NPC(victim))
	{

	    if((time(NULL) - victim->pk_timer > 90) && IS_SET(victim->in_room->room_flags,ROOM_SAFE))
	    {
		send_to_char("This person is safe. You may not push them at this time.\n\r",ch);
		return;
	    }

	    if(ch->in_room->exit[direction]==NULL)
	    {
	    	sprintf(buf,"You run and slam %s into a wall. Ouch, that hurt.\n\r",victim->name);
	    	send_to_char(buf,ch);
	    	sprintf(buf,"%s runs and slams you into a wall. Ouch!\n\r",ch->name);
	    	send_to_char(buf,victim);
		return;
	    }

	    if(IS_SET(ch->in_room->exit[direction]->exit_info,EX_CLOSED) && !IS_AFFECTED(victim,AFF_PASS_DOOR))
	    {
	    	sprintf(buf,"You run and slam %s into a closed door. Ouch, that hurt.\n\r",victim->name);
	    	send_to_char(buf,ch);
	    	sprintf(buf,"%s runs and slams you into a closed door. Ouch!\n\r",ch->name);
	    	send_to_char(buf,victim);
		return;
	    }
	    
	    sprintf(buf,"You run and slam into %s, pushing them from the room.\r\n",victim->name);
  	    send_to_char(buf,ch);
   	    sprintf(buf,"%s runs and slams into you, pushing you from the room.\r\n",ch->name);
	    send_to_char(buf,victim);
	    act("$n slams into $N and pushes $M from the room.",ch,NULL,victim,TO_NOTVICT);
	    char_from_room(victim);
	    char_to_room(victim, ch->in_room->exit[direction]->u1.to_room);
	    do_look(victim,"auto");
	    ch->pk_timer=time(NULL);
	}
	else
	{
	    if(ch->in_room->exit[direction]==NULL)
	    {
	    	sprintf(buf,"You run and slam %s into a wall. Ouch, that hurt.\n\r",victim->short_descr);
	    	send_to_char(buf,ch);
		return;
	    }

	    if(IS_SET(ch->in_room->exit[direction]->exit_info,EX_CLOSED) && !IS_AFFECTED(victim,AFF_PASS_DOOR))
	    {
	    	sprintf(buf,"You run and slam %s into a closed door. Ouch, that hurt.\n\r",victim->short_descr);
	    	send_to_char(buf,ch);
		return;
	    }
	    
	    sprintf(buf,"You run and slam into %s, pushing them from the room.\r\n",victim->short_descr);
  	    send_to_char(buf,ch);
	    act("$n slams into $N and pushes $M from the room.",ch,NULL,victim,TO_ROOM);
	    char_from_room(victim);
	    char_to_room(victim, ch->in_room->exit[direction]->u1.to_room);
	}
    }
}


void do_drag( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH]="\0";
    char arg1[MAX_INPUT_LENGTH]="\0";
    char arg2[MAX_INPUT_LENGTH]="\0";
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *new_room;
    int direction=-1;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if(arg1[0]=='\0')
    {
	send_to_char("Drag who what where?\n\r",ch);
	return;
    }

    if((victim = get_char_room(ch,arg1)) == NULL)
    {
	send_to_char("They aren't there.\n\r",ch);
	return;
    }

    if(victim==ch)
    {
	send_to_char("You drag yourself in vain.\n\r",ch);
	return;
    }

    if( IS_NPC(victim) && (IS_SET(victim->act,ACT_SENTINEL) || IS_SET(victim->act,ACT_TRAIN) || IS_SET(victim->act,ACT_PRACTICE) || IS_SET(victim->act,ACT_IS_HEALER) ))
    {
	send_to_char("They cannot be dragged!\n\r",ch);
	return;
    }

    if(victim->position == POS_STANDING)
    {
	sprintf(buf,"%s is standing. Try pushing them.\n\r",victim->short_descr[0]=='\0'?victim->name:victim->short_descr);
	send_to_char(buf,ch);
	return;
    }

    if(victim->position == POS_FIGHTING)
    {
	sprintf(buf,"%s is fighting. Wait until they are done.\n\r",victim->short_descr[0]=='\0'?victim->name:victim->short_descr);
	send_to_char(buf,ch);
	return;
    }

    if(IS_AFFECTED(ch, AFF_WEB))
    {
	send_to_char("You can't leave the room! You're covered in webbing!\n\r",ch);
	return;
    }

    if(IS_AFFECTED(victim, AFF_WEB))
    {
	act("The webs holding $N prevent you from dragging $M from the room.",ch,NULL,victim,TO_CHAR);
	act("The webs holding you prevent $n from dragging you from the room.",ch,NULL,victim,TO_VICT);
	return;
    }

    if(victim->level < 16 && !IS_NPC(victim))
    {
	send_to_char("You may not drag characters under the level of 16.\n\r",ch);
	return;
    }

    if(ch->level < 16 && !IS_NPC(victim))
    {
	send_to_char("Dragging a player is an aggressive action. Wait until you are level 16.\n\r",ch);
	return;
    }

    if(arg2[0]=='\0')
    {
	send_to_char("Drag which direction?\n\r",ch);
	return;
    }

    if(get_curr_stat(ch,STAT_STR)*42 < victim->carry_weight)
    {
	sprintf(buf,"You grab %s's feet and attempt to drag them out, but they are too heavy.\r\n",victim->short_descr[0]=='\0'?victim->name:victim->short_descr);
	send_to_char(buf,ch);
	sprintf(buf,"%s tries to drag you out of the room, but is too weak.\n\r",ch->name);
	send_to_char(buf,victim);
	act("$n grabs $N's feet and attempts to drag $N, but is too weak.",ch,NULL,victim,TO_NOTVICT);
	return;
    }
    else
    {
        switch(UPPER(arg2[0]))
	{
	    case 'N': direction=DIR_NORTH; break;
	    case 'S': direction=DIR_SOUTH; break;
	    case 'E': direction=DIR_EAST; break;
	    case 'W': direction=DIR_WEST; break;
	    case 'U': direction=DIR_UP; break;
	    case 'D': direction=DIR_DOWN; break;
	    default : direction=-1;
	}

	if(direction==-1)
	{
	    send_to_char("Drag them where?\n\r",ch);
	    return;
	}

        if(!IS_NPC(victim))
	{

	    if((time(NULL) - victim->pk_timer > 90) && IS_SET(victim->in_room->room_flags,ROOM_SAFE))
	    {
		send_to_char("This person is safe. You may not drag them at this time.\n\r",ch);
		return;
	    }

	    if(ch->in_room->exit[direction]==NULL)
	    {
	    	sprintf(buf,"You grab %s's feet and drag them into a wall. Ouch, that hurt.\n\r",victim->name);
	    	send_to_char(buf,ch);
	    	sprintf(buf,"%s grabs your feet and drags you into a wall. Ouch!\n\r",ch->name);
	    	send_to_char(buf,victim);
		return;
	    }

	    if(IS_SET(ch->in_room->exit[direction]->exit_info,EX_CLOSED) && !IS_AFFECTED(victim,AFF_PASS_DOOR))
	    {
	    	sprintf(buf,"You grab %s's feet and drag them into a closed door. Ouch, that hurt.\n\r",victim->name);
	    	send_to_char(buf,ch);
	    	sprintf(buf,"%s grabs your feet and drags you into a closed door. Ouch!\n\r",ch->name);
	    	send_to_char(buf,victim);
		return;
	    }
	    
	    sprintf(buf,"You grab %s's feet and drag them from the room.\r\n",victim->name);
  	    send_to_char(buf,ch);
   	    sprintf(buf,"%s grabs your feet and drags you from the room.\r\n",ch->name);
	    send_to_char(buf,victim);
	    act("$n grabs $N's feet and drags $M from the room.",ch,NULL,victim,TO_NOTVICT);
	    new_room=ch->in_room->exit[direction]->u1.to_room;
	    char_from_room(ch);
	    char_to_room(ch,new_room);
	    char_from_room(victim);
	    char_to_room(victim, new_room);
	    do_look(ch,"auto");
	    do_look(victim,"auto");
	    ch->pk_timer=time(NULL);
	}
	else
	{
	    if(ch->in_room->exit[direction]==NULL)
	    {
	    	sprintf(buf,"You grab %s's feet and drag them into a wall. Ouch, that hurt.\n\r",victim->short_descr);
	    	send_to_char(buf,ch);
		return;
	    }

	    if(IS_SET(ch->in_room->exit[direction]->exit_info,EX_CLOSED) && !IS_AFFECTED(victim,AFF_PASS_DOOR))
	    {
	    	sprintf(buf,"You grab %s's feet and drag them into a closed door. Ouch, that hurt.\n\r",victim->short_descr);
	    	send_to_char(buf,ch);
		return;
	    }
	    
	    sprintf(buf,"You grab %s's feet dragging them from the room.\r\n",victim->short_descr);
  	    send_to_char(buf,ch);
	    act("$n grabs $N's feet and drags $M from the room.",ch,NULL,victim,TO_ROOM);
	    new_room=ch->in_room->exit[direction]->u1.to_room;
	    char_from_room(ch);
	    char_to_room(ch,new_room);
	    char_from_room(victim);
	    char_to_room(victim, new_room);
	    do_look(ch,"auto");
	}
    }
}

void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Open what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'open object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_CLOSED);
	send_to_char( "Ok.\n\r", ch );
	act( "$n opens $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'open door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if (  IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* open the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
    }

    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Close what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'close object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }

	SET_BIT(obj->value[1], CONT_CLOSED);
	send_to_char( "Ok.\n\r", ch );
	act( "$n closes $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'close door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* close the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
    }

    return;
}



bool has_key_to_door( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == key )
	    return TRUE;
    }

    return FALSE;
}



void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Lock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'lock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key_to_door( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'lock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key_to_door( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* lock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    SET_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unlock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'unlock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key_to_door( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'unlock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key_to_door( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* unlock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_pick( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Pick what?\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );

    /* look for guards */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level )
	{
	    act( "$N is standing too close to the lock.",
		ch, NULL, gch, TO_CHAR );
	    return;
	}
    }

    if ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[gsn_pick_lock] )
    {
	send_to_char( "You failed.\n\r", ch);
	check_improve(ch,gsn_pick_lock,FALSE,2);
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'pick object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
	    { send_to_char( "You failed.\n\r",             ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	check_improve(ch,gsn_pick_lock,TRUE,2);
	act( "$n picks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'pick door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch))
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 && !IS_IMMORTAL(ch))
	    { send_to_char( "It can't be picked.\n\r",     ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL(ch))
	    { send_to_char( "You failed.\n\r",             ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	check_improve(ch,gsn_pick_lock,TRUE,2);

	/* pick the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}




void do_stand( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	    { send_to_char( "You can't wake up!\n\r", ch ); return; }

	send_to_char( "You wake and stand up.\n\r", ch );
	act( "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_RESTING: case POS_SITTING:
	send_to_char( "You stand up.\n\r", ch );
	act( "$n stands up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char( "You are already standing.\n\r", ch );
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}



void do_rest( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You wake up and start resting.\n\r", ch );
	act ("$n wakes up and starts resting.",ch,NULL,NULL,TO_ROOM);
	ch->position = POS_RESTING;
	break;

    case POS_RESTING:
	send_to_char( "You are already resting.\n\r", ch );
	break;

    case POS_STANDING:
	send_to_char( "You rest.\n\r", ch );
	act( "$n sits down and rests.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_RESTING;
	break;

    case POS_SITTING:
	send_to_char("You rest.\n\r",ch);
	act("$n rests.",ch,NULL,NULL,TO_ROOM);
	ch->position = POS_RESTING;
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }


    return;
}


void do_sit (CHAR_DATA *ch, char *argument )
{
    switch (ch->position)
    {
	case POS_SLEEPING:
	    send_to_char("You wake up.\n\r",ch);
	    act("$n wakes and sits up.\n\r",ch,NULL,NULL,TO_ROOM);
	    ch->position = POS_SITTING;
	    break;
 	case POS_RESTING:
	    send_to_char("You stop resting.\n\r",ch);
	    ch->position = POS_SITTING;
	    break;
	case POS_SITTING:
	    send_to_char("You are already sitting down.\n\r",ch);
	    break;
	case POS_FIGHTING:
	    send_to_char("Maybe you should finish this fight first?\n\r",ch);
	    break;
	case POS_STANDING:
	    send_to_char("You sit down.\n\r",ch);
	    act("$n sits down on the ground.\n\r",ch,NULL,NULL,TO_ROOM);
	    ch->position = POS_SITTING;
	    break;
    }
    return;
}


void do_sleep( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You are already sleeping.\n\r", ch );
	break;

    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING: 
	send_to_char( "You go to sleep.\n\r", ch );
	act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_SLEEPING;
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}



void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_stand( ch, argument ); return; }

    if ( !IS_AWAKE(ch) )
	{ send_to_char( "You are asleep yourself!\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{ send_to_char( "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
	{ act( "$N is already awake.", ch, NULL, victim, TO_CHAR ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) )
	{ act( "You can't wake $M!",   ch, NULL, victim, TO_CHAR );  return; }

    victim->position = POS_STANDING;
    act( "You wake $M.", ch, NULL, victim, TO_CHAR );
    act( "$n wakes you.", ch, NULL, victim, TO_VICT );
    return;
}



void do_sneak( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    send_to_char( "You attempt to move silently.\n\r", ch );
    affect_strip( ch, gsn_sneak );

    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_sneak] )
    {
	check_improve(ch,gsn_sneak,TRUE,3);
	af.type      = gsn_sneak;
	af.level     = ch->level; 
	af.duration  = ch->level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SNEAK;
	affect_to_char( ch, &af );
    }
    else
	check_improve(ch,gsn_sneak,FALSE,3);

    return;
}



void do_hide( CHAR_DATA *ch, char *argument )
{
    send_to_char( "You attempt to hide.\n\r", ch );

    if ( IS_AFFECTED(ch, AFF_HIDE) )
	REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_hide] )
    {
	SET_BIT(ch->affected_by, AFF_HIDE);
	check_improve(ch,gsn_hide,TRUE,3);
    }
    else
	check_improve(ch,gsn_hide,FALSE,3);

    return;
}

void do_disguise( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    send_to_char( "You attempt disguise yourself.\n\r", ch );
    affect_strip( ch, gsn_disguise );

    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_disguise] )
    {
	check_improve(ch,gsn_disguise,TRUE,3);
	af.type      = gsn_disguise;
	af.level     = ch->level; 
	af.duration  = ch->level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_DISGUISE;
	affect_to_char( ch, &af );
    }
    else
	check_improve(ch,gsn_disguise,FALSE,3);

    return;

}



/*
 * Contributed by Alander.
 */
void do_visible( CHAR_DATA *ch, char *argument )
{
    affect_strip ( ch, gsn_invis			);
    affect_strip ( ch, gsn_mass_invis			);
    affect_strip ( ch, gsn_sneak			);
    REMOVE_BIT   ( ch->affected_by, AFF_HIDE		);
    REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE	);
    REMOVE_BIT   ( ch->affected_by, AFF_SNEAK		);
    REMOVE_BIT	 ( ch->affected_by, AFF_DISGUISE	);
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_home( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;

    if (IS_NPC(ch) && !IS_SET(ch->act,ACT_PET))
    {
	send_to_char("Only players can go home.\n\r",ch);
	return;
    }
  
    act( "$n clicks his heels together three times and disappears.", ch, 0, 0, TO_ROOM );

    if ( ( location = get_room_index( ROOM_VNUM_THOUGHT ) ) == NULL )
    {
	send_to_char( "You are completely lost.\n\r", ch );
	return;
    }

 if ( ch->in_room == location )
	return;

    if ( ( victim = ch->fighting ) != NULL )
    {
	int lose;

/*	if (IS_NPC(ch))
	    skill = 40 + ch->level;
	else
	    skill = ch->pcdata->learned[gsn_home];
*/
	lose = (ch->desc != NULL) ? 500 : 50;
	gain_exp( ch, 0 - lose );
	check_improve(ch,gsn_home,TRUE,4);
	sprintf( buf, "You go home during combat!  You lose %d exps.\n\r", lose );
	send_to_char( buf, ch );
	stop_fighting( ch, TRUE );
	
    }

    ch->move /= 4;
    ch->mana /= 4;
    ch->hit /= 4;
//    act( "$n disappears.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );
    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    
    if (ch->pet != NULL)
	do_home(ch->pet,"");

    return;
}


void do_recall( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;

    if (IS_NPC(ch) && !IS_SET(ch->act,ACT_PET))
    {
	send_to_char("Only players can recall.\n\r",ch);
	return;
    }

    if (ch->level > 15)
    {
	send_to_char("Aren't you a little old for that?\n\r",ch);
	return;
    }
  
    act( "$n prays for transportation!", ch, 0, 0, TO_ROOM );

    if ( ( location = get_room_index( ROOM_VNUM_TEMPLE ) ) == NULL )
    {
	send_to_char( "You are completely lost.\n\r", ch );
	return;
    }

    if ( ch->in_room == location )
	return;

    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_AFFECTED(ch, AFF_CURSE) )
    {
	send_to_char( "The gods have forsaken you.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) != NULL )
    {
	int lose,skill;

	if (IS_NPC(ch))
	    skill = 40 + ch->level;
	else
	    skill = ch->pcdata->learned[gsn_recall];

	if ( number_percent() < 80 * skill / 100 )
	{
	    check_improve(ch,gsn_recall,FALSE,6);
	    WAIT_STATE( ch, 4 );
	    sprintf( buf, "You failed!.\n\r");
	    send_to_char( buf, ch );
	    return;
	}

	lose = (ch->desc != NULL) ? 25 : 50;
	gain_exp( ch, 0 - lose );
	check_improve(ch,gsn_recall,TRUE,4);
	sprintf( buf, "You recall from combat!  You lose %d exps.\n\r", lose );
	send_to_char( buf, ch );
	stop_fighting( ch, TRUE );
	
    }

    ch->move /= 2;
    act( "$n disappears.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );
    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    
    if (ch->pet != NULL)
	do_recall(ch->pet,"");

    return;
}



void do_train( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mob;
    sh_int stat = - 1;
    char *pOutput = NULL;
    int cost;

    if ( IS_NPC(ch) )
	return;

    /*
     * Check for trainer.
     */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
	if ( IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN) )
	    break;
    }

    if ( mob == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "You have %d training sessions.\n\r", ch->train );
	send_to_char( buf, ch );
	argument = "foo";
    }

    cost = 1;

    if ( !str_cmp( argument, "str" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_STR )
	    cost    = 1;
	stat        = STAT_STR;
	pOutput     = "strength";
    }

    else if ( !str_cmp( argument, "int" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_INT )
	    cost    = 1;
	stat	    = STAT_INT;
	pOutput     = "intelligence";
    }

    else if ( !str_cmp( argument, "wis" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_WIS )
	    cost    = 1;
	stat	    = STAT_WIS;
	pOutput     = "wisdom";
    }

    else if ( !str_cmp( argument, "dex" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_DEX )
	    cost    = 1;
	stat  	    = STAT_DEX;
	pOutput     = "dexterity";
    }

    else if ( !str_cmp( argument, "con" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_CON )
	    cost    = 1;
	stat	    = STAT_CON;
	pOutput     = "constitution";
    }

    else if ( !str_cmp(argument, "hp" ) )
	cost = 1;

    else if ( !str_cmp(argument, "mana" ) )
	cost = 1;

    else
    {
	strcpy( buf, "You can train:" );
	if ( ch->perm_stat[STAT_STR] < get_max_train(ch,STAT_STR)) 
	    strcat( buf, " str" );
	if ( ch->perm_stat[STAT_INT] < get_max_train(ch,STAT_INT))  
	    strcat( buf, " int" );
	if ( ch->perm_stat[STAT_WIS] < get_max_train(ch,STAT_WIS)) 
	    strcat( buf, " wis" );
	if ( ch->perm_stat[STAT_DEX] < get_max_train(ch,STAT_DEX))  
	    strcat( buf, " dex" );
	if ( ch->perm_stat[STAT_CON] < get_max_train(ch,STAT_CON))  
	    strcat( buf, " con" );
	strcat( buf, " hp mana");

	if ( buf[strlen(buf)-1] != ':' )
	{
	    strcat( buf, ".\n\r" );
	    send_to_char( buf, ch );
	}
	else
	{
	    /*
	     * This message dedicated to Jordan ... you big stud!
	     */
	    act( "You have nothing left to train, you $T!",
		ch, NULL,
		ch->sex == SEX_MALE   ? "big stud" :
		ch->sex == SEX_FEMALE ? "hot babe" :
					"wild thing",
		TO_CHAR );
	}

	return;
    }

    if (!str_cmp("hp",argument))
    {
    	if ( cost > ch->train )
    	{
       	    send_to_char( "You don't have enough training sessions.\n\r", ch );
            return;
        }
 
	ch->train -= cost;
        ch->perm_hit += 10;
        ch->max_hit += 10;
        ch->hit +=10;
        act( "Your durability increases!",ch,NULL,NULL,TO_CHAR);
        act( "$n's durability increases!",ch,NULL,NULL,TO_ROOM);
        return;
    }
 
    if (!str_cmp("mana",argument))
    {
        if ( cost > ch->train )
        {
            send_to_char( "You don't have enough training sessions.\n\r", ch );
            return;
        }

	ch->train -= cost;
        ch->perm_mana += 10;
        ch->max_mana += 10;
        ch->mana += 10;
        act( "Your power increases!",ch,NULL,NULL,TO_CHAR);
        act( "$n's power increases!",ch,NULL,NULL,TO_ROOM);
        return;
    }

    if ( ch->perm_stat[stat]  >= get_max_train(ch,stat) )
    {
	act( "Your $T is already at maximum.", ch, NULL, pOutput, TO_CHAR );
	return;
    }

    if ( cost > ch->train )
    {
	send_to_char( "You don't have enough training sessions.\n\r", ch );
	return;
    }

    ch->train		-= cost;
  
    ch->perm_stat[stat]		+= 1;
    act( "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
    act( "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
    return;
}

void do_breath(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if(ch->race!=race_lookup("draconian"))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }
    
    if(argument[0]=='\0' && ch->fighting==NULL)
    {
	send_to_char("Breathe what where?\n\r",ch);
	return;
    }
    
    victim=NULL;

    if(argument[0]=='\0')
	victim=ch->fighting;
    else 
	victim=get_char_room(ch,argument);

    if(victim==NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    switch (ch->dracspell)
    {
	case DRAC_FROST:
		act("$n opens $s jaws and spits a cone of frost at $N!",ch,NULL,victim,TO_NOTVICT);
		act("You open your jaws and spit a cone of frost at $N!",ch,NULL,victim,TO_CHAR);
		act("$n opens $s jaws and spits a cone of frost at you!",ch,NULL,victim,TO_VICT);
		sprintf(buf,"'frost breath' %s",victim->name);
		do_cast(ch,buf);
		break;
	case DRAC_FIRE:
		act("$n opens $s jaws and spits a large ball of fire at $N!",ch,NULL,victim,TO_NOTVICT);
		act("You open your jaws and spit a large ball of fire at $N!",ch,NULL,victim,TO_CHAR);
		act("$n opens $s jaws and spits a large ball of fire at you!",ch,NULL,victim,TO_VICT);
		sprintf(buf,"'fire breath' %s",victim->name);
		do_cast(ch,buf);
		break;
	case DRAC_LIGHTNING:
		act("$n opens $s jaws and spits a lightning bolt at $N!",ch,NULL,victim,TO_NOTVICT);
		act("You open your jaws and spit a lightning bolt at $N!",ch,NULL,victim,TO_CHAR);
		act("$n opens $s jaws and spits a lightning bolt at you!",ch,NULL,victim,TO_VICT);
		sprintf(buf,"'lightning breath' %s",victim->name);
		do_cast(ch,buf);
		break;
	case DRAC_ACID:
		act("$n opens $s jaws and spits a stream of acid at $N!",ch,NULL,victim,TO_NOTVICT);
		act("You open your jaws and spit a stream of acid at $N!",ch,NULL,victim,TO_CHAR);
		act("$n opens $s jaws and spits a stream of acid at you!",ch,NULL,victim,TO_VICT);
		sprintf(buf,"'acid breath' %s",victim->name);
		do_cast(ch,buf);
		break;
	case DRAC_GAS:
		act("$n opens $s jaws and releases a cloud of noxious gas at $N!",ch,NULL,victim,TO_NOTVICT);
		act("You open your jaws and releases a cloud of noxious gas at $N!",ch,NULL,victim,TO_CHAR);
		act("$n opens $s jaws and releases a cloud of noxious gas at you!",ch,NULL,victim,TO_VICT);
		sprintf(buf,"'gas breath' %s",victim->name);
		do_cast(ch,buf);
		break;
	}

}

/* command procedures needed */
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_stand		);

/* random room generation procedure */
ROOM_INDEX_DATA  *get_random_room(CHAR_DATA *ch)
{
    ROOM_INDEX_DATA *room;

    for ( ; ; )
    {
        room = get_room_index( number_range( 0, 65535 ) );
        if ( room != NULL )
        if ( can_see_room(ch,room)
	&&   !room_is_private(room)
        &&   !IS_SET(room->room_flags, ROOM_PRIVATE)
        &&   !IS_SET(room->room_flags, ROOM_SOLITARY) 
	&&   !IS_SET(room->room_flags, ROOM_SAFE)) 
            break;
    }

    return room;
}
