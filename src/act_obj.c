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
DECLARE_DO_FUN(do_split		);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_say		);



/*
 * Local functions.
 */
#define CD CHAR_DATA
bool	remove_obj	args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
CD *	find_keeper	args( ( CHAR_DATA *ch ) );
int	get_cost	args( ( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) );
#undef	CD

/* RT part of the corpse looting code */

bool can_loot(CHAR_DATA *ch, OBJ_DATA *obj)
{
    CHAR_DATA *owner, *wch;

    if (IS_IMMORTAL(ch))
	return TRUE;

    if (!obj->owner || obj->owner == NULL)
	return TRUE;

    owner = NULL;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
        if (!str_cmp(wch->name,obj->owner))
            owner = wch;

    if (owner == NULL)
	return TRUE;

    if(ch->level > 15)
	return TRUE;
    else if(!str_cmp(ch->name,owner->name))
	return TRUE;

/*    if (!IS_NPC(owner) && IS_SET(owner->act,PLR_CANLOOT))
	return TRUE;

    if (is_same_group(ch,owner))
	return TRUE;
*/
    return FALSE;
}

void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];

    if ( !CAN_WEAR(obj, ITEM_TAKE) )
    {
	send_to_char( "You can't take that.\n\r", ch );
	return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	act( "$d: you can't carry that many items.",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }


    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
	act( "$d: you can't carry that much weight.",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if (!can_loot(ch,obj))
    {
	act("Corpse looting is not permitted.",ch,NULL,NULL,TO_CHAR );
	return;
    }

    if ( container != NULL )
    {
    	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  get_trust(ch) < obj->level)
	{
	    send_to_char("You are not powerful enough to use it.\n\r",ch);
	    return;
	}

    	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  !CAN_WEAR(container, ITEM_TAKE) && obj->timer)
	    obj->timer = 0;	
	act( "You get $p from $P.", ch, obj, container, TO_CHAR );
	act( "$n gets $p from $P.", ch, obj, container, TO_ROOM );
	obj_from_obj( obj );
    }
    else
    {
	act( "You get $p.", ch, obj, container, TO_CHAR );
	act( "$n gets $p.", ch, obj, container, TO_ROOM );
	obj_from_room( obj );
    }

    if ( obj->item_type == ITEM_MONEY)
    {
	ch->gold += obj->value[0];
        if (IS_SET(ch->act,PLR_AUTOSPLIT))
        { /* AUTOSPLIT code */
    	  members = 0;
    	  for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    	  {
            if ( is_same_group( gch, ch ) )
              members++;
    	  }

	  if ( members > 1 && obj->value[0] > 1)
	  {
	    sprintf(buffer,"%d",obj->value[0]);
	    do_split(ch,buffer);	
	  }
        }
 
	extract_obj( obj );
    }
    else
    {
	obj_to_char( obj, ch );
    }

    return;
}

void do_storage( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *hole = NULL;
    char buf[MAX_STRING_LENGTH];
    
    if(IS_NPC(ch))	//Mobiles don't have lockers
	return;


    /* just a point of info here. str_cmp is case insensitive, so capitalize
	here just  burns cycles. */

    if(!str_cmp(capitalize(argument),"Close") || ch->pcdata->locker!=NULL)
    {
	if(ch->pcdata->locker==NULL)
	    return;
	obj_from_room(ch->pcdata->locker);
    	save_char_locker(ch);
	act("$n's multi-dimensional pocket folds in upon itself.",ch,ch->pcdata->locker,NULL,TO_ROOM);
	act("Your multi-dimensional pocket folds in upon itself.",ch,ch->pcdata->locker,NULL,TO_CHAR);
	extract_obj(ch->pcdata->locker);
	ch->pcdata->locker=NULL;
    }
    else
    {
    	if(time(NULL) - ch->pk_timer < 90)
        {
	    send_to_char("Wait until you are safe.\n\r",ch);
	    return;
   	}

	if(ch->pcdata->locker!=NULL)
	    return;

    	if(load_locker(ch)==FALSE)
	{
/* ADDED Arawn

   this safeguards against the possiblity that the object is not in the database
   and protects versus extra crashes.

 */
	    OBJ_INDEX_DATA *pIndex;

	    pIndex = get_obj_index(OBJ_VNUM_LOCKER);
	    if (!pIndex)
	    {
		send_to_char("There is no such item.\r\n",ch);
		return;
	    }

	    hole = create_object(pIndex, 100);
	
/*** End Add ***/

	    obj_to_room(hole, ch->in_room);
	    free_string(hole->owner);
	    hole->owner = str_dup(ch->name);

/**** ADDED Arawn *****/

	    if (!is_name(ch->name,hole->name))
	    {
/**** END ADD ****/

	    sprintf(buf,"%s %s",hole->name, ch->name);
	    free_string(hole->name);
	    hole->name = str_dup(buf);


	/* these lines moved inside the if statement, because there won't 
	   be  format specifier after the first time this is run. */
	    sprintf(buf,hole->description,ch->name);
	    free_string(hole->description);
	    hole->description = str_dup(buf);

/**** ADDED Arawn *****/
	    
	    }
/**** End Add ***/


	    ch->pcdata->locker = hole;
	}

	hole = ch->pcdata->locker;


	act("$n's multi-dimensional pocket appears and unfolds.",ch,hole,NULL,TO_ROOM);
	act("Your multi-dimensional pocket appears and unfolds.",ch,hole,NULL,TO_CHAR);
    }
}

void do_repair( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *worker;
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    long cost=0;
    bool examine=FALSE;

    if(argument[0]=='\0')
    {
	send_to_char("Repair what?\n\r",ch);
	return;
    }

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);

    if( !str_cmp(arg,"e") ||
	!str_cmp(arg,"ex") ||
	!str_cmp(arg,"exa") ||
	!str_cmp(arg,"exam") ||
	!str_cmp(arg,"exami") ||
	!str_cmp(arg,"examin") ||
	!str_cmp(arg,"examine") )
    {
	examine=TRUE;
    	if((obj=get_obj_carry(ch,arg2))==NULL)
    	{
	    send_to_char("You can't seem to find it.\n\r",ch);
	    return;
    	}
    }
    else
    {
    	if((obj=get_obj_carry(ch,arg))==NULL)
    	{
	    send_to_char("You can't seem to find it.\n\r",ch);
	    return;
    	}
    }

    for(worker = ch->in_room->people; worker!=NULL; worker = worker->next_in_room)
    {
	if(!IS_NPC(worker))
	    continue;

	if(ch == worker)
	    continue;

	if(!IS_SET(worker->act, ACT_REPAIRMAN))
	    continue;

        /*We now have our mob*/
	if(obj->pIndexData->new_format)
            cost = obj->pIndexData->cost*2;
	else
            obj->cost = number_fuzzy( 10 ) * number_fuzzy( obj->level ) * number_fuzzy( obj->level )*2;


        if(examine)
        {
	    sprintf(buf,"It will cost you %ld gold coins for your %s to be repaired.",cost,obj->short_descr);
	    do_say(worker,buf);
	    return;
        }	    	

	if(ch->gold<cost)
	{
	    sprintf(buf,"It will cost you %ld gold coins for your %s to be repaired.",cost,obj->short_descr);
	    do_say(worker,buf);
	    do_say(worker,"You don't seem to have enough.");
	    return;
        }

        if(obj->value[0]==obj->pIndexData->value[0]
	   && obj->value[1]==obj->pIndexData->value[1]
	   && obj->value[2]==obj->pIndexData->value[2]
	   && obj->value[3]==obj->pIndexData->value[3]
	   && obj->value[4]==obj->pIndexData->value[4])
	{
	    do_say(worker,"That object doesn't appear to need repairing.");
	    return;
        }

	act("$N tinkers with $p and restores it to its original condition.",ch,obj,worker,TO_ROOM);
	act("$N tinkers with $p and restores it to its original condition.",ch,obj,worker,TO_CHAR);
	act("$N gives $p to $n.",ch,obj,worker,TO_ROOM);
	sprintf(buf,"$N gives $p to you and accepts %ld gold coins.",cost);
	act(buf,ch,obj,worker,TO_CHAR);
	ch->gold-=cost;
	obj->value[0]=obj->pIndexData->value[0];
	obj->value[1]=obj->pIndexData->value[1];
	obj->value[2]=obj->pIndexData->value[2];
	obj->value[3]=obj->pIndexData->value[3];
	obj->value[4]=obj->pIndexData->value[4];
	obj->cost=cost/2;
	obj->condition=obj->pIndexData->condition;
	return;
    }

    send_to_char("Nobody in the room appears to know how to fix that.\n\r",ch);
}

void do_settrap( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;
    OBJ_DATA *obj2 = NULL;
    OBJ_DATA *testobj = NULL;
    char test;
    bool okay=TRUE;


    if(get_skill(ch,skill_lookup("settrap"))<1)
    {
	send_to_char("Huh?\n\r",ch);
        return;
    }


    test = UPPER(argument[0]);

    if(argument[0]=='\0' || (test!='N' && test!='E' && test!='S' &&
       test!='W' && test!='U' && test!='D'))
    {
        send_to_char("Set the trap where?\n\r",ch);
        return;
    }

    switch(test)
    {
        case 'N': if(ch->in_room->exit[0]==NULL) okay=FALSE; break;
        case 'E': if(ch->in_room->exit[1]==NULL) okay=FALSE; break;
        case 'S': if(ch->in_room->exit[2]==NULL) okay=FALSE; break;
        case 'W': if(ch->in_room->exit[3]==NULL) okay=FALSE; break;
        case 'U': if(ch->in_room->exit[4]==NULL) okay=FALSE; break;
        case 'D': if(ch->in_room->exit[5]==NULL) okay=FALSE; break;
    }

    if(okay==FALSE)
    {
        send_to_char("There is no exit in that direction.\n\r",ch);
        return;
    }

    for( testobj=ch->in_room->contents; testobj!=NULL; testobj=testobj->next_content)
    {
        switch(test)
        {
            case 'N': if(testobj->value[0]==0 && testobj->pIndexData->vnum==OBJ_VNUM_TRAP)
		      {
			send_to_char("There is already a trap to the north.\n\r",ch);
			return;
		      }
			break;
            case 'E': if(testobj->value[0]==1 && testobj->pIndexData->vnum==OBJ_VNUM_TRAP)
		      {
			send_to_char("There is already a trap to the east.\n\r",ch);
			return;
		      }
			break;
            case 'S': if(testobj->value[0]==2 && testobj->pIndexData->vnum==OBJ_VNUM_TRAP)
		      {
			send_to_char("There is already a trap to the south.\n\r",ch);
			return;
		      }
			break;
            case 'W': if(testobj->value[0]==3 && testobj->pIndexData->vnum==OBJ_VNUM_TRAP)
		      {
			send_to_char("There is already a trap to the west.\n\r",ch);
			return;
		      }
			break;
            case 'U': if(testobj->value[0]==4 && testobj->pIndexData->vnum==OBJ_VNUM_TRAP)
		      {
			send_to_char("There is already a trap above you.\n\r",ch);
			return;
		      }
			break;
            case 'D': if(testobj->value[0]==5 && testobj->pIndexData->vnum==OBJ_VNUM_TRAP)
		      {
			send_to_char("There is already a trap below you.\n\r",ch);
			return;
		      }
			break;
        }
    }	

    obj = create_object(get_obj_index(50),0);
    obj->timer = 10;
    obj->level = ch->level;
    obj->value[1]=DAM_PIERCE;
    obj->value[2]=skill_lookup("settrap");
    free_string(obj->name);
    obj->name = str_dup(ch->name);

    obj2 = create_object(obj->pIndexData,0);
    clone_object(obj, obj2);
    free_string(obj->description);
    free_string(obj2->description);

    if(UPPER(argument[0]) == 'N')
    {
        obj->description = str_dup("(^BKLethal^NW) A lethal looking trap has been set to the north.");
        obj->value[0]=0;
        obj2->description = str_dup("(^BKLethal^NW) A lethal looking trap has been set to the south.");
        obj2->value[0]=2;
        send_to_char("You set a trap to the north.\n\r",ch);
        act("$n sets a trap to the north.",ch,NULL,NULL,TO_ROOM);
    }
    else if(UPPER(argument[0]) == 'E')
    {
        obj->description = str_dup("(^BKLethal^NW) A lethal looking trap has been set to the east.");
        obj->value[0]=1;
        obj2->description = str_dup("(^BKLethal^NW) A lethal looking trap has been set to the west.");
        obj2->value[0]=3;
        send_to_char("You set a trap to the east.\n\r",ch);
        act("$n sets a trap to the east.",ch,NULL,NULL,TO_ROOM);
    }
    else if(UPPER(argument[0]) == 'S')
    {
        obj->description = str_dup("(^BKLethal^NW) A lethal looking trap has been set to the south.");
        obj->value[0]=2;
        obj2->description = str_dup("(^BKLethal^NW) A lethal looking trap has been set to the north.");
        obj2->value[0]=0;
        send_to_char("You set a trap to the south.\n\r",ch);
        act("$n sets a trap to the south.",ch,NULL,NULL,TO_ROOM);
    }
    else if(UPPER(argument[0]) == 'W')
    {
        obj->description = str_dup("(^BKLethal^NW) A lethal looking trap has been set to the west.");
        obj->value[0]=3;
        obj2->description = str_dup("(^BKLethal^NW) A lethal looking trap has been set to the east.");
        obj2->value[0]=1;
        send_to_char("You set a trap to the west.\n\r",ch);
        act("$n sets a trap to the west.",ch,NULL,NULL,TO_ROOM);
    }
    else if(UPPER(argument[0]) == 'U')
    {
        obj->description = str_dup("(^BKLethal^NW) A lethal looking trap has been set at the exit leading up.");
        obj->value[0]=4;
        obj2->description = str_dup("(^BKLethal^NW) A lethal looking trap has been set at the exit leading down.");
        obj2->value[0]=5;
        send_to_char("You set a trap on the exit leading up.\n\r",ch);
        act("$n sets a trap on the exit leading up.",ch,NULL,NULL,TO_ROOM);
    }
    else if(UPPER(argument[0]) == 'D')
    {
        obj->description = str_dup("(^BKLethal^NW) A lethal looking trap has been set at the exit leading down.");
        obj->value[0]=5;
        obj2->description = str_dup("(^BKLethal^NW) A lethal looking trap has been set at the exit leading up.");
        obj2->value[0]=4;
        send_to_char("You set a trap on the exit leading down.\n\r",ch);
        act("$n sets a trap on the exit leading up.",ch,NULL,NULL,TO_ROOM);
    }

    obj_to_room(obj2, ch->in_room->exit[obj->value[0]]->u1.to_room);
    obj_to_room(obj, ch->in_room);
}

void do_taste( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH];

    if(ch->race != race_lookup("orc"))
    {
	send_to_char("Huh?",ch);
	return;
    }

    argument = one_argument(argument, arg);

    if((obj = get_obj_carry(ch, arg)) == NULL)
    {
	send_to_char("You don't have that item.\n\r",ch);
	return;
    }

    if(obj->item_type!=ITEM_FOOD)
    {
	send_to_char("It wouldn't do you any good to taste that.\n\r",ch);
	return;
    }    

    if(obj->value[3]!=0)
	act("You nibble on $p and taste a trace of poison.",ch,obj,NULL,TO_CHAR);
    else
	act("You nibble on $p and don't discover any poison.",ch,obj,NULL,TO_CHAR);
    
    act("$n nibbles on $p.",ch,obj,NULL,TO_ROOM);
}

void do_sharpen( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int fail = 0;
    int result;

    argument = one_argument(argument, arg);

    if((obj = get_obj_carry(ch, arg)) == NULL)
    {
	send_to_char("You don't have that item.\n\r",ch);
	return;
    }

    if(IS_NPC(ch))
	return;

    if(obj->item_type!=ITEM_WEAPON)
    {
	send_to_char("You can only sharpen weapons.\n\r",ch);
	return;
    }    

    if(obj->wear_loc != -1)
    {
	send_to_char("The item must be carried to be sharpened.\n\r",ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_SHARP))
    {
	send_to_char("You can't make it any sharper.\n\r",ch);
	return;
    }

    fail = 25; /* fail 25% of the time no matter what */
    
    if (IS_WEAPON_STAT(obj,WEAPON_VORPAL))
        fail = 80;
    if (IS_WEAPON_STAT(obj,WEAPON_FROST))
        fail = 90;
    if (IS_WEAPON_STAT(obj,WEAPON_FLAMING))
        fail = 95;
    if (IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC))
	fail = 95;

    fail = 100 - get_skill(ch,skill_lookup("sharpen")) + (obj->level - ch->level)*2;

/*    if (obj->level <= ch->level / 2)
	fail -= 10;
    if (obj->level <= ch->level-10)
	fail -= 10;
    if (obj->level <= ch->level)
	fail -= 5;
*/

    if ((obj->value[0] != 1) && (obj->value[0] != 2))
	{
	send_to_char("You can only sharpen pointy weapons.\n\r",ch);
	return;
	}
    if ((ch->level > MAX_LEVEL - 9) && (ch->level < MAX_LEVEL - 1))
	{
	send_to_char("Sorry, sharpen isn't a skill for imms.\n\r",ch);
	return;
	}   

	fail = URANGE(5,fail,95);
	result = number_percent();
	
	if (result < (fail / 5))
	{
	act("$p breaks off and leaves you with a useless handle!",ch,obj,NULL,TO_CHAR);
	act("$p breaks off and leaves $n with a useless handle!",ch,obj,NULL,TO_ROOM);
	obj->value[1] = 0;
	obj->value[2] = 0;
	return;
	}
	if (result < (fail / 2))
	{
	send_to_char("Ack! You've made it duller!\n\r",ch);
	act("$n tries to sharpen $s $p, but makes it duller.",ch,obj,NULL,TO_ROOM);  
	obj->value[1] /= 2;
	obj->value[2] /= 2;
	return;
	}

    (obj->value[4] += WEAPON_SHARP);
	act("You make $p nice and sharp.",ch,obj,NULL,TO_CHAR);
	check_improve(ch,gsn_sharpen,TRUE,8);
	return;

	

}

void do_bank( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *banker;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int i;

    for(banker=ch->in_room->people;banker!=NULL;banker=banker->next_in_room)
    {
	if(banker==ch) continue;
        if(!can_see(ch,banker)) continue;
	if(!IS_NPC(banker)) continue;
        if(!IS_SET(banker->act,ACT_BANKER)) continue;
	
	break;
    }

    if(banker==NULL)
    {
	send_to_char("There is no banker here to help you.\n\r",ch);
	return;
    }

    argument = one_argument(argument,arg);

    for(i=0;i<strlen(arg);i++)
	arg[i]=UPPER(arg[i]);

    if(IS_NPC(ch))
    {
	do_say(banker,"I'm sorry, we currently do not offer accounts for mobiles.");
	return;
    }

    if(!str_prefix(arg,"BALANCE"))
    {
	sprintf(buf,"Hello %s, you currently have %ld gold in the bank.",ch->name,ch->balance);
	do_say(banker,buf);
	return;
    }

    if(!str_prefix(arg,"DEPOSIT"))
    {
	argument = one_argument(argument, arg);
	if(atol(arg)<=0)
	{
	    sprintf(buf,"I'm sorry %s, but I don't accept sums of such small denominations.",ch->name);
	    do_say(banker,buf);
	    return;
 	}
	if(atol(arg) > ch->gold)
	{
	    sprintf(buf,"I'm sorry %s, but it appears you don't have that much gold on you.",ch->name);
	    do_say(banker,buf);
	    return;
	}
	else
	{
	    sprintf(buf,"Thank you %s. Your deposit of %ld gold coins will be wisely invested.",ch->name,atol(arg));
	    do_say(banker,buf);
	    ch->balance += atol(arg);
	    ch->gold -= atol(arg);
	    return;
	}
    }

    if(!str_prefix(arg,"WITHDRAW"))
    {
	argument = one_argument(argument, arg);
	if(atol(arg) > ch->balance)
	{
	    sprintf(buf,"I'm sorry %s, but you don't have that much money in your account with us.",ch->name);
	    do_say(banker,buf);
	    return;
	}
	if(atoi(arg) <= 0)
	{
	    sprintf(buf,"I'm sure your teachers were very exasperated with you %s when you were a child. Please only withdraw positive integers.",ch->name);
	    do_say(banker,buf);
	    return;
	}
	if(atoi(arg) <= ch->balance)
	{
	    sprintf(buf,"Thank you for choosing Midgaard Savings and Loan. Have a nice day %s.",ch->name);
	    do_say(banker, buf);
	    ch->gold += atol(arg);
	    ch->balance -= atol(arg);
	    return;
	}
    }
    do_say(banker,"Pardon me?");
}

void do_donate( CHAR_DATA *ch, char *argument ) {
    char buf[MAX_STRING_LENGTH]="\0";
    char arg1[MAX_STRING_LENGTH]="\0";
    char arg2[MAX_STRING_LENGTH]="\0";
    static int don_room=1234;
    int reward=0;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);
   
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Donate what?\n\r", ch );
	return;
    }

    if ( IS_SET(ch->act,PLR_KILLER) )
    {
	send_to_char("The gods will not accept donations from killers.\n\r",ch);
	return;
    }

    if ( arg2[0] == '\0' )
    {
	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj' */
	    obj = get_obj_carry( ch, arg1);
	    if ( obj == NULL )
	    {
		send_to_char("You don't have that.\n\r",ch);
		return;
	    }

	    if(obj->level <= 0 || obj->value <= 0)
	    {
		send_to_char("Donate something useful.\n\r",ch);
		return;
	    }

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}


	    act("You donate $P.",ch,NULL,obj,TO_CHAR);
	    if(obj->donated==FALSE)
	    {
	    	reward=obj->level*2;
	    	ch->gold+=reward;
	    	sprintf(buf,"Tybalt rewards you for your generosity with %d gold coins.\n\r",reward);
	    	send_to_char(buf,ch);
		obj->donated=TRUE;
	    }
	    obj_from_char(obj);
	    obj_to_room(obj,get_room_index(don_room));
	    act("A bright flash of light leaves a $P in your midst.",get_room_index(don_room)->people,NULL,obj,TO_ROOM);
	    act("A bright flash of light leaves a $P in your midst.",get_room_index(don_room)->people,NULL,obj,TO_CHAR);
	    if(don_room==1234)
		don_room=9610;
	    else don_room=1234;
	}
	else
	{
	    /* 'get all' or 'get all.obj' */
	    found = FALSE;
	    reward=0;
	    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) 
		&& obj->wear_loc==WEAR_NONE )
		{
		    found = TRUE;
		    act("You donate $P.",ch,NULL,obj,TO_CHAR);
		    if(obj->donated==FALSE)
		    {
			reward+=obj->level*2;
			obj->donated=TRUE;
		    }
	    if(obj->level <= 0 || obj->value <= 0)
	    {
		send_to_char("Donate something useful.\n\r",ch);
		continue;
	    }

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    continue;
	}


		    obj_from_char(obj);
		    obj_to_room(obj,get_room_index(don_room));
		    act("A bright flash of light leaves a $P in your midst.",get_room_index(don_room)->people,NULL,obj,TO_ROOM);
		    act("A bright flash of light leaves a $P in your midst.",get_room_index(don_room)->people,NULL,obj,TO_CHAR);
		    if(don_room==1234)
			don_room=9610;
		    else don_room=1234;

		}
	    }

	    if( reward>0 )
	    {
		ch->gold+=reward;
		sprintf(buf,"Tybalt rewards you for your generosity with %d gold coins.\n\r",reward);
		send_to_char(buf,ch);
	    }

	    if ( !found ) 
	    {
		if ( arg1[3] == '\0' )
		    send_to_char( "You don't see that in your inventory.\n\r", ch );
		else
		    act( "You don't see that in your inventory.", ch, NULL, NULL, TO_CHAR );
	    }
	}
    }
}



void do_get( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    bool found;
	int count = 0;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"from"))
	argument = one_argument(argument,arg2);

    /* Get type. */
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Get what?\n\r", ch );
	return;
    }

    if ( IS_SET(ch->act,PLR_KILLER) )
    {
	send_to_char("The gods do not allow you to pick things up if you are a killer.\n\r",ch);
	return;
    }

    if ( arg2[0] == '\0' )
    {
	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj' */
	    obj = get_obj_list( ch, arg1, ch->in_room->contents );
	    if ( obj == NULL )
	    {
		act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
		return;
	    }

	    get_obj( ch, obj, NULL );
	}
	else
	{
	    /* 'get all' or 'get all.obj' */
	    found = FALSE;
	    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		count +=1;
	if (count >=11)
		{
	send_to_char("Your hands get cramped from trying to take too much at once.\n\r",ch);
	return;
	}
		    get_obj( ch, obj, NULL );
		}
	    }

	    if ( !found ) 
	    {
		if ( arg1[3] == '\0' )
		    send_to_char( "I see nothing here.\n\r", ch );
		else
		    act( "I see no $T here.", ch, NULL, &arg1[4], TO_CHAR );
	    }
	}
    }
    else
    {
	/* 'get ... container' */
	if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}

	if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	    return;
	}

	switch ( container->item_type )
	{
	default:
	    send_to_char( "That's not a container.\n\r", ch );
	    return;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	    break;

	case ITEM_CORPSE_PC:
	    {

		if (!can_loot(ch,container))
		{
		    send_to_char( "You can't do that.\n\r", ch );
		    return;
		}
	    }
	}

	if( container->pIndexData->vnum==OBJ_VNUM_LOCKER && !IS_NPC(ch) && ch->pcdata->locker != container)
	{
	    send_to_char("You can't do that.\n\r",ch);
	    return;
        }

	if ( IS_SET(container->value[1], CONT_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	    return;
	}

	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj container' */
	    obj = get_obj_list( ch, arg1, container->contains );
	    if ( obj == NULL )
	    {
		act( "I see nothing like that in the $T.",
		    ch, NULL, arg2, TO_CHAR );
		return;
	    }
	    get_obj( ch, obj, container );
	}
	else
	{
	    /* 'get all container' or 'get all.obj container' */
	    found = FALSE;
	    for ( obj = container->contains; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    if (container->pIndexData->vnum == OBJ_VNUM_PIT
		    &&  !IS_IMMORTAL(ch))
		    {
			send_to_char("Don't be so greedy!\n\r",ch);
			return;
		    }
		count +=1;
	if (count >=11)
		{
	send_to_char("Your hands get cramped from trying to take too much at once.\n\r",ch);
	return;
	}

		    get_obj( ch, obj, container );
		}
	    }

	    if ( !found )
	    {
		if ( arg1[3] == '\0' )
		    act( "I see nothing in the $T.",
			ch, NULL, arg2, TO_CHAR );
		else
		    act( "I see nothing like that in the $T.",
			ch, NULL, arg2, TO_CHAR );
	    }
	}
    }

    return;
}



void do_put( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *container;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"in"))
	argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Put what in what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return;
    }

    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
    {
	act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	return;
    }

    if ( container->item_type != ITEM_CONTAINER )
    {
	send_to_char( "That's not a container.\n\r", ch );
	return;
    }

    if ( IS_SET(container->value[1], CONT_CLOSED) )
    {
	act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	return;
    }

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
	/* 'put obj container' */
	if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( obj == container )
	{
	    send_to_char( "You can't fold it into itself.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

       if ( IS_SET(ch->act,PLR_KILLER) )
       {
    	    send_to_char("The gods will not allow you let go of your equipment.\n\r",ch);
	    return;
       }


	if ( get_obj_weight( obj ) + get_obj_weight( container )
	     > container->value[0] && container->pIndexData->vnum!=OBJ_VNUM_LOCKER)
	{
	    send_to_char( "It won't fit.\n\r", ch );
	    return;
	}
	
	if (container->pIndexData->vnum == OBJ_VNUM_PIT 
        &&  !CAN_WEAR(container,ITEM_TAKE))
    {
	    if (obj->timer)
	    {
	    	send_to_char( "Only permanent items may go in the pit.\n\r",ch);
	    	return;
	    }
	    else
        {
	        obj->timer = number_range(100,200);
        }
    }
        
        if(container->pIndexData->vnum == OBJ_VNUM_LOCKER &&
	   !IS_NPC(ch) && ch->pcdata->locker!=container)
        {
	    send_to_char("You can't put that in there.\n\r",ch);
	    return;
        }

	obj_from_char( obj );
	obj_to_obj( obj, container );
	act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
	act( "You put $p in $P.", ch, obj, container, TO_CHAR );
    }
    else
    {
	/* 'put all container' or 'put all.obj container' */
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   obj != container
	    &&   can_drop_obj( ch, obj )
	    &&   get_obj_weight( obj ) + get_obj_weight( container )
		 <= container->value[0] )
	    {
	    	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	    	&&  !CAN_WEAR(obj, ITEM_TAKE) )
	    	    if (obj->timer)
                {
	    	        continue;
                }
	    	    else
                {
	    	    	obj->timer = number_range(100,200);
                }
		obj_from_char( obj );
		obj_to_obj( obj, container );
		act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
		act( "You put $p in $P.", ch, obj, container, TO_CHAR );
	    }
	}
    }

    return;
}



void do_drop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Drop what?\n\r", ch );
	return;
    }

    if ( IS_SET(ch->act,PLR_KILLER) )
    {
	send_to_char("The gods prevent you from dropping your equipment.\n\r",ch);
	return;
    }
    if ( is_number( arg ) )
    {
	/* 'drop NNNN coins' */
	int amount;

	amount   = atoi(arg);
	argument = one_argument( argument, arg );
	if ( amount <= 0
	|| ( str_cmp( arg, "coins" ) && str_cmp( arg, "coin" ) && 
	     str_cmp( arg, "gold"  ) ) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

	if ( ch->gold < amount )
	{
	    send_to_char( "You haven't got that many coins.\n\r", ch );
	    return;
	}

	ch->gold -= amount;

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    switch ( obj->pIndexData->vnum )
	    {
	    case OBJ_VNUM_MONEY_ONE:
		amount += 1;
		extract_obj( obj );
		break;

	    case OBJ_VNUM_MONEY_SOME:
		amount += obj->value[0];
		extract_obj( obj );
		break;
	    }
	}

	obj_to_room( create_money( amount ), ch->in_room );
	act( "$n drops some gold.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "OK.\n\r", ch );
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
	/* 'drop obj' */
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

/*
	if (obj->value <= 0)
        {
	    act("As $n drops $p it turns to dust.",ch,obj,NULL,TO_ROOM);
	    act("As you drop $p it turns to dust.",ch,obj,NULL,TO_CHAR);
	    obj_from_char(obj);
	    free(obj);
	    return;
	}
*/
	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
	act( "You drop $p.", ch, obj, NULL, TO_CHAR );
  if ((obj->cost <= 0) && (obj->item_type != ITEM_CONTAINER) && (obj->item_type != ITEM_CORPSE_PC))
	{	
act("$p bursts into flames, leaving only a pile of dust in its place.",
	ch,obj,NULL,TO_ROOM);
act("$p bursts into flames, leaving only a pile of dust in its place.",
	ch,obj,NULL,TO_CHAR);
	extract_obj(obj);
	}
  }
    else
    {
	/* 'drop all' or 'drop all.obj' */
	found = FALSE;
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   can_drop_obj( ch, obj ) )
	    {
		found = TRUE;
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
		act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
		act( "You drop $p.", ch, obj, NULL, TO_CHAR );
if (obj->cost <= 0 && (obj->item_type != ITEM_CORPSE_PC))
	{	
act("$p bursts into flames, leaving only a pile of dust in its place.",
	ch,obj,NULL,TO_ROOM);
act("$p bursts into flames, leaving only a pile of dust in its place.",
	ch,obj,NULL,TO_CHAR);
	extract_obj(obj);
	}


	    }
	}

	if ( !found )
	{
	    if ( arg[3] == '\0' )
		act( "You are not carrying anything.",
		    ch, NULL, arg, TO_CHAR );
	    else
		act( "You are not carrying any $T.",
		    ch, NULL, &arg[4], TO_CHAR );
	}
    }

    return;
}



void do_give( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Give what to whom?\n\r", ch );
	return;
    }

    if ( IS_SET(ch->act,PLR_KILLER) )
    {
	send_to_char("The gods do not allow the transfer of objects between killers.\n\r",ch);
	return;
    }


    if ( is_number( arg1 ) )
    {
	/* 'give NNNN coins victim' */
	int amount;

	amount   = atoi(arg1);
	if ( amount <= 0
	|| ( str_cmp( arg2, "coins" ) && str_cmp( arg2, "coin" ) && 
	     str_cmp( arg2, "gold"  ) ) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

	argument = one_argument( argument, arg2 );
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Give what to whom?\n\r", ch );
	    return;
	}

	if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( ch->gold < amount )
	{
	    send_to_char( "You haven't got that much gold.\n\r", ch );
	    return;
	}

	ch->gold     -= amount;
	victim->gold += amount;
	sprintf(buf,"$n gives you %d gold.",amount);
	act( buf, ch, NULL, victim, TO_VICT    );
	act( "$n gives $N some gold.",  ch, NULL, victim, TO_NOTVICT );
	sprintf(buf,"You give $N %d gold.",amount);
	act( buf, ch, NULL, victim, TO_CHAR    );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "You must remove it first.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
	act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( victim->carry_weight + get_obj_weight( obj ) > can_carry_w( victim ) )
    {
	act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
	act( "$N can't see it.", ch, NULL, victim, TO_CHAR );
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, victim );
    act( "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );
    act( "$n gives you $p.",   ch, obj, victim, TO_VICT    );
    act( "You give $p to $N.", ch, obj, victim, TO_CHAR    );
    return;
}




void do_fill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *fountain;
    bool found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Fill what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    found = FALSE;
    for ( fountain = ch->in_room->contents; fountain != NULL;
	fountain = fountain->next_content )
    {
	if ( fountain->item_type == ITEM_FOUNTAIN )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
    {
	send_to_char( "There is no fountain here!\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "You can't fill that.\n\r", ch );
	return;
    }

    if ( obj->value[1] != 0 && obj->value[2] != 0 )
    {
	send_to_char( "There is already another liquid in it.\n\r", ch );
	return;
    }

    if ( obj->value[1] >= obj->value[0] )
    {
	send_to_char( "Your container is full.\n\r", ch );
	return;
    }

    act( "You fill $p.", ch, obj, NULL, TO_CHAR );
    obj->value[2] = 0;
    obj->value[1] = obj->value[0];
    return;
}
void do_empty( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Empty what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "You can't empty that.\n\r", ch );
	return;
    }

    if ( obj->value[1] == 0 )
    {
	send_to_char( "It's already empty.\n\r", ch );
	return;
    }

    act( "You empty $p.", ch, obj, NULL, TO_CHAR );
    obj->value[2] = 0;
    obj->value[1] = 0;
    return;
}



void do_drink( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;
    int liquid;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	{
	    if ( obj->item_type == ITEM_FOUNTAIN )
		break;
	}

	if ( obj == NULL )
	{
	    send_to_char( "Drink what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
    {
	send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
	return;
    }

    switch ( obj->item_type )
    {
    default:
	send_to_char( "You can't drink from that.\n\r", ch );
	break;

    case ITEM_FOUNTAIN:
	if ( !IS_NPC(ch) )
	    ch->pcdata->condition[COND_THIRST] = 48;
	act( "$n drinks from $p.", ch, obj, NULL, TO_ROOM );
	send_to_char( "You are no longer thirsty.\n\r", ch );
	break;

    case ITEM_DRINK_CON:
	if ( obj->value[1] <= 0 )
	{
	    send_to_char( "It is already empty.\n\r", ch );
	    return;
	}

	if ( ( liquid = obj->value[2] ) >= LIQ_MAX )
	{
	    bug( "Do_drink: bad liquid number %d.", liquid );
	    liquid = obj->value[2] = 0;
	}

	act( "$n drinks $T from $p.",
	    ch, obj, liq_table[liquid].liq_name, TO_ROOM );
	act( "You drink $T from $p.",
	    ch, obj, liq_table[liquid].liq_name, TO_CHAR );

	amount = number_range(3, 10);
	amount = UMIN(amount, obj->value[1]);
	
	gain_condition( ch, COND_DRUNK,
	    amount * liq_table[liquid].liq_affect[COND_DRUNK  ] );
	gain_condition( ch, COND_FULL,
	    amount * liq_table[liquid].liq_affect[COND_FULL   ] );
	gain_condition( ch, COND_THIRST,
	    amount * liq_table[liquid].liq_affect[COND_THIRST ] );

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	    send_to_char( "You feel drunk.\n\r", ch );
	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   > 40 )
	    send_to_char( "You are full.\n\r", ch );
	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 40 )
	    send_to_char( "You do not feel thirsty.\n\r", ch );
	
	if ( obj->value[3] != 0 )
	{
	    /* The shit was poisoned ! */
	    AFFECT_DATA af;

	    act( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
	    send_to_char( "You choke and gag.\n\r", ch );
	    af.type      = gsn_poison;
	    af.level	 = number_fuzzy(amount); 
	    af.duration  = 3 * amount;
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af );
	}
	
	obj->value[1] -= amount;
	break;
    }

    return;
}



void do_eat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Eat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( !IS_IMMORTAL(ch) )
    {
	if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL )
	{
	    send_to_char( "That's not edible.\n\r", ch );
	    return;
	}

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 40 )
	{   
	    send_to_char( "You are too full to eat more.\n\r", ch );
	    return;
	}
    }

    act( "$n eats $p.",  ch, obj, NULL, TO_ROOM );
    act( "You eat $p.", ch, obj, NULL, TO_CHAR );

    switch ( obj->item_type )
    {

    case ITEM_FOOD:
	if ( !IS_NPC(ch) )
	{
	    int condition;

	    condition = ch->pcdata->condition[COND_FULL];
	    gain_condition( ch, COND_FULL, obj->value[0] );
	    if ( condition == 0 && ch->pcdata->condition[COND_FULL] > 0 )
		send_to_char( "You are no longer hungry.\n\r", ch );
	    else if ( ch->pcdata->condition[COND_FULL] > 40 )
		send_to_char( "You are full.\n\r", ch );
	}

	if ( obj->value[3] != 0 )
	{
	    /* The shit was poisoned! */
	    AFFECT_DATA af;

	    act( "$n chokes and gags.", ch, 0, 0, TO_ROOM );
	    send_to_char( "You choke and gag.\n\r", ch );

	    af.type      = gsn_poison;
	    af.level 	 = number_fuzzy(obj->value[0]);
	    af.duration  = 2 * obj->value[0];
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af );
	}
	break;

    case ITEM_PILL:
	obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
	break;
    }

    extract_obj( obj );
    return;
}



/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	return TRUE;

    if ( !fReplace )
	return FALSE;

if (ch->real_hit > 0)
  {
    if ( IS_SET(obj->wear_flags, ITEM_WEAR_TATTOO) && get_trust(ch)<91)
    {
	send_to_char("Tattoos are for life! Laser surgery hasn't been invented yet!",ch);
	return FALSE;
    }

    if ( IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
    {
	act( "You can't remove $p.", ch, obj, NULL, TO_CHAR );
	return FALSE;
    }
   }
    unequip_char( ch, obj );
    act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
    act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
    return TRUE;
}



/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace )
{
    char buf[MAX_STRING_LENGTH];

    if (( ch->level+50 < obj->level) && ch->level < 50 )
    {
	sprintf( buf, "You must be level %d to use this object.\n\r",
	    obj->level );
	send_to_char( buf, ch );
	act( "$n tries to use $p, but is too inexperienced.",
	    ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( obj->item_type == ITEM_LIGHT )
    {
	if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
	    return;
	act( "$n lights $p and holds it.", ch, obj, NULL, TO_ROOM );
	act( "You light $p and hold it.",  ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LIGHT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )
    {
	if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
	&&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
	&&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
	&&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
	{
	    act( "$n wears $p on $s left finger.",    ch, obj, NULL, TO_ROOM );
	    act( "You wear $p on your left finger.",  ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
	{
	    act( "$n wears $p on $s right finger.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p on your right finger.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_R );
	    return;
	}

	bug( "Wear_obj: no free finger.", 0 );
	send_to_char( "You already wear two rings.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )
    {
	if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	&&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	&&   !remove_obj( ch, WEAR_NECK_1, fReplace )
	&&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
	{
	    act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_1 );
	    return;
	}

	if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
	{
	    act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_2 );
	    return;
	}

	bug( "Wear_obj: no free neck.", 0 );
	send_to_char( "You already wear two neck items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
    {
	if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
	    return;
	act( "$n wears $p on $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_BODY );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
    {
	if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
	    return;
	act( "$n wears $p on $s head.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your head.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HEAD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )
    {
	if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
	    return;
	act( "$n wears $p on $s legs.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your legs.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LEGS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )
    {
	if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
	    return;
	act( "$n wears $p on $s feet.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your feet.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FEET );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_TATTOO ) )
    {
	if ( !remove_obj( ch, WEAR_TATTOO, fReplace ) )
	   return;
	act( "$n gets a tattoo done on $s bicep.", ch, obj, NULL, TO_ROOM );
	act( "You get a tattoo done.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_TATTOO );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
    {
	if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
	    return;
	act( "$n wears $p on $s hands.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HANDS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) )
    {
	if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
	    return;
	act( "$n wears $p on $s arms.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your arms.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ARMS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
    {
	if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
	    return;
	act( "$n wears $p about $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p about your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ABOUT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
    {
	if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
	    return;
	act( "$n wears $p about $s waist.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p about your waist.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WAIST );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
    {
	if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
	&&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
	&&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
	&&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
	{
	    act( "$n wears $p around $s left wrist.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your left wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
	{
	    act( "$n wears $p around $s right wrist.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your right wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_R );
	    return;
	}

	bug( "Wear_obj: no free wrist.", 0 );
	send_to_char( "You already wear two wrist items.\n\r", ch );
	return;
    }

/* shield section */
    if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {

        if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
        {
            send_to_char ("You cannot use a shield while using 2 weapons.\n\r",ch);
            return;
        }

        if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
            return;
        act( "$n wears $p as a shield.", ch, obj, NULL, TO_ROOM );
        act( "You wear $p as a shield.", ch, obj, NULL, TO_CHAR );
        equip_char( ch, obj, WEAR_SHIELD );
        return;
    }


/* wield section */

    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    {
      int sn,skill;

	            
        if ( !remove_obj( ch, WEAR_WIELD, fReplace ) )
            return;
        
        if ( !IS_NPC(ch)
        && get_obj_weight( obj ) > str_app[get_curr_stat(ch,STAT_STR)].wield )
        {
            send_to_char( "It is too heavy for you to wield.\n\r", ch );
            return;
        }   
            
        if (!IS_NPC(ch) && ch->size < SIZE_LARGE
        &&  IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)
        &&  get_eq_char(ch,WEAR_SHIELD) != NULL)
        {
            send_to_char("You need two hands free for that weapon.\n\r",ch);
            return;
        }
            
        act( "$n wields $p.", ch, obj, NULL, TO_ROOM );
        act( "You wield $p.", ch, obj, NULL, TO_CHAR );
        equip_char( ch, obj, WEAR_WIELD );
        
        sn = get_weapon_sn(ch);
         
        if (sn == gsn_hand_to_hand)
           return; 
            
        skill = get_weapon_skill(ch,sn);
        
        if (skill >= 100)
            act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR);
        else if (skill > 85)

           act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 70)
            act("You are skilled with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 50)
            act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR);
        else if (skill > 25)
            act("$p feels a little clumsy in your hands.",ch,obj,NULL,TO_CHAR);
        else if (skill > 1)
            act("You fumble and almost drop $p.",ch,obj,NULL,TO_CHAR);
        else
            act("You don't even know which is end is up on $p.",
                ch,obj,NULL,TO_CHAR);
        
        return;
    } 


    if ( CAN_WEAR( obj, ITEM_HOLD ) )
    {
        if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
        {
            send_to_char ("You cannot hold an item while using 2 weapons.\n\r",ch);
            return;
        }

        if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
            return;
        act( "$n holds $p in $s hands.",   ch, obj, NULL, TO_ROOM );
        act( "You hold $p in your hands.", ch, obj, NULL, TO_CHAR );
        equip_char( ch, obj, WEAR_HOLD );
        return;
    }


    if ( fReplace )
	send_to_char( "You can't wear, wield, or hold that.\n\r", ch );

    return;
}



void do_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Wear, wield, or hold what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	OBJ_DATA *obj_next;

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
		wear_obj( ch, obj, FALSE );
	}
	return;
    }
    else
    {
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	wear_obj( ch, obj, TRUE );
    }

    return;
}

void do_second(CHAR_DATA *ch, char *argument)
{


    OBJ_DATA *obj;
    int wt = 0;

    if(get_skill(ch,gsn_dual_wield)==0)
    {
	send_to_char("\n\rHuh?",ch);
	return;
    }

    if (argument[0] == '\0') /* empty */
    {
        send_to_char ("Wear which weapon in your off-hand?\n\r",ch);
        return;
    }

    obj = get_obj_carry (ch, argument); /* find the obj withing ch's inventory */

    if (obj == NULL)
    {
        send_to_char ("You have no such thing in your backpack.\n\r",ch);
        return;
    }

    if( !CAN_WEAR(obj,ITEM_WIELD)){
      send_to_char("You can only dual wield a weapon!\n\r",ch);
      return;
    }

    /* check if the char is using a shield or a held weapon */

    if ( (get_eq_char (ch,WEAR_SHIELD) != NULL) ||
         (get_eq_char (ch,WEAR_HOLD)   != NULL) )
    {
        send_to_char ("You cannot use a secondary weapon while using a shield or holding an item.\n\r",ch);
        return;
    }


/* check that the character is using a first weapon at all */
    if (get_eq_char (ch, WEAR_WIELD) == NULL) /* oops - != here was a bit wrong :) */
    {
        send_to_char ("You need to wield a primary weapon, before using a secondary one!\n\r",ch);
        return;
    }


/* check for str - secondary weapons have to be lighter */
    if ( get_obj_weight( obj ) > ( str_app[get_curr_stat(ch,STAT_STR)].wield / 2) )
    {
        send_to_char( "This weapon is too heavy to be used as a secondary weapon by you.\n\r", ch );
        return;
    }

    wt = get_obj_weight(get_eq_char(ch,WEAR_WIELD));

    if ( ch->class == class_lookup("warrior") ){
	if( (wt <= 15) && (get_obj_weight(obj) > wt) ){
	   send_to_char( "Your weapon must be lighter than your primary one.\n\r",ch);    
	   return;
	}
	if( (wt > 15) && (get_obj_weight(obj) > (wt/2)) ){
	   send_to_char( "Your weapon must be lighter than your primary one.\n\r",ch);    
	   return;
	}
    }


    if ( ch->class == class_lookup("thief")){
	if( (wt <= 5) && (get_obj_weight(obj) > wt) ){
	   send_to_char( "Your weapon must be lighter than your primary one.\n\r",ch);    
	   return;
	}
	if( (wt > 5) ){
	   send_to_char( "Your primary weapon MUST be lighter than 5lbs!\n\r",ch);    
	   return;
	}
    }

/* check if the secondary weapon is at least half as light as the primary weapon */
/*    if ( (get_obj_weight (obj)*2) > 
get_obj_weight(get_eq_char(ch,WEAR_WIELD)) )
    {
        send_to_char ("Your secondary weapon has to be considerably lighter than the primary one.\n\r",ch);
        return;
    }
*/

/* at last - the char uses the weapon */

    if (!remove_obj(ch, WEAR_SECONDARY, TRUE)) /* remove the current weapon if any */
        return;                                /* remove obj tells about any no_remove */

/* char CAN use the item! that didn't take long at aaall */

    act ("$n wields $p in $s off-hand.",ch,obj,NULL,TO_ROOM);
    act ("You wield $p in your off-hand.",ch,obj,NULL,TO_CHAR);
    equip_char ( ch, obj, WEAR_SECONDARY);
    return;
}

void do_remove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int i = 0;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove what?\n\r", ch );
	return;
    }

    if ( !str_cmp(arg,"all") )
    {
	if (time(NULL) - ch->pk_timer < 90)
		{
		send_to_char( "You have to be safe to remove all.\n\r", ch);
		return;
		}
	if (IS_AFFECTED(ch, AFF_CHARM))
		{
		return;
		}
	for(i=0; i<19; i++)
	remove_obj( ch, i, TRUE );
	return;
    }

    if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    remove_obj( ch, obj->wear_loc, TRUE );
    return;
}



void do_sacrifice( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int gold;
    
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];


    one_argument( argument, arg );

    if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
    {
	act( "$n offers $mself to Tybalt, who graciously declines.",
	    ch, NULL, NULL, TO_ROOM );
	send_to_char(
	    "Tybalt appreciates your offer and may accept it later.\n\r", ch );
	return;
    }

    obj = get_obj_list( ch, arg, ch->in_room->contents );
    if ( obj == NULL )
    {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }

    if ( obj->pIndexData->vnum == OBJ_VNUM_LOCKER )
    {
	send_to_char( "Tybalt wouldn't like that one bit.\n\r",ch);
	return;
    }

    if ( obj->item_type == ITEM_CORPSE_PC )
    {
	if (obj->contains)
        {
	   send_to_char(
	     "Tybalt wouldn't like that.\n\r",ch);
	   return;
        }
    }


    if ( !CAN_WEAR(obj, ITEM_TAKE))
    {
	act( "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
	return;
    }

    gold = UMAX(1,obj->level * 2);

    if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    	gold = UMIN(gold,obj->cost);

    if (gold == 1)
        send_to_char(
	    "Tybalt gives you one gold coin for your sacrifice.\n\r", ch );
    else
    {
	sprintf(buf,"Tybalt gives you %d gold coins for your sacrifice.\n\r",
		gold);
	send_to_char(buf,ch);
    }

    wiznet("$N sends up $p as a burnt offering.",ch,obj,WIZ_SACCING,0,0);

    ch->gold += gold;
    
    if (IS_SET(ch->act,PLR_AUTOSPLIT) )
    { /* AUTOSPLIT code */
    	members = 0;
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    	{
    	    if ( is_same_group( gch, ch ) )
            members++;
    	}

	if ( members > 1 && gold > 1)
	{
	    sprintf(buffer,"%d",gold);
	    do_split(ch,buffer);	
	}
    }

    act( "$n sacrifices $p to Tybalt.", ch, obj, NULL, TO_ROOM );
    extract_obj( obj );
    return;
}



void do_quaff( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Quaff what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that potion.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_POTION )
    {
	send_to_char( "You can quaff only potions.\n\r", ch );
	return;
    }

    if (ch->level < obj->level)
    {
	send_to_char("This liquid is too powerful for you to drink.\n\r",ch);
	return;
    }

    act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
    act( "You quaff $p.", ch, obj, NULL ,TO_CHAR );

    obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );

    extract_obj( obj );
    return;
}



void do_recite( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *scroll;
    OBJ_DATA *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ( scroll = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that scroll.\n\r", ch );
	return;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
	send_to_char( "You can recite only scrolls.\n\r", ch );
	return;
    }

    if ( ch->level < scroll->level)
    {
	send_to_char(
		"This scroll is too complex for you to comprehend.\n\r",ch);
	return;
    }

    obj = NULL;
    if ( arg2[0] == '\0' )
    {
	victim = ch;
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    act( "$n recites $p.", ch, scroll, NULL, TO_ROOM );
    act( "You recite $p.", ch, scroll, NULL, TO_CHAR );

    if (number_percent() >= 20 + get_skill(ch,gsn_scrolls) * 4/5)
    {
	send_to_char("You mispronounce a syllable.\n\r",ch);
	check_improve(ch,gsn_scrolls,FALSE,2);
    }

    else
    {
    	obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
    	obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
    	obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );
	check_improve(ch,gsn_scrolls,TRUE,2);
    }

    extract_obj( scroll );
    return;
}



void do_brandish( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *staff;
    int sn;

    if ( ( staff = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( staff->item_type != ITEM_STAFF )
    {
	send_to_char( "You can brandish only with a staff.\n\r", ch );
	return;
    }

    if ( ( sn = staff->value[3] ) < 0
    ||   sn >= MAX_SKILL
    ||   skill_table[sn].spell_fun == 0 )
    {
	bug( "Do_brandish: bad sn %d.", sn );
	return;
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( staff->value[2] > 0 )
    {
	act( "$n brandishes $p.", ch, staff, NULL, TO_ROOM );
	act( "You brandish $p.",  ch, staff, NULL, TO_CHAR );
	if ( ch->level < staff->level 
	||   number_percent() >= 20 + get_skill(ch,gsn_staves) * 4/5)
 	{
	    act ("You fail to invoke $p.",ch,staff,NULL,TO_CHAR);
	    act ("...and nothing happens.",ch,NULL,NULL,TO_ROOM);
	    check_improve(ch,gsn_staves,FALSE,2);
	}
	
	else for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next	= vch->next_in_room;

	    switch ( skill_table[sn].target )
	    {
	    default:
		bug( "Do_brandish: bad target for sn %d.", sn );
		return;

	    case TAR_IGNORE:
		if ( vch != ch )
		    continue;
		break;

	    case TAR_CHAR_OFFENSIVE:
		if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
		    continue;
		break;
		
	    case TAR_CHAR_DEFENSIVE:
		if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
		    continue;
		break;

	    case TAR_CHAR_SELF:
		if ( vch != ch )
		    continue;
		break;
	    }

	    obj_cast_spell( staff->value[3], staff->value[0], ch, vch, NULL );
	    check_improve(ch,gsn_staves,TRUE,2);
	}
    }

    if ( --staff->value[2] <= 0 )
    {
	act( "$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM );
	act( "Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR );
	extract_obj( staff );
    }

    return;
}



void do_zap( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *wand;
    OBJ_DATA *obj;

    one_argument( argument, arg );
    if ( arg[0] == '\0' && ch->fighting == NULL )
    {
	send_to_char( "Zap whom or what?\n\r", ch );
	return;
    }

    if ( ( wand = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( wand->item_type != ITEM_WAND )
    {
	send_to_char( "You can zap only with a wand.\n\r", ch );
	return;
    }

    obj = NULL;
    if ( arg[0] == '\0' )
    {
	if ( ch->fighting != NULL )
	{
	    victim = ch->fighting;
	}
	else
	{
	    send_to_char( "Zap whom or what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( wand->value[2] > 0 )
    {
	if ( victim != NULL )
	{
	    act( "$n zaps $N with $p.", ch, wand, victim, TO_ROOM );
	    act( "You zap $N with $p.", ch, wand, victim, TO_CHAR );
	}
	else
	{
	    act( "$n zaps $P with $p.", ch, wand, obj, TO_ROOM );
	    act( "You zap $P with $p.", ch, wand, obj, TO_CHAR );
	}

 	if (ch->level < wand->level 
	||  number_percent() >= 20 + get_skill(ch,gsn_wands) * 4/5) 
	{
	    act( "Your efforts with $p produce only smoke and sparks.",
		 ch,wand,NULL,TO_CHAR);
	    act( "$n's efforts with $p produce only smoke and sparks.",
		 ch,wand,NULL,TO_ROOM);
	    check_improve(ch,gsn_wands,FALSE,2);
	}
	else
	{
	    obj_cast_spell( wand->value[3], wand->value[0], ch, victim, obj );
	    check_improve(ch,gsn_wands,TRUE,2);
	}
    }

    if ( --wand->value[2] <= 0 )
    {
	act( "$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM );
	act( "Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR );
	extract_obj( wand );
    }

    return;
}



void do_steal( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Steal what from whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && IS_SET(victim->act,PLR_KILLER) )
    {
	send_to_char("You may not steal from killers.\n\r",ch);
	return;
    }

    if ( !IS_NPC(victim) && victim->level < 16 )
    {
	send_to_char("You may not steal from them until they reach level 16.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (victim->position == POS_FIGHTING)
    {
	send_to_char("You'd better not -- you might get hit.\n\r",ch);
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_steal].beats );
    percent  = number_percent( ) + ( IS_AWAKE(victim) ? 10 : -50 );

    if ( ch->level + 5 < victim->level
    ||   victim->position == POS_FIGHTING
    ||   !IS_NPC(victim)
    || ( !IS_NPC(ch) && percent > ch->pcdata->learned[gsn_steal] ) )
    {
	/*
	 * Failure.
	 */
	send_to_char( "Oops.\n\r", ch );
	act( "$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT    );
	act( "$n tried to steal from $N.\n\r",  ch, NULL, victim, TO_NOTVICT );
	switch(number_range(0,3))
	{
	case 0 :
	   sprintf( buf, "%s is a lousy thief!", ch->name );
	   break;
        case 1 :
	   sprintf( buf, "%s couldn't rob %s way out of a paper bag!",
		    ch->name,(ch->sex == 2) ? "her" : "his");
	   break;
	case 2 :
	    sprintf( buf,"%s tried to rob me!",ch->name );
	    break;
	case 3 :
	    sprintf(buf,"Keep your hands out of there, %s!",ch->name);
	    break;
        }
	do_yell( victim, buf );
	if ( !IS_NPC(ch) )
	{
	    if ( IS_NPC(victim) )
	    {
	        check_improve(ch,gsn_steal,FALSE,2);
		multi_hit( victim, ch, TYPE_UNDEFINED );
	    }
	    else
	    {

		sprintf(buf,"$N tried to steal from %s.",victim->name);
		wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);

		log_string( buf );
		if ( !IS_SET(ch->act, PLR_THIEF) )
		{
		    SET_BIT(ch->act, PLR_THIEF);
		    send_to_char( "*** You are now a THIEF!! ***\n\r", ch );
		    save_char_obj( ch );
		}
	    }
	}

	return;
    }

    if ( !str_cmp( arg1, "coin"  )
    ||   !str_cmp( arg1, "coins" )
    ||   !str_cmp( arg1, "gold"  ) )
    {
	int amount;

	amount = victim->gold * number_range(1, 10) / 100;
	if ( amount <= 0 )
	{
	    send_to_char( "You couldn't get any gold.\n\r", ch );
	    return;
	}

	ch->gold     += amount;
	victim->gold -= amount;
	sprintf( buf, "Bingo!  You got %d gold coins.\n\r", amount );
	send_to_char( buf, ch );
	check_improve(ch,gsn_steal,TRUE,2);
	return;
    }

    if ( ( obj = get_obj_carry( victim, arg1 ) ) == NULL )
    {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }
	
    if ( !can_drop_obj( ch, obj )
    ||   IS_SET(obj->extra_flags, ITEM_INVENTORY)
    ||   obj->level > ch->level )
    {
	send_to_char( "You can't pry it away.\n\r", ch );
	return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	send_to_char( "You have your hands full.\n\r", ch );
	return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
	send_to_char( "You can't carry that much weight.\n\r", ch );
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, ch );
    check_improve(ch,gsn_steal,TRUE,2);
    return;
}



/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *keeper;
    SHOP_DATA *pShop;

    pShop = NULL;
    for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
    {
	if ( IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL )
	    break;
    }

    if ( pShop == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return NULL;
    }

    /*
     * Undesirables.
     */
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_KILLER) )
    {
	do_say( keeper, "Killers are not welcome!" );
	sprintf( buf, "%s the KILLER is over here!\n\r", ch->name );
	do_yell( keeper, buf );
	return NULL;
    }

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_THIEF) )
    {
	do_say( keeper, "Thieves are not welcome!" );
	sprintf( buf, "%s the THIEF is over here!\n\r", ch->name );
	do_yell( keeper, buf );
	return NULL;
    }

    /*
     * Shop hours.
     */
    if ( time_info.hour < pShop->open_hour )
    {
	do_say( keeper, "Sorry, I am closed. Come back later." );
	return NULL;
    }
    
    if ( time_info.hour > pShop->close_hour )
    {
	do_say( keeper, "Sorry, I am closed. Come back tomorrow." );
	return NULL;
    }

    /*
     * Invisible or hidden people.
     */
    if ( !can_see( keeper, ch ) )
    {
	do_say( keeper, "I don't trade with folks I can't see." );
	return NULL;
    }

    return keeper;
}



int get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
    SHOP_DATA *pShop;
    int cost;

    if ( obj == NULL || ( pShop = keeper->pIndexData->pShop ) == NULL )
	return 0;

    if ( fBuy )
    {
	cost = obj->cost * pShop->profit_buy  / 100;
    }
    else
    {
	OBJ_DATA *obj2;
	int itype;

	cost = 0;
	for ( itype = 0; itype < MAX_TRADE; itype++ )
	{
	    if ( obj->item_type == pShop->buy_type[itype] )
	    {
		cost = obj->cost * pShop->profit_sell / 100;
		break;
	    }
	}

	for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content )
	{
	    if ( obj->pIndexData == obj2->pIndexData )
            {
                cost = 0;
                break;
            }
	}
    }

    if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND )
	cost = cost * obj->value[2] / obj->value[1];

    return cost;
}



void do_buy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int number=0;
    int cost,roll,i;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Buy what?\n\r", ch );
	return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    {
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *pet;
	ROOM_INDEX_DATA *pRoomIndexNext;
	ROOM_INDEX_DATA *in_room;

	if ( IS_NPC(ch) )
	    return;

	argument = one_argument(argument,arg);

	pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	}

	in_room     = ch->in_room;
	ch->in_room = pRoomIndexNext;
	pet         = get_char_room( ch, arg );
	ch->in_room = in_room;

	if ( pet == NULL || !IS_SET(pet->act, ACT_PET) )
	{
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	}

	if ( ch->pet != NULL )
	{
	    send_to_char("You already own a pet.\n\r",ch);
	    return;
	}

 	cost = 10 * pet->level * pet->level;

	if ( ch->gold < cost )
	{
	    send_to_char( "You can't afford it.\n\r", ch );
	    return;
	}

	if ( ch->level < pet->level )
	{
	    send_to_char( "You're not powerful enough to master this pet.\n\r", ch );
	    return;
	}

	/* haggle */
	roll = number_percent();
	if (!IS_NPC(ch) && roll < ch->pcdata->learned[gsn_haggle])
	{
	    cost -= cost / 2 * roll / 100;
	    sprintf(buf,"You haggle the price down to %d coins.\n\r",cost);
	    send_to_char(buf,ch);
	    check_improve(ch,gsn_haggle,TRUE,4);
	
	}

	ch->gold		-= cost;
	pet			= create_mobile( pet->pIndexData );
	SET_BIT(ch->act, PLR_BOUGHT_PET);
	SET_BIT(pet->act, ACT_PET);
	SET_BIT(pet->affected_by, AFF_CHARM);
	pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;

	argument = one_argument( argument, arg );
/*	if ( arg[0] != '\0' )
	{
	    sprintf( buf, "%s %s", pet->name, arg );
	    free_string( pet->name );
	    pet->name = str_dup( buf );
	}*/

	sprintf( buf, "%sA neck tag says 'I belong to %s'.\n\r",
	    pet->description, ch->name );
	free_string( pet->description );
	pet->description = str_dup( buf );

	char_to_room( pet, ch->in_room );
	add_follower( pet, ch );
	pet->leader = ch;
	ch->pet = pet;
	send_to_char( "Enjoy your pet.\n\r", ch );
	act( "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );
	return;
    }
    else
    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj;

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;

	argument = one_argument(argument, arg);

	if(!is_number(arg))
	{
 	    obj = get_obj_carry( keeper, arg );
	    number = 1;
	    cost = get_cost( keeper, obj, TRUE)*number;
 	}
	else
	{
	    number = atoi(arg);
            argument = one_argument(argument, arg);
      
	    obj  = get_obj_carry( keeper, arg );
	    cost = get_cost( keeper, obj, TRUE )*number;
	}

        if(number==0) number = 1;

	if ( cost <= 0 || !can_see_obj( ch, obj ) )
	{
	    act( "$n tells you 'I don't sell that -- try 'list'.",keeper, NULL, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}



	if ( !IS_SET( obj->extra_flags, ITEM_INVENTORY ) && number > 1)
	{
	    act("$n tells you 'I don't have that many of that item.'",keeper,NULL,ch,TO_VICT);
	    return;
	}

	if(number < 1)
	{
	    send_to_char("Please use only positive integers.\n\r",ch);
	    return;
	}

	if ( ch->gold < cost )
	{
	    act( "$n tells you 'You can't afford to buy $p'.",keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}
/*	
	if ( obj->level > ch->level )
	{
	    act( "$n tells you 'You can't use $p yet'.",
		keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}
*/


	if ( ch->carry_number + get_obj_number( obj )*number > can_carry_n( ch ) )
	{
	    send_to_char( "You can't carry that many items.\n\r", ch );
	    return;
	}

	if ( ch->carry_weight + get_obj_weight( obj )*number > can_carry_w( ch ) )
	{
	    send_to_char( "You can't carry that much weight.\n\r", ch );
	    return;
	}

	/* haggle */
	roll = number_percent();
	if (!IS_NPC(ch) && roll < ch->pcdata->learned[gsn_haggle])
	{
	    cost -= (obj->cost) / 2 * roll / 100;
	    sprintf(buf,"You haggle the price down to %d coins.\n\r",cost);
	    send_to_char(buf,ch);
	    check_improve(ch,gsn_haggle,TRUE,4);
	}

	if(number > 1)
	{
	sprintf(buf, "$n buys %d things of $p.",number);
	act(buf, ch, obj, NULL, TO_ROOM );
	sprintf(buf, "You buy %d things of $p.",number);
	act(buf, ch, obj, NULL, TO_CHAR );
	}
        else
	{
	sprintf(buf, "$n buys $p.");
	act(buf, ch, obj, NULL, TO_ROOM );
	sprintf(buf, "You buy $p.");
	act(buf, ch, obj, NULL, TO_CHAR );
	}
	ch->gold     -= cost;
	keeper->gold += cost;

	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	{
	    for(i=1;i<=number;i++)
	    {
	    obj = create_object( obj->pIndexData, obj->level );
    	    if (obj->timer > 0)
	    	obj-> timer = 0;
	    obj_to_char( obj, ch );
	    if (cost < obj->cost)
	    	obj->cost = cost/number;
   	    }
	}

	else
	{
	    obj_from_char( obj );
	if (obj->timer > 0)
	    obj-> timer = 0;
	obj_to_char( obj, ch );
	if (cost < obj->cost)
	    obj->cost = cost;
	}
	return;

    }
}

void do_list( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    {
	ROOM_INDEX_DATA *pRoomIndexNext;
	CHAR_DATA *pet;
	bool found;

	pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

	found = FALSE;
	for ( pet = pRoomIndexNext->people; pet; pet = pet->next_in_room )
	{
	    if ( IS_SET(pet->act, ACT_PET) )
	    {
		if ( !found )
		{
		    found = TRUE;
		    send_to_char( "Pets for sale:\n\r", ch );
		}
		sprintf( buf, "[%2d] %8d - %s\n\r",
		    pet->level,
		    10 * pet->level * pet->level,
		    pet->short_descr );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "Sorry, we're out of pets right now.\n\r", ch );
	return;
    }
    else
    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost;
	bool found;
	char arg[MAX_INPUT_LENGTH];

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;
        one_argument(argument,arg);

	found = FALSE;
	for ( obj = keeper->carrying; obj; obj = obj->next_content )
	{
	    if ( obj->wear_loc == WEAR_NONE
	    &&   can_see_obj( ch, obj )
	    &&   ( cost = get_cost( keeper, obj, TRUE ) ) > 0 
	    &&   ( arg[0] == '\0'  
 	       ||  is_name(arg,obj->name) ))
	    {
		if ( !found )
		{
		    found = TRUE;
		    send_to_char( "[Lv Price] Item\n\r", ch );
		}

		sprintf( buf, "[%2d %5d] %s.\n\r",
		    obj->level, cost, obj->short_descr);
		send_to_char( buf, ch );
	    }
	}

	if ( !found )
	    send_to_char( "You can't buy anything here.\n\r", ch );
	return;
    }
}



void do_sell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost,roll;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Sell what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if (!can_see_obj(keeper,obj))
    {
	act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
	return;
    }

    /* won't buy rotting goods */
    if ( obj->timer || ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    if ( cost > keeper->gold )
    {
	act("$n tells you 'I'm afraid I don't have enough gold to buy $p.",
	    keeper,obj,ch,TO_VICT);
	return;
    }

    act( "$n sells $p.", ch, obj, NULL, TO_ROOM );
    /* haggle */
    roll = number_percent();
    if (!IS_NPC(ch) && roll < ch->pcdata->learned[gsn_haggle])
    {
        send_to_char("You haggle with the shopkeeper.\n\r",ch);
        cost += obj->cost / 2 * roll / 100;
        cost = UMIN(cost,95 * get_cost(keeper,obj,TRUE) / 100);
	cost = UMIN(cost,keeper->gold);
        check_improve(ch,gsn_haggle,TRUE,4);
    }
    sprintf( buf, "You sell $p for %d gold piece%s.",
	cost, cost == 1 ? "" : "s" );
    act( buf, ch, obj, NULL, TO_CHAR );
    ch->gold     += cost;
    keeper->gold -= cost;
    if ( keeper->gold < 0 )
	keeper->gold = 0;

    if ( obj->item_type == ITEM_TRASH )
    {
	extract_obj( obj );
    }
    else
    {
	obj_from_char( obj );
	obj->timer = number_range(50,100);
	obj_to_char( obj, keeper );
    }

    return;
}



void do_value( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Value what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if (!can_see_obj(keeper,obj))
    {
        act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    sprintf( buf, "$n tells you 'I'll give you %d gold coins for $p'.", cost );
    act( buf, keeper, obj, ch, TO_VICT );
    ch->reply = keeper;

    return;
}
