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
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"

#if defined(unix)
#include <unistd.h>
#endif

/* command procedures needed */
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN( do_look		);
DECLARE_DO_FUN( do_help		);
DECLARE_DO_FUN( do_scan		);
DECLARE_DO_FUN( do_color	);


char *	const	where_name	[] =
{
    "<used as light>     ",
    "<worn on finger>    ",
    "<worn on finger>    ",
    "<worn around neck>  ",
    "<worn around neck>  ",
    "<worn on body>      ",
    "<worn on head>      ",
    "<worn on legs>      ",
    "<worn on feet>      ",
    "<worn on hands>     ",
    "<worn on arms>      ",
    "<worn as shield>    ",
    "<worn about body>   ",
    "<worn about waist>  ",
    "<worn around wrist> ",
    "<worn around wrist> ",
    "<wielded>           ",
    "<held>              ",
    "<secondary weapon>  ",
    "<etched onto bicep> ",
};


/* for do_count */
int max_on = 0;



/*
 * Local functions.
 */
char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				    bool fShort ) );
void	show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing ) );
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool	check_blind		args( ( CHAR_DATA *ch ) );



char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';
    if ( IS_OBJ_STAT(obj, ITEM_INVIS)     )  strcat( buf, "(^DBInvis^NW)"     );
    if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
         && IS_OBJ_STAT(obj, ITEM_EVIL)   ) strcat( buf, "(^NRRed Aura^NW)"  );
    if ( IS_OBJ_STAT(obj, ITEM_GLOW)    )   strcat( buf, "(^BBGlowing^NW) "   );
    if ( IS_OBJ_STAT(obj, ITEM_HUM)   )   strcat( buf, "(^BPHumming^NW) "   );

    if ( fShort )
    {
	if ( obj->short_descr != NULL )
	    strcat( buf, obj->short_descr );
    }
    else
    {
	if ( obj->description != NULL )
	    strcat( buf, obj->description );
    }

    return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    char buf[MAX_STRING_LENGTH];
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if ( ch->desc == NULL )
	return;

    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
	count++;
    prgpstrShow	= alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(int)    );
    nShow	= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_content )
    { 
	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );
	    fCombine = FALSE;

	    if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "(%2d) ", prgnShow[iShow] );
		send_to_char( buf, ch );
	    }
	    else
	    {
		send_to_char( "     ", ch );
	    }
	}
	send_to_char( prgpstrShow[iShow], ch );
	send_to_char( "\n\r", ch );
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    send_to_char( "     ", ch );
	send_to_char( "Nothing.\n\r", ch );
    }

    /*
     * Clean up.
     */
    free_mem( prgpstrShow, count * sizeof(char *) );
    free_mem( prgnShow,    count * sizeof(int)    );

    return;
}

void do_remort(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    if(IS_NPC(ch)) return;

    if(ch->level != 90) 
    {
	send_to_char("You have not yet attained a high enough level.\n\r",ch);
	return;
    }

    if(ch->fighting!=NULL)
    {
	send_to_char("Wait until you've finished the fight.\n\r",ch);
	return;
    }

    if ( time(NULL) - ch->pk_timer < 90 && get_trust(ch) < 91)
    {
        sprintf(buf,"You are not safe yet! You need to wait %ld more seconds.\n\r",(int) ch->pk_timer - time(NULL));
	send_to_char(buf,ch);
        return;
    }
/*
    if ( (get_eq_char (ch,WEAR_SHIELD) != NULL) ||
	 (get_eq_char (ch,WEAR_LIGHT) != NULL) ||	 	
	 (get_eq_char (ch,WEAR_FINGER_L) != NULL) ||	 	
	 (get_eq_char (ch,WEAR_FINGER_R) != NULL) ||	 	
	 (get_eq_char (ch,WEAR_NECK_1) != NULL) ||	 	
	 (get_eq_char (ch,WEAR_NECK_2) != NULL) ||	 	
	 (get_eq_char (ch,WEAR_BODY) != NULL) ||	 	
	 (get_eq_char (ch,WEAR_HEAD) != NULL) ||	 	
	 (get_eq_char (ch,WEAR_LEGS) != NULL) ||	 	
	 (get_eq_char (ch,WEAR_FEET) != NULL) ||	 	
	 (get_eq_char (ch,WEAR_HANDS) != NULL) ||	 	
	 (get_eq_char (ch,WEAR_ARMS) != NULL) ||	 	
	 (get_eq_char (ch,WEAR_ABOUT) != NULL) ||	 	
	 (get_eq_char (ch,WEAR_WAIST) != NULL) ||	 	
	 (get_eq_char (ch,WEAR_WRIST_L) != NULL) ||	 	
	 (get_eq_char (ch,WEAR_WRIST_R) != NULL) ||	 	
	 (get_eq_char (ch,WEAR_WIELD) != NULL) ||	 	
	 (get_eq_char (ch,WEAR_HOLD) != NULL) ||	 	
	 (get_eq_char (ch,WEAR_SECONDARY) != NULL) ||	 	
	 (get_eq_char (ch,WEAR_TATTOO) != NULL))
	 {
		send_to_char("You must remove ALL equipment before you can remort!\n\r",ch);
	 }
*/



    if(ch->class==class_lookup("magician") ||
       ch->class==class_lookup("mage") ||
       ch->class==class_lookup("acolyte") ||
       ch->class==class_lookup("cleric") ||
       ch->class==class_lookup("pickpocket") ||
       ch->class==class_lookup("thief")	||
       ch->class==class_lookup("swordpupil") ||
       ch->class==class_lookup("warrior"))
    {
	ch->class++;
	ch->max_hit 	= 20;
	ch->real_hit 	= 20;
	ch->real_mana 	= 100;
	ch->real_move 	= 100;
	ch->hit 	= 20;
	ch->max_mana 	= 100;
	ch->mana	= 100;
	ch->move	= 100;
	ch->max_move	= 100;
	ch->level	= 1;
	reset_char(ch);

	sprintf(buf,"You begin a new life as a %s.\n\r",class_table[ch->class].name);
	send_to_char(buf,ch);
	act("$n begins the remort process. They vanish from the room!",ch,NULL,NULL,TO_ROOM);
	char_from_room(ch);
	ch->desc->connected=CON_REMORT;
	nanny(ch->desc,ch->desc->incomm);
	ch->exp = exp_per_level(ch,ch->points);
    }
    else
	send_to_char("You may not progress any further. You are already at the top!\n\r",ch);

}

void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if ( !IS_NPC(victim) && victim->ld==TRUE ) strcat( buf, "(^BKLD^NW) " );
    if ( IS_SET(victim->comm, COMM_AFK)	)	strcat( buf, "[^NBAFK^NW] ");
    if ( IS_AFFECTED(victim, AFF_INVISIBLE)   ) strcat( buf, "(^NBInvis^NW) " );
    if ( IS_SET(victim->act, PLR_WIZINVIS)    ) strcat( buf, "(^NPWizi^NW) " );
    if ( victim->incog_level > 0 ) strcat( buf, "(^NBIncog^NW) " );
    if ( IS_AFFECTED(victim, AFF_HIDE)     ) strcat( buf, "(^NRHide^NW) "    );
    if ( IS_AFFECTED(victim, AFF_CHARM)       ) strcat( buf, "(^NYCharmed^NW) ");
    if ( IS_AFFECTED(victim, AFF_PASS_DOOR)) strcat(buf,"(^BRTranslucent^NW) ");
    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE)) strcat( buf,"(^BPPink Aura^NW) ");
    if ( IS_EVIL(victim)
    &&   IS_AFFECTED(ch, AFF_DETECT_EVIL)  ) strcat( buf,"(^NRRed Aura^NW) ");
    if ( IS_AFFECTED(victim, AFF_SANCTUARY)) strcat( buf, "(^BWWhite Aura^NW) ");
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER ) )
					strcat(buf,"(^BRKILLER^NW) "    );
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF  ) )
					strcat( buf, "(^NYTHIEF^NW) "      );
    if (IS_NPC(victim) &&ch->questmob > 0 && victim->pIndexData->vnum == ch->questmob)
	strcat( buf, "[TARGET] ");

    if ( victim->position == victim->start_pos && victim->long_descr[0] != '\0' )
    {
	strcat( buf, victim->long_descr );
	send_to_char( buf, ch );
	return;
    }

    if( IS_SET(victim->affected_by, AFF_DISGUISE ) && (get_curr_stat(ch,STAT_INT)<23 || (ch->level < victim->level+7)))
	strcat( buf, disguise_table[(victim->level >= 90) ? 69 : (victim->level-21)].long_descr );
    else
    {
	strcat( buf, PERS( victim, ch ) );
    	if ( !IS_NPC(victim) && !IS_SET(ch->comm, COMM_BRIEF) )
	strcat( buf, victim->pcdata->title );

    switch ( victim->position )
    {
    case POS_DEAD:     strcat( buf, " is DEAD!!" );              break;
    case POS_MORTAL:   strcat( buf, " is mortally wounded." );   break;
    case POS_INCAP:    strcat( buf, " is incapacitated." );      break;
    case POS_STUNNED:  strcat( buf, " is lying here stunned." ); break;
    case POS_SLEEPING: strcat( buf, " is sleeping here." );      break;
    case POS_RESTING:  strcat( buf, " is resting here." );       break;
    case POS_SITTING:  strcat( buf, " is sitting here." );	 break;
    case POS_STANDING: strcat( buf, " is here." );               break;
    case POS_FIGHTING:
	strcat( buf, " is here, fighting " );
	if ( victim->fighting == NULL )
	    strcat( buf, "thin air??" );
	else if ( victim->fighting == ch )
	    strcat( buf, "YOU!" );
	else if ( victim->in_room == victim->fighting->in_room )
	{
	    strcat( buf, PERS( victim->fighting, ch ) );
	    strcat( buf, "." );
	}
	else
	    strcat( buf, "someone who left??" );
	break;
    }
    }
    strcat( buf, "\n\r" );
    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );
    return;
}



void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int iWear;
    int percent;
    bool found;

    if ( can_see( victim, ch ) )
    {
	if (ch == victim)
	    act( "$n looks at $mself.",ch,NULL,NULL,TO_ROOM);
	else
	{
	    act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
	    act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
	}
    }

    if ( victim->description[0] != '\0' )
    {
	send_to_char( victim->description, ch );
    }
    else
    {
	act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }

    if ( victim->max_hit > 0 )
	percent = ( 100 * victim->hit ) / victim->max_hit;
    else
	percent = -1;

    strcpy( buf, PERS(victim, ch) );

    if (percent >= 100) 
	strcat( buf, " is in excellent condition.\n\r");
    else if (percent >= 90) 
	strcat( buf, " has a few scratches.\n\r");
    else if (percent >= 75) 
	strcat( buf," has some small wounds and bruises.\n\r");
    else if (percent >=  50) 
	strcat( buf, " has quite a few wounds.\n\r");
    else if (percent >= 30)
	strcat( buf, " has some big nasty wounds and scratches.\n\r");
    else if (percent >= 15)
	strcat ( buf, " looks pretty hurt.\n\r");
    else if (percent >= 0 )
	strcat (buf, " is in awful condition.\n\r");
    else
	strcat(buf, " is bleeding to death.\n\r");

    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {

        if (iWear == WEAR_HANDS && ((obj = get_eq_char(victim,WEAR_TATTOO)) != NULL))
        {

	    send_to_char( where_name[WEAR_TATTOO], ch );
	    if ( can_see_obj( ch, obj ) )
	    {
	        send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	        send_to_char( "\n\r", ch );
	    }
	    found = TRUE;
	}


	if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
	&&   can_see_obj( ch, obj ) )
	{
	    if ( !found )
	    {
		send_to_char( "\n\r", ch );
		act( "$N is using:", ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    send_to_char( where_name[iWear], ch );
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
    }

    if ( victim != ch
    &&   !IS_NPC(ch)
    &&   number_percent( ) < ch->pcdata->learned[gsn_peek] )
    {
	send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
	check_improve(ch,gsn_peek,TRUE,4);
	show_list_to_char( victim->carrying, ch, TRUE, TRUE );
    }

    return;
}



void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    for ( rch = list; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch == ch )
	    continue;

	if ( !IS_NPC(rch)
	&&   IS_SET(rch->act, PLR_WIZINVIS)
	&&   get_trust( ch ) < rch->invis_level )
	    continue;

	if ( can_see( ch, rch ) )
	{
	    show_char_to_char_0( rch, ch );
	}
	else if ( room_is_dark( ch->in_room )
	&&        IS_AFFECTED(rch, AFF_INFRARED ) )
	{
	 send_to_char( "You see glowing ^NRred^NW eyes watching YOU!\n\r", ch );
	}
    }

    return;
} 



bool check_blind( CHAR_DATA *ch )
{

    if (!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    { 
	send_to_char( "You can't see a thing!\n\r", ch ); 
	return FALSE; 
    }

    return TRUE;
}

/* changes your scroll */
void do_scroll(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int lines;

    one_argument(argument,arg);
    
    if (arg[0] == '\0')
    {
	if (ch->lines == 0)
	    send_to_char("You do not page long messages.\n\r",ch);
	else
	{
	    sprintf(buf,"You currently display %d lines per page.\n\r",
		    ch->lines + 2);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!is_number(arg))
    {
	send_to_char("You must provide a number.\n\r",ch);
	return;
    }

    lines = atoi(arg);

    if (lines == 0)
    {
//        send_to_char("Paging disabled.\n\r",ch);
//        ch->lines = 0;
	send_to_char("You can't disable paging.\n\r",ch);
        return;
    }

    if (lines < 10 || lines > 100)
    {
	send_to_char("You must provide a reasonable number.\n\r",ch);
	return;
    }

    sprintf(buf,"Scroll set to %d lines.\n\r",lines);
    send_to_char(buf,ch);
    ch->lines = lines - 2;
}

/* RT does socials */
void do_socials(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int iSocial;
    int col;
     
    col = 0;
   
    for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
    {
	sprintf(buf,"%-12s",social_table[iSocial].name);
	send_to_char(buf,ch);
	if (++col % 6 == 0)
	    send_to_char("\n\r",ch);
    }

    if ( col % 6 != 0)
	send_to_char("\n\r",ch);
    return;
}


 
/* RT Commands to replace news, motd, imotd, etc from ROM */

void do_news(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"news");
}

void do_motd(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"motd");
}

void do_imotd(CHAR_DATA *ch, char *argument)
{  
    do_help(ch,"imotd");
}

void do_rules(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"rules");
}

void do_story(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"story");
}

void do_changes(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"changes");
}

void do_wizlist(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"wizlist");
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

void do_autolist(CHAR_DATA *ch, char *argument)
{
    /* lists most player flags */
    if (IS_NPC(ch))
      return;

    send_to_char("   action     status\n\r",ch);
    send_to_char("---------------------\n\r",ch);
 
    send_to_char("autoassist     ",ch);
    if (IS_SET(ch->act,PLR_AUTOASSIST))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch); 

    send_to_char("autoexit       ",ch);
    if (IS_SET(ch->act,PLR_AUTOEXIT))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autogold       ",ch);
    if (IS_SET(ch->act,PLR_AUTOGOLD))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autoloot       ",ch);
    if (IS_SET(ch->act,PLR_AUTOLOOT))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autosac        ",ch);
    if (IS_SET(ch->act,PLR_AUTOSAC))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autosplit      ",ch);
    if (IS_SET(ch->act,PLR_AUTOSPLIT))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("prompt         ",ch);
    if (IS_SET(ch->comm,COMM_PROMPT))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("combine items  ",ch);
    if (IS_SET(ch->comm,COMM_COMBINE))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    if (!IS_SET(ch->act,PLR_CANLOOT))
	send_to_char("Your corpse is safe from thieves.\n\r",ch);
    else 
        send_to_char("Your corpse may be looted.\n\r",ch);

    if (IS_SET(ch->act,PLR_NOSUMMON))
	send_to_char("You cannot be summoned.\n\r",ch);
    else
	send_to_char("You can be summoned.\n\r",ch);
   
    if (IS_SET(ch->act,PLR_NOFOLLOW))
	send_to_char("You do not welcome followers.\n\r",ch);
    else
	send_to_char("You accept followers.\n\r",ch);
}

void do_autoassist(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
    
    if (IS_SET(ch->act,PLR_AUTOASSIST))
    {
      send_to_char("Autoassist removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOASSIST);
    }
    else
    {
      send_to_char("You will now assist when needed.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOASSIST);
    }
}

void do_autoexit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOEXIT))
    {
      send_to_char("Exits will no longer be displayed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOEXIT);
    }
    else
    {
      send_to_char("Exits will now be displayed.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOEXIT);
    }
}

void do_autogold(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOGOLD))
    {
      send_to_char("Autogold removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOGOLD);
    }
    else
    {
      send_to_char("Automatic gold looting set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOGOLD);
    }
}

void do_autoloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOLOOT))
    {
      send_to_char("Autolooting removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOLOOT);
    }
    else
    {
      send_to_char("Automatic corpse looting set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOLOOT);
    }
}

void do_autosac(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOSAC))
    {
      send_to_char("Autosacrificing removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOSAC);
    }
    else
    {
      send_to_char("Automatic corpse sacrificing set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOSAC);
    }
}

void do_autosplit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOSPLIT))
    {
      send_to_char("Autosplitting removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOSPLIT);
    }
    else
    {
      send_to_char("Automatic gold splitting set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOSPLIT);
    }
}

void do_brief(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_BRIEF))
    {
      send_to_char("Full descriptions activated.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_BRIEF);
    }
    else
    {
      send_to_char("Short descriptions activated.\n\r",ch);
      SET_BIT(ch->comm,COMM_BRIEF);
    }
}

void do_compact(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMPACT))
    {
      send_to_char("Compact mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMPACT);
    }
    else
    {
      send_to_char("Compact mode set.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMPACT);
    }
}

void do_prompt(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_PROMPT))
    {
      send_to_char("You will no longer see prompts.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_PROMPT);
    }
    else
    {
      send_to_char("You will now see prompts.\n\r",ch);
      SET_BIT(ch->comm,COMM_PROMPT);
    }
}

void do_combine(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMBINE))
    {
      send_to_char("Long inventory selected.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMBINE);
    }
    else
    {
      send_to_char("Combined inventory selected.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMBINE);
    }
}

void do_noloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_CANLOOT))
    {
      send_to_char("Your corpse is now safe from thieves.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_CANLOOT);
    }
    else
    {
      send_to_char("Your corpse may now be looted.\n\r",ch);
      SET_BIT(ch->act,PLR_CANLOOT);
    }
}

void do_nofollow(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_NOFOLLOW))
    {
      send_to_char("You now accept followers.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    }
    else
    {
      send_to_char("You no longer accept followers.\n\r",ch);
      SET_BIT(ch->act,PLR_NOFOLLOW);
      die_follower( ch );
    }
}

void do_nosummon(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
    {
      if (IS_SET(ch->imm_flags,IMM_SUMMON))
      {
	send_to_char("You are no longer immune to summon.\n\r",ch);
	REMOVE_BIT(ch->imm_flags,IMM_SUMMON);
      }
      else
      {
	send_to_char("You are now immune to summoning.\n\r",ch);
	SET_BIT(ch->imm_flags,IMM_SUMMON);
      }
    }
    else
    {
      if (IS_SET(ch->act,PLR_NOSUMMON))
      {
        send_to_char("You are no longer immune to summon.\n\r",ch);
        REMOVE_BIT(ch->act,PLR_NOSUMMON);
      }
      else
      {
        send_to_char("You are now immune to summoning.\n\r",ch);
        SET_BIT(ch->act,PLR_NOSUMMON);
      }
    }
}

void do_color( CHAR_DATA *ch, char *argument )
{
    if(ch->color==TRUE)
    {
        ch->color=FALSE;
	send_to_char("You will no longer see color.\n\r",ch);
    }
    else
    {
	ch->color=TRUE;
	send_to_char("^BBYou ^BGnow ^BRsee ^BWcolor!!!^NW\n\r",ch);
    }
}


void do_scan( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    EXIT_DATA *thisroom[6];
    CHAR_DATA *ch_list;
    char *dir[] = {"right here","north","east","south","west","up","down"};
    int loop,direction=0;

    if(ch->position < POS_SLEEPING)
    {
        send_to_char("You're nearly dead! You can't look around you!\n\r", ch);
	return;
    }

    if(ch->position == POS_SLEEPING)
    {
        send_to_char("You're sleeping! You can't see anything around you. Open your eyes.\n\r",ch);
	return;
    }

    if(!check_blind(ch))
    {
	send_to_char("You're blind, you can't see a thing!\n\r", ch);
	return;
    }

    argument = one_argument( argument, arg1 );

    if(arg1[0]=='\0')
    {
	act("$n looks all around $m, scanning the horizon.",ch,NULL,NULL,TO_ROOM);
	send_to_char("Looking around, you see:\n\r",ch);
	for(loop=0;loop<6;loop++)
	   thisroom[loop]=ch->in_room->exit[loop];

	for(loop=0;loop<7;loop++)
	{
	  ch_list=NULL;
	  if(loop==0) ch_list=ch->in_room->people;
	  else if ((thisroom[loop-1] == NULL) || (thisroom[loop-1]->u1.to_room==NULL))
		continue;
		else ch_list=thisroom[loop-1]->u1.to_room->people;

	  if(IS_SET(thisroom[loop-1]->exit_info,EX_CLOSED)) continue;

	  while(ch_list!=NULL)
	  {
	     if(can_see(ch,ch_list))
	     {
		if(ch==ch_list)
	     {
	        ch_list=ch_list->next_in_room;
	        continue;
	     }

	      sprintf(buf,"%s %s %s.\n\r",(ch_list->short_descr[0]=='\0')?ch_list->name:ch_list->short_descr,loop==0?"is":"is nearby to the",dir[loop]);
	      send_to_char(buf,ch);
	     }
	   ch_list=ch_list->next_in_room;
	}
     }
   }
   else
   {
	if(UPPER(arg1[0])=='N') direction=DIR_NORTH;
	else if(UPPER(arg1[0])=='E') direction=DIR_EAST;
	else if(UPPER(arg1[0])=='S') direction=DIR_SOUTH;
	else if(UPPER(arg1[0])=='W') direction=DIR_WEST;
	else if(UPPER(arg1[0])=='U') direction=DIR_UP;
	else if(UPPER(arg1[0])=='D') direction=DIR_DOWN;
	
	if((ch->in_room->exit[direction]==NULL) || (ch->in_room->exit[direction]->u1.to_room == NULL))
	{
	   send_to_char("You stare blankly at a wall.\n\r",ch);
	   return;
	}

	if(IS_SET(ch->in_room->exit[direction]->exit_info,EX_CLOSED))
	{
	   send_to_char("You stare blankly at a closed door.\n\r",ch);
	   return;
	}
	
	ch_list=ch->in_room->exit[direction]->u1.to_room->people;

	sprintf(buf,"$n squints $s eyes and stares intently %s.",dir[direction+1]);
	act(buf,ch,NULL,NULL,TO_ROOM);
	sprintf(buf,"You focus your eyes %s, and you see:\n\r",dir[direction+1]);
	send_to_char(buf,ch);

	for(loop=0;loop<2;loop++)
	{
	  while(ch_list!=NULL)
	  {
	     if(ch==ch_list)
	     {
	        ch_list=ch_list->next_in_room;
	        continue;
	     }

	      sprintf(buf,"%s %s %s.\n\r",(ch_list->short_descr[0]=='\0')?ch_list->name:ch_list->short_descr,loop==0?"is nearby to the":"is farther off to the",dir[direction+1]);
	      send_to_char(buf,ch);
	   
	   ch_list=ch_list->next_in_room;
	}
	if((ch->in_room->exit[direction]->u1.to_room->exit[direction]==NULL) || 
 	(ch->in_room->exit[direction]->u1.to_room->exit[direction]->u1.to_room==NULL) ||
	IS_SET(ch->in_room->exit[direction]->u1.to_room->exit[direction]->exit_info,EX_CLOSED))
	   break;
	else 
	   ch_list=ch->in_room->exit[direction]->u1.to_room->exit[direction]->u1.to_room->people;
	}
}
}

void do_lore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if(ch->race != race_lookup("elf"))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    argument = one_argument(argument, arg);

    if((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    act("You size $N up and consider $M carefully.",ch,NULL,victim,TO_CHAR);

    sprintf(buf,"%s has %d/%d hitpoints, %d/%d mana, and %d/%d movement.\n\r",victim->name,victim->hit,victim->max_hit,victim->mana,victim->max_mana,victim->move,victim->max_move);

    send_to_char(buf,ch);
}

void do_look( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *in_room;
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char *pdesc;
    int door;
    int number,count;

    if ( ch->desc == NULL )
	return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
	return;
    }

    if ( !check_blind( ch ) )
	return;

    if ( !IS_NPC(ch)
    &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
    &&   room_is_dark( ch->in_room ) )
    {
	send_to_char( "It is pitch black ... \n\r", ch );
	show_char_to_char( ch->in_room->people, ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    number = number_argument(arg1,arg3);
    count = 0;

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
	/* 'look' or 'look auto' */
	send_to_char( ch->in_room->name, ch );

	if(IS_SET(ch->act,PLR_HOLYLIGHT))
	{
	    sprintf(buf," [%d]",ch->in_room->vnum);
	    send_to_char(buf,ch);
	}
	    
	send_to_char( "\n\r", ch );

	if ( arg1[0] == '\0'
	|| ( !IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF) ) )
	{
	    send_to_char( "  ",ch);
	    send_to_char( ch->in_room->description, ch );
	}

        if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) )
	{
	    send_to_char("\n\r",ch);
            do_exits( ch, "auto" );
	}

	show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
	show_char_to_char( ch->in_room->people,   ch );
	return;
    }

    if ( !str_cmp( arg1, "i" ) || !str_cmp( arg1, "in" ) )
    {
	/* 'look in' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Look in what?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}

	switch ( obj->item_type )
	{
	default:
	    send_to_char( "That is not a container.\n\r", ch );
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char( "It is empty.\n\r", ch );
		break;
	    }

	    sprintf( buf, "It's %s full of a %s liquid.\n\r",
		obj->value[1] <     obj->value[0] / 4
		    ? "less than" :
		obj->value[1] < 3 * obj->value[0] / 4
		    ? "about"     : "more than",
		liq_table[obj->value[2]].liq_color
		);

	    send_to_char( buf, ch );
	    break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		send_to_char( "It is closed.\n\r", ch );
		break;
	    }

	    act( "$p contains:", ch, obj, NULL, TO_CHAR );
	    show_list_to_char( obj->contains, ch, TRUE, TRUE );
	    break;
	case ITEM_PORTAL:
	if (obj->value[3] <= 0)
	{
	send_to_char("You can just see a dark haze.\n\r",ch);
	break;
	}
	if ((in_room = get_room_index(obj->value[3])) != NULL)
         {
		send_to_char( in_room->name, ch );
		if(IS_SET(ch->act,PLR_HOLYLIGHT))
		{
	    	    sprintf(buf," [%d]",in_room->vnum);
	    	    send_to_char(buf,ch);
		}
	    
		send_to_char( "\n\r", ch );

		if ( arg1[0] == '\0'
		|| ( !IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF) ) )
		{
	    	    send_to_char( "  ",ch);
	    	    send_to_char( in_room->description, ch );
		}

		show_list_to_char( in_room->contents, ch, FALSE, FALSE );
		show_char_to_char( in_room->people,   ch );
		break;
	}
else
{
	send_to_char("All you can see is a dark haze.\n\r",ch);
	break;
}    
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
    {
	show_char_to_char_1( victim, ch );
	return;
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}
	    	else 
                {
                    continue;
                }

 	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
 	    if ( pdesc != NULL )
 	    	if (++count == number)
 	    	{	
		    send_to_char( pdesc, ch );
		    return;
	     	}
		else
                {
                    continue;
                }

	    if ( is_name( arg3, obj->name ) )
	    	if (++count == number)
	    	{
	    	    send_to_char( obj->description, ch );
	    	    send_to_char( "\n\r",ch);
	    	   return;
	    	}
	}
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}

	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}
	}

	if ( is_name( arg3, obj->name ) )
	    if (++count == number)
	    {
	    	send_to_char( obj->description, ch );
	    	send_to_char("\n\r",ch);
	    	return;
	    }
    }
    
    if (count > 0 && count != number)
    {
    	if (count == 1)
    	    sprintf(buf,"You only see one %s here.\n\r",arg3);
    	else
    	    sprintf(buf,"You only see %d %s's here.\n\r",count,arg3);
    	
    	send_to_char(buf,ch);
    	return;
    }

    pdesc = get_extra_descr( arg1, ch->in_room->extra_descr );
    if ( pdesc != NULL )
    {
	send_to_char( pdesc, ch );
	return;
    }

         if ( !str_cmp( arg1, "n" ) || !str_cmp( arg1, "north" ) ) door = 0;
    else if ( !str_cmp( arg1, "e" ) || !str_cmp( arg1, "east"  ) ) door = 1;
    else if ( !str_cmp( arg1, "s" ) || !str_cmp( arg1, "south" ) ) door = 2;
    else if ( !str_cmp( arg1, "w" ) || !str_cmp( arg1, "west"  ) ) door = 3;
    else if ( !str_cmp( arg1, "u" ) || !str_cmp( arg1, "up"    ) ) door = 4;
    else if ( !str_cmp( arg1, "d" ) || !str_cmp( arg1, "down"  ) ) door = 5;
    else
    {
	send_to_char( "You do not see that here.\n\r", ch );
	return;
    }

    /* 'look direction' */
    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	send_to_char( "Nothing special there.\n\r", ch );
	return;
    }

    if ( pexit->description != NULL && pexit->description[0] != '\0' )
	send_to_char( pexit->description, ch );
    else
	send_to_char( "Nothing special there.\n\r", ch );

    if ( pexit->keyword    != NULL
    &&   pexit->keyword[0] != '\0'
    &&   pexit->keyword[0] != ' ' )
    {
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	}
	else if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
	{
	    act( "The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR );
	}
    }

    return;
}

/* RT added back for the hell of it */
void do_read (CHAR_DATA *ch, char *argument )
{
    do_look(ch,argument);
}

void do_examine( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Examine what?\n\r", ch );
	return;
    }

    do_look( ch, arg );

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	switch ( obj->item_type )
	{
	default:
	    break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    send_to_char( "When you look inside, you see:\n\r", ch );
	    sprintf( buf, "in %s", arg );
	    do_look( ch, buf );
	}
    }

    return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA *ch, char *argument )
{
    extern char * const dir_name[];
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;
    int door;

    buf[0] = '\0';
    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

    strcpy( buf, fAuto ? "[Exits:" : "Obvious exits:\n\r" );

    found = FALSE;
    for ( door = 0; door <= 5; door++ )
    {
	if ( ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   can_see_room(ch,pexit->u1.to_room)
	&&   !IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    found = TRUE;
	    if ( fAuto )
	    {
		strcat( buf, " " );
		strcat( buf, dir_name[door] );
	    }
	    else
	    {
		sprintf( buf + strlen(buf), "%-5s - %s\n\r",
		    capitalize( dir_name[door] ),
		    room_is_dark( pexit->u1.to_room )
			?  "Too dark to tell"
			: pexit->u1.to_room->name
		    );
	    }
	}
    }

    if ( !found )
	strcat( buf, fAuto ? " none" : "None.\n\r" );

    if ( fAuto )
	strcat( buf, "]\n\r" );

    send_to_char( buf, ch );
    return;
}

void do_worth( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
    {
	sprintf(buf,"You have %ld gold on hand and %ld gold in the bank.\n\r",ch->gold,ch->balance);
	send_to_char(buf,ch);
	return;
    }

    sprintf(buf,"You have %ld gold on hand, %ld gold in the bank.\n\rYou have %ld experience (%ld exp to level).\n\r",ch->gold,ch->balance,ch->exp,(ch->level+1) * exp_per_level(ch,ch->points) - ch->exp);
    send_to_char(buf,ch);

    return;
}


void do_safe( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    long time_to_safe=0;

    time_to_safe = (time(NULL) - ch->pk_timer);
    
    if(time_to_safe < 90)
    {
	sprintf(buf,"You are not safe yet. You must wait %d more seconds.\n\r",90-(int)time_to_safe);
	send_to_char(buf,ch);
    }
    else
	send_to_char("You are safe. For the time being.\n\r",ch);
}

/*
sprintf(buf,"You are %s, a %s%s %s%s\n\r",ch->name,race_table[ch->race].name,IS_NPC(ch)?"mobile":class_table[ch->class].name,IS_NPC(ch)?".":(ch->pcdata->guild!=0)?", and a member of ":".",(!IS_NPC(ch) && ch->pcdata->guild!=0)?guild_table[ch->pcdata->guild
































].screen:"");
strcat(buf,"----------------------------------------------------------------------\n\r");
strcat(buf,"Level    : %-13dExp Earnd: %d\n\r",ch->level,ch->exp);
strcat(buf,"Age      : %-13dExp Left : %d\n\r",get_age(ch),((ch->level + 1) * exp_per_level(ch,ch->points) - ch->exp));
strcat(buf,"----------------------------------------------------------------------\n\r");
strcat(buf,"Practices: %-13dGold     : %-13ldHungry  : %s\n\r",ch->practices,ch->gold,(ch->full==0)?"Yes":"No");
strcat(buf,"Trains   : %-13dBalance  : %-13ldThirsty : %s\n\r",ch->trains,ch->balance,(ch->thirst==0)?"Yes":"No");
strcat(buf,"----------------------------------------------------------------------\n\r");
strcat(buf,"Hitpoints: %-6d/%-6dMana     : %-6d/%-6dMove    : %-6d/%d\n\r",ch->hit,ch->max_hit,ch->mana,ch->max_mana,ch->move,ch->max_move);
strcat(buf,"----------------------------------------------------------------------\n\r");
strcat(buf,"Pierce   : %-13dSaves    : %-13dDeaths  : %d\n\r",GET_AC(AC_PIERCE),ch->saving_throw,IS_NPC(ch)?0:ch->pkstat[0]);
strcat(buf,"Bash     : %-13dHitroll  : %-13dPkills  : %d\n\r",GET_AC(AC_BASH), ch->hitroll, IS_NPC(ch)?0:ch->pkdata[1]);
strcat(buf,"Slash    : %-13dDamroll  : %-13dPKRate  : %d\n\r",GET_AC(AC_SLASH), ch->damroll, IS_NPC(ch)?0:ch->pkstat[2]);
strcat(buf,"Magic    : %-13dWimpy    : %-13dAlign   : %d\n\r",GET_AC(AC_MAGIC), ch->wimpy, ch->alignment);
strcat(buf,"----------------------------------------------------------------------\n\r");
strcat(buf,"      Str: %2d(%2d) Int: %2d(%2d) Wis: %2d(%2d) Dex: %2d(%2d) Con: %2d(%2d)\n\r",ch->perm_stat[STAT_STR],get_curr_stat(ch,STAT_STR),ch->perm_stat[STAT_INT],get_curr_stat(ch,STAT_INT),ch->perm_stat[STAT_WIS],get_curr_stat(ch,STAT_WIS),ch->perm_
































stat[STAT_DEX],get_curr_stat(ch,STAT_DEX),ch->perm_stat[STAT_CON],get_curr_stat(ch,STAT_CON));
strcat(buf,"----------------------------------------------------------------------\n\r");
strcat(buf,"[Hit Return for Spell Affects...]");
*/

void do_score( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char trust[10];
    char gender[10];
    char weight[100];
    char hp[100];
    char mana[100];
    char move[100];

    sprintf(trust,"(%d)",ch->trust);
    switch(ch->sex)
    {
	case SEX_MALE : sprintf(gender,"Male"); break;
	case SEX_FEMALE : sprintf(gender,"Female"); break;
	case SEX_NEUTRAL : sprintf(gender,"Neuter"); break;
    }
    
    sprintf(weight,"%d/%d",ch->carry_weight,can_carry_w(ch));
    sprintf(hp,"%d/%d",ch->hit,ch->max_hit);
    sprintf(mana,"%d/%d",ch->mana,ch->max_mana);
    sprintf(move,"%d/%d",ch->move,ch->max_move);


sprintf(buf,"You are %s, a %s %s %s%s%s\n\r",ch->name,gender,
	race_table[ch->race].name,
        IS_NPC(ch)?"mobile":class_table[ch->class].name,
        IS_NPC(ch)?".":(ch->pcdata->guild!=0)?", and a member of ":".",
        (!IS_NPC(ch) && ch->pcdata->guild!=0)?guild_table[ch->pcdata->guild].screen:"");
send_to_char(buf,ch);

send_to_char("----------------------------------------------------------------------\n\r",ch);

sprintf(buf,"Level    : %-8d%5sExp Earnd: %ld\n\r",ch->level,(ch->trust>ch->level)?trust:"",ch->exp);
send_to_char(buf,ch);
if (!IS_NPC(ch))
{
sprintf(buf,"Age      : %-13dExp Left : %ld\n\r",get_age(ch),((ch->level + 1) * exp_per_level(ch,ch->points) - ch->exp));
send_to_char(buf,ch);
}
send_to_char("----------------------------------------------------------------------\n\r",ch);

sprintf(buf,"Practices: %-13dGold     : %-13ldHungry  : %s\n\r",ch->practice,ch->gold,(!IS_NPC(ch) && ch->pcdata->condition[COND_FULL]==0)?"Yes":"No");
send_to_char(buf,ch);

sprintf(buf,"Trains   : %-13dBalance  : %-13ldThirsty : %s\n\r",ch->train,ch->balance,(!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST]==0)?"Yes":"No");
send_to_char(buf,ch);

sprintf(buf,"Class Lvl: %-13dWeight   : %-13sDrunk   : %s\n\r",ch->class-((ch->class%3)*3),weight,(!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]>10)?"Yes":"No"); send_to_char(buf,ch);

send_to_char("----------------------------------------------------------------------\n\r",ch);

sprintf(buf,"Hitpoints: %-13sMana     : %-13sMove    : %-13s\n\r",hp,mana,move);
send_to_char(buf,ch);
sprintf(buf,"Naked: Hp: %-13dMana     : %-13dMove    : %-13d\n\r",
ch->real_hit,ch->real_mana,ch->real_move);
send_to_char(buf,ch);


send_to_char("----------------------------------------------------------------------\n\r",ch);

sprintf(buf,"Pierce   : %-13dSaves    : %-13dDeaths  : %d\n\r",GET_AC(ch,AC_PIERCE),ch->saving_throw,IS_NPC(ch)?0:ch->pkstat[0]);
send_to_char(buf,ch);
sprintf(buf,"Bash     : %-13dHitroll  : %-13dPkills  : %d\n\r",GET_AC(ch,AC_BASH), ch->hitroll, IS_NPC(ch)?0:ch->pkstat[1]);
send_to_char(buf,ch);
sprintf(buf,"Slash    : %-13dDamroll  : %-13dPKRate  : %d\n\r",GET_AC(ch,AC_SLASH), ch->damroll, IS_NPC(ch)?0:ch->pkstat[2]);
send_to_char(buf,ch);
sprintf(buf,"Magic    : %-13dWimpy    : %-13dAlign   : %d\n\r",GET_AC(ch,AC_EXOTIC), ch->wimpy, ch->alignment);
send_to_char(buf,ch);

send_to_char("----------------------------------------------------------------------\n\r",ch);

sprintf(buf,"      Str: %2d(%2d) Int: %2d(%2d) Wis: %2d(%2d) Dex: %2d(%2d) Con: %2d(%2d)\n\r",
 	ch->perm_stat[STAT_STR],get_curr_stat(ch,STAT_STR),ch->perm_stat[STAT_INT],get_curr_stat(ch,STAT_INT),ch->perm_stat[STAT_WIS],
	get_curr_stat(ch,STAT_WIS), ch->perm_stat[STAT_DEX],get_curr_stat(ch,STAT_DEX),
	ch->perm_stat[STAT_CON],get_curr_stat(ch,STAT_CON));
send_to_char(buf,ch);

send_to_char("----------------------------------------------------------------------\n\r",ch);

send_to_char("To see what spells affect you, type \"affects.\"\n\r",ch);

return;
}

/*void do_score( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int i;

    sprintf( buf,
	"You are %s%s, level %d, %d years old (%d hours).\n\r",
	ch->name,
	IS_NPC(ch) ? "" : ch->pcdata->title,
	ch->level, get_age(ch),
        ( ch->played + (int) (current_time - ch->logon) ) / 3600);
    send_to_char( buf, ch );

    if ( get_trust( ch ) != ch->level )
    {
	sprintf( buf, "You are trusted at level %d.\n\r",
	    get_trust( ch ) );
	send_to_char( buf, ch );
    }

    sprintf(buf, "Race: %s  Sex: %s  Class:  %s\n\r",
	race_table[ch->race].name,
	ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
 	IS_NPC(ch) ? "mobile" : class_table[ch->class].name);
    send_to_char(buf,ch);
	

    sprintf( buf,
	"You have %d/%d hit, %d/%d mana, %d/%d movement.\n\r",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move);
    send_to_char( buf, ch );

    sprintf( buf,
	"You have %d practices and %d training sessions.\n\r",
	ch->practice, ch->train);
    send_to_char( buf, ch );

    sprintf( buf,
	"You are carrying %d/%d items with weight %d/%d pounds.\n\r",
	ch->carry_number, can_carry_n(ch),
	ch->carry_weight, can_carry_w(ch) );
    send_to_char( buf, ch );

    sprintf( buf,
	"Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
	ch->perm_stat[STAT_STR],
	get_curr_stat(ch,STAT_STR),
	ch->perm_stat[STAT_INT],
	get_curr_stat(ch,STAT_INT),
	ch->perm_stat[STAT_WIS],
	get_curr_stat(ch,STAT_WIS),
	ch->perm_stat[STAT_DEX],
	get_curr_stat(ch,STAT_DEX),
	ch->perm_stat[STAT_CON],
	get_curr_stat(ch,STAT_CON) );
    send_to_char( buf, ch );

    sprintf( buf,
	"You have scored %ld exp, and have %ld gold on hand, and %ld in the bank.\n\r",ch->exp,  ch->gold, ch->balance );
    send_to_char( buf, ch );
    if(ch->level >= 30)
    {
    	sprintf( buf, "Saves: %d\n\r",ch->saving_throw );
    	send_to_char(buf, ch);
    }
    if (!IS_NPC(ch) && ch->level < LEVEL_HERO)
    {
      sprintf (buf, 
	"You need %ld exp to level.\n\r",
	((ch->level + 1) * exp_per_level(ch,ch->points) - ch->exp));
      send_to_char( buf, ch );
     }

    sprintf( buf, "Wimpy set to %d hit points.\n\r", ch->wimpy );
    send_to_char( buf, ch );

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]   > 10 )
	send_to_char( "You are drunk.\n\r",   ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] ==  0 )
	send_to_char( "You are thirsty.\n\r", ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   ==  0 )
	send_to_char( "You are hungry.\n\r",  ch );

    switch ( ch->position )
    {
    case POS_DEAD:     
	send_to_char( "You are DEAD!!\n\r",		ch );
	break;
    case POS_MORTAL:
	send_to_char( "You are mortally wounded.\n\r",	ch );
	break;
    case POS_INCAP:
	send_to_char( "You are incapacitated.\n\r",	ch );
	break;
    case POS_STUNNED:
	send_to_char( "You are stunned.\n\r",		ch );
	break;
    case POS_SLEEPING:
	send_to_char( "You are sleeping.\n\r",		ch );
	break;
    case POS_RESTING:
	send_to_char( "You are resting.\n\r",		ch );
	break;
    case POS_STANDING:
	send_to_char( "You are standing.\n\r",		ch );
	break;
    case POS_FIGHTING:
	send_to_char( "You are fighting.\n\r",		ch );
	break;
    }


    if (ch->level >= 25)
    {	
	sprintf( buf,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
		 GET_AC(ch,AC_PIERCE),
		 GET_AC(ch,AC_BASH),
		 GET_AC(ch,AC_SLASH),
		 GET_AC(ch,AC_EXOTIC));
    	send_to_char(buf,ch);
    }

    for (i = 0; i < 4; i++)
    {
	char * temp;

	switch(i)
	{
	    case(AC_PIERCE):	temp = "piercing";	break;
	    case(AC_BASH):	temp = "bashing";	break;
	    case(AC_SLASH):	temp = "slashing";	break;
	    case(AC_EXOTIC):	temp = "magic";		break;
	    default:		temp = "error";		break;
	}
	
	send_to_char("You are ", ch);

	if      (GET_AC(ch,i) >=  101 ) 
	    sprintf(buf,"hopelessly vulnerable to %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 80) 
	    sprintf(buf,"defenseless against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 60)
	    sprintf(buf,"barely protected from %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 40)
	    sprintf(buf,"slighty armored against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 20)
	    sprintf(buf,"somewhat armored against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 0)
	    sprintf(buf,"armored against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -20)
	    sprintf(buf,"well-armored against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -40)
	    sprintf(buf,"very well-armored against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -60)
	    sprintf(buf,"heavily armored against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -80)
	    sprintf(buf,"superbly armored against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -100)
	    sprintf(buf,"almost invulnerable to %s.\n\r",temp);
	else
	    sprintf(buf,"divinely armored against %s.\n\r",temp);

	send_to_char(buf,ch);
    }


    if ( IS_IMMORTAL(ch))
    {
      send_to_char("Holy Light: ",ch);
      if (IS_SET(ch->act,PLR_HOLYLIGHT))
        send_to_char("on",ch);
      else
        send_to_char("off",ch);
 
      if (IS_SET(ch->act,PLR_WIZINVIS))
      {
        sprintf( buf, "  Invisible: level %d",ch->invis_level);
        send_to_char(buf,ch);
      }
      send_to_char("\n\r",ch);
    }

    if ( ch->level >= 15 )
    {
	sprintf( buf, "Hitroll: %d  Damroll: %d.\n\r",
	    GET_HITROLL(ch), GET_DAMROLL(ch) );
	send_to_char( buf, ch );
    }
    
    if ( ch->level >= 10 )
    {
	sprintf( buf, "Alignment: %d.  ", ch->alignment );
	send_to_char( buf, ch );
    }

    send_to_char( "You are ", ch );
         if ( ch->alignment >  900 ) send_to_char( "angelic.\n\r", ch );
    else if ( ch->alignment >  700 ) send_to_char( "saintly.\n\r", ch );
    else if ( ch->alignment >  350 ) send_to_char( "good.\n\r",    ch );
    else if ( ch->alignment >  100 ) send_to_char( "kind.\n\r",    ch );
    else if ( ch->alignment > -100 ) send_to_char( "neutral.\n\r", ch );
    else if ( ch->alignment > -350 ) send_to_char( "mean.\n\r",    ch );
    else if ( ch->alignment > -700 ) send_to_char( "evil.\n\r",    ch );
    else if ( ch->alignment > -900 ) send_to_char( "demonic.\n\r", ch );
    else                             send_to_char( "satanic.\n\r", ch );
    
    sprintf( buf, "Deaths: %d, Kills: %d, Rating: %d\n\r",ch->pkstat[0],ch->pkstat[1],ch->pkstat[2]);
    send_to_char(buf, ch);
    send_to_char("To see what spells affect you, type affects at the prompt.\n\r",ch);

    return;
}
*/
void do_affects( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf;
    char buf[MAX_STRING_LENGTH];

    if ( ch->affected != NULL )
    {
	send_to_char( "You are affected by:\n\r", ch );
	for ( paf = ch->affected; paf != NULL; paf = paf->next )
	{
	    sprintf( buf, "Spell: '%s'", skill_table[paf->type].name );
	    send_to_char( buf, ch );

	    if ( ch->level >= 20 )
	    {
		sprintf( buf,
		    " modifies %s by %d for %d hours",
		    affect_loc_name( paf->location ),
		    paf->modifier,
		    paf->duration );
		send_to_char( buf, ch );
	    }

	    send_to_char( ".\n\r", ch );
	}
    }
    else
    	send_to_char("You are not affected by anything at this time.\n\r",ch);
}

void do_finger( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH],buf[100];
    CHAR_DATA *victim;    

    argument = one_argument( argument, arg );
    victim = get_char_world(ch, arg);

    if(victim == NULL)
    {
	send_to_char("They aren't on.\n\r",ch);
	return;
    }

    sprintf(buf, "^BK<^NW Name: %s%s ^BK>^NW\n\r",IS_NPC(victim)?victim->short_descr:victim->name,IS_NPC(victim)?"":victim->pcdata->title);
    send_to_char(buf, ch);
    sprintf(buf, "^BK<^NW Level: %d, Guild: %s, Age: %d years old (%d hours) ^BK>^NW\n\r",victim->level,IS_NPC(victim)?"None":guild_table[victim->pcdata->guild].screen,get_age(victim),(victim->played + (int) (current_time - victim->logon) ) / 3600);
    send_to_char(buf, ch);
    sprintf(buf, "^BK< ^NWRace: %s, Align %d, Sex: ",race_table[victim->race].name,victim->alignment);
    switch(victim->sex)
    {
	case SEX_MALE: strcat(buf,"Male"); break;
   	case SEX_FEMALE: strcat(buf,"Female"); break;
	case SEX_NEUTRAL: strcat(buf, "Neutral"); break;
    }
    strcat(buf,", Class: ");
    strcat(buf,class_table[victim->class].name);
    strcat(buf," ^BK>^NW\n\r");
    send_to_char(buf, ch);
    sprintf(buf,"^BK<^NW PDeaths: %d, PKills: %d, PRating: %d ^BK>^NW\n\r",victim->pkstat[0],victim->pkstat[1],victim->pkstat[2]);
    send_to_char(buf,ch);
}


char *	const	day_name	[] =
{
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char *	const	month_name	[] =
{
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time( CHAR_DATA *ch, char *argument )
{
    extern char str_boot_time[];
    char buf[MAX_STRING_LENGTH];
    char *suf;
    int day;

    day     = time_info.day + 1;

         if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    sprintf( buf,
	"It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\rROM started up at %s\rThe system time is %s\r",

	(time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
	time_info.hour >= 12 ? "pm" : "am",
	day_name[day % 7],
	day, suf,
	month_name[time_info.month],
	str_boot_time,
	(char *) ctime( &current_time )
	);

    send_to_char( buf, ch );
    return;
}



void do_weather( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    static char * const sky_look[4] =
    {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"
    };

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You can't see the weather indoors.\n\r", ch );
	return;
    }

    sprintf( buf, "The sky is %s and %s.\n\r",
	sky_look[weather_info.sky],
	weather_info.change >= 0
	? "a warm southerly breeze blows"
	: "a cold northern gust blows"
	);
    send_to_char( buf, ch );
    return;
}



void do_help( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];

    if ( argument[0] == '\0' )
	argument = "summary";

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0' )
    {
	argument = one_argument(argument,argone);
	if (argall[0] != '\0')
	    strcat(argall," ");
	strcat(argall,argone);
    }

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
	if ( pHelp->level > get_trust( ch ) )
	    continue;

	if ( is_name( argall, pHelp->keyword ) )
	{
	    if ( pHelp->level >= 0 && str_cmp( argall, "imotd" ) )
	    {
		send_to_char( pHelp->keyword, ch );
		send_to_char( "\n\r", ch );
	    }

	    /*
	     * Strip leading '.' to allow initial blanks.
	     */
	    if ( pHelp->text[0] == '.' )
		page_to_char( pHelp->text+1, ch );
	    else
		page_to_char( pHelp->text  , ch );
	    return;
	}
    }

    send_to_char( "No help on that word.\n\r", ch );
    return;
}


/* whois command */
void do_whois (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char output[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    bool found = FALSE;
    char color[30];

    color[0]='\0';

    one_argument(argument,arg);
  
    if (arg[0] == '\0')
    {
	send_to_char("You must provide a name.\n\r",ch);
	return;
    }


    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;
	char const *class;
        char virt[100];

 	if (d->connected != CON_PLAYING || !can_see(ch,d->character))
	    continue;
	
	wch = ( d->original != NULL ) ? d->original : d->character;

 	if (!can_see(ch,wch))
	    continue;

	if (!str_prefix(arg,wch->name))
	{
	    found = TRUE;
	    
	    /* work out the printing */
	    class = class_table[wch->class].who_name;

	    sprintf(virt,"[^BR%2d^NW]",wch->pcdata->virtual_level[0]);
	    
            if(wch->level > MAX_LEVEL - 9)
	    {
		switch(wch->level)
		{
		case MAX_LEVEL - 0 : class = "^BWIMP^NW"; 	break;
		case MAX_LEVEL - 1 : class = "^BGCRE^NW";	break;
		case MAX_LEVEL - 2 : class = "^BBSUP^NW";	break;
		case MAX_LEVEL - 3 : class = "^BCDEI^NW";	break;
		case MAX_LEVEL - 4 : class = "^BRGOD^NW";	break;
		case MAX_LEVEL - 5 : class = "^NGIMM^NW";	break;
		case MAX_LEVEL - 6 : class = "^NBDEM^NW";	break;
		case MAX_LEVEL - 7 : class = "^NCANG^NW";	break;
		case MAX_LEVEL - 8 : class = "^NRAVA^NW";	break;
	        }

	    sprintf(color,"^NW");
	    switch( wch->dracspell )
	    {
		case DRAC_ACID: sprintf(color, "^BK"); break;
		case DRAC_LIGHTNING: sprintf(color, "^BB"); break;
		case DRAC_FROST: sprintf(color, "^BW"); break;
		case DRAC_FIRE: sprintf(color, "^BR"); break;
		case DRAC_GAS: sprintf(color, "^BG"); break;
	    }

	    sprintf(buf, "[%2d %s%s^NW %s] %s%s%s%s%s%s%s%s%s^NW\n\r",
		wch->level,
		color,
		wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name
					: "     ",
		class,
		IS_SET(wch->act,PLR_IMMQUEST)?"(^BGQ^NW) ":"",
		wch->pcdata->virtual_level[0]>wch->level?virt:"",
		guild_table[wch->pcdata->guild].screen,
		wch->pcdata->guild==0?"":" ",
                IS_SET(wch->comm,COMM_AFK)?"[^NBAFK^NW]":"",
		IS_SET(wch->act,PLR_KILLER) ? "(^NRKILLER^NW) " : "",
		IS_SET(wch->act,PLR_THIEF) ? "(^BYTHIEF^NW) " : "",
		wch->name, IS_NPC(wch) ? "" : wch->pcdata->title);
	    strcat(output,buf);
	    }
	    else
            {
	    /* a little formatting */
	    sprintf(buf, "[%2d %s %s] %s%s%s%s%s%s%s%s%s\n\r",
		wch->level,
		wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name
					: "     ",
		class,
		IS_SET(wch->act,PLR_IMMQUEST)?"(^BGQ^NW) ":"",
		wch->pcdata->virtual_level[0]>wch->level?virt:"",
		guild_table[wch->pcdata->guild].screen,
		wch->pcdata->guild==0?"":" ",
                IS_SET(wch->comm,COMM_AFK)?"[^NBAFK^NW]":"",
		IS_SET(wch->act,PLR_KILLER) ? "(^NRKILLER^NW) " : "",
		IS_SET(wch->act,PLR_THIEF) ? "(^BYTHIEF^NW) " : "",
		wch->name, IS_NPC(wch) ? "" : wch->pcdata->title);
		    strcat(output,buf);
	 	}
	}
    }

    if (!found)
    {
	send_to_char("No one of that name is playing.\n\r",ch);
	return;
    }

    page_to_char(output,ch);
}


/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
void do_who( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char output[4 * MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int iClass;
    int iRace;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    int guild=0;
    bool rgfClass[MAX_CLASS];
    bool rgfRace[MAX_PC_RACE];
    bool fClassRestrict;
    bool fRaceRestrict;
    bool fImmortalOnly;

    /*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    fClassRestrict = FALSE;
    fRaceRestrict = FALSE;
    fImmortalOnly  = FALSE;
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	rgfClass[iClass] = FALSE;
    for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
	rgfRace[iRace] = FALSE;

    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
	char arg[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	    break;

	if ( is_number( arg ) )
	{
	    switch ( ++nNumber )
	    {
	    case 1: iLevelLower = atoi( arg ); break;
	    case 2: iLevelUpper = atoi( arg ); break;
	    default:
		send_to_char( "Only two level numbers allowed.\n\r", ch );
		return;
	    }
	}
	else
	{

	    /*
	     * Look for classes to turn on.
	     */
	    if ( arg[0] == 'i' )
	    {
		fImmortalOnly = TRUE;
	    }
	    else
	    {
		guild = guild_lookup(arg);

		if(guild==0)
		{
		iClass = class_lookup(arg);
		if (iClass == -1)
		{
		    iRace = race_lookup(arg);

		    if (iRace == 0 || iRace >= MAX_PC_RACE)
		    {
			send_to_char( 
			    "That's not a valid race or class.\n\r",ch);
			return;
		    }
		    else
		    {
			fRaceRestrict = TRUE;
			rgfRace[iRace] = TRUE;
		    }
		}
		else
		{
		    fClassRestrict = TRUE;
		    rgfClass[iClass] = TRUE;
		}
		}
	    } 
	}  
    }

    /*
     * Now show matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    output[0] = '\0';

    output[0] = '\0';
    sprintf(output,"                                 ^NRRi^BRve^NWrs ^BWof^NW Bl^BRoo^NRd!^NW\n\r");
    strcat(output,"                                 ~~~~~~~~~~~~~~~~\n\r");

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *wch;
	char const *class;
	char virt[100];
	char color[100];

	/*
	 * Check for match against restrictions.
	 * Don't use trust as that exposes trusted mortals.
	 */
	if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
	    continue;

        if(IS_NPC(d->character))
	    continue;

	if(guild!=0)
	    if(d->character->pcdata->guild!=guild) continue;

	wch   = ( d->original != NULL ) ? d->original : d->character;
	if ( wch->level < iLevelLower
	||   wch->level > iLevelUpper
	|| ( fImmortalOnly  && wch->level < LEVEL_HERO )
	|| ( fClassRestrict && !rgfClass[wch->class] ) 
	|| ( fRaceRestrict && !rgfRace[wch->race]))
	    continue;

	nMatch++;

	/*
	 * Figure out what to print for class.
	 */
	class = class_table[wch->class].who_name;
	switch ( wch->level )
	{
	default: break;
            {
		case MAX_LEVEL - 0 : class = "^BWIMP^NW"; 	break;
		case MAX_LEVEL - 1 : class = "^BGCRE^NW";	break;
		case MAX_LEVEL - 2 : class = "^BBSUP^NW";	break;
		case MAX_LEVEL - 3 : class = "^BCDEI^NW";	break;
		case MAX_LEVEL - 4 : class = "^BRGOD^NW";	break;
		case MAX_LEVEL - 5 : class = "^NGIMM^NW";	break;
		case MAX_LEVEL - 6 : class = "^NBDEM^NW";	break;
		case MAX_LEVEL - 7 : class = "^NCANG^NW";	break;
		case MAX_LEVEL - 8 : class = "^NRAVA^NW";	break;
            }
	}


        if(!IS_NPC(wch))
	   sprintf(virt,"[^BR%2d^NW]",wch->pcdata->virtual_level[0]);
        else strcpy(virt,"");

	sprintf(color,"^NW");
	switch( wch->dracspell )
	{
	    case DRAC_ACID: sprintf(color, "^BK"); break;
	    case DRAC_LIGHTNING: sprintf(color, "^BB"); break;
	    case DRAC_FROST: sprintf(color, "^BW"); break;
	    case DRAC_FIRE: sprintf(color, "^BR"); break;
	    case DRAC_GAS: sprintf(color, "^BG"); break;
	}

	/*
	 * Format it up.
	 */
	sprintf( buf, "[%2d %s%s^NW %s] %s%s%s%s%s%s%s%s%s%s^NW\n\r",
	    wch->level,
	    color,
	    wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name 
				    : "     ",
	    class,
	    IS_SET(wch->act,PLR_IMMQUEST)?"(^BGQ^NW) ":"",
	    wch->new_char==FALSE?"[^BBRELEVEL ME^NW]":"",
	    wch->pcdata->virtual_level[0]>wch->level?virt:"",
	    guild_table[wch->pcdata->guild].screen,
	    wch->pcdata->guild==0?"":" ",
            IS_SET(wch->comm,COMM_AFK)?"[^NBAFK^NW]":"",
	    IS_SET(wch->act, PLR_KILLER) ? "(^BRKILLER^NW) " : "",
	    IS_SET(wch->act, PLR_THIEF)  ? "(^BYTHIEF^NW) "  : "",
	    wch->name,
	    IS_NPC(wch) ? "" : wch->pcdata->title );
	strcat(output,buf);
    }

    sprintf( buf2, "\n\rPlayers found: %d\n\r", nMatch );
    strcat(output,buf2);
    page_to_char( output, ch );
    return;
}

void do_count ( CHAR_DATA *ch, char *argument )
{
    int count;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    count = 0;

    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING && can_see( ch, d->character ) )
	    count++;

    max_on = UMAX(count,max_on);

    if (max_on == count)
        sprintf(buf,"There are %d characters on, the most so far today.\n\r",
	    count);
    else
	sprintf(buf,"There are %d characters on, the most on today was %d.\n\r",
	    count,max_on);

    send_to_char(buf,ch);
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
    send_to_char( "You are carrying:\n\r", ch );
    show_list_to_char( ch->carrying, ch, TRUE, TRUE );
    return;
}



void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear;
    bool found;

    send_to_char( "You are using:\n\r", ch );
    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {

        if (iWear == WEAR_HANDS && ((obj = get_eq_char(ch,WEAR_TATTOO)) != NULL))
        {

	    send_to_char( where_name[WEAR_TATTOO], ch );
	    if ( can_see_obj( ch, obj ) )
	    {
	    	send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    	send_to_char( "\n\r", ch );
	    }
	    else
	    {
	    	send_to_char( "something.\n\r", ch );
	    }
	    found = TRUE;
	}

	if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	    continue;

	send_to_char( where_name[iWear], ch );
	if ( can_see_obj( ch, obj ) )
	{
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
	else
	{
	    send_to_char( "something.\n\r", ch );
	}
	found = TRUE;
    }

    if ( !found )
	send_to_char( "Nothing.\n\r", ch );

    return;
}



void do_compare( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Compare what to what?\n\r", ch );
	return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if (arg2[0] == '\0')
    {
	for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
	{
	    if (obj2->wear_loc != WEAR_NONE
	    &&  can_see_obj(ch,obj2)
	    &&  obj1->item_type == obj2->item_type
	    &&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if (obj2 == NULL)
	{
	    send_to_char("You aren't wearing anything comparable.\n\r",ch);
	    return;
	}
    } 

    else if ( (obj2 = get_obj_carry(ch,arg2) ) == NULL )
    {
	send_to_char("You do not have that item.\n\r",ch);
	return;
    }

    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if ( obj1 == obj2 )
    {
	msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "You can't compare $p and $P.";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
	    msg = "You can't compare $p and $P.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
	    break;

	case ITEM_WEAPON:
	    if (obj1->pIndexData->new_format)
		value1 = (1 + obj1->value[2]) * obj1->value[1];
	    else
	    	value1 = obj1->value[1] + obj1->value[2];

	    if (obj2->pIndexData->new_format)
		value2 = (1 + obj2->value[2]) * obj2->value[1];
	    else
	    	value2 = obj2->value[1] + obj2->value[2];
	    break;
	}
    }

    if ( msg == NULL )
    {
	     if ( value1 == value2 ) msg = "$p and $P look about the same.";
	else if ( value1  > value2 ) msg = "$p looks better than $P.";
	else                         msg = "$p looks worse than $P.";
    }

    act( msg, ch, obj1, obj2, TO_CHAR );
    return;
}



void do_credits( CHAR_DATA *ch, char *argument )
{
    do_help( ch, "diku" );
    return;
}



void do_where( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;
    char string[MAX_STRING_LENGTH];
    char string2[MAX_STRING_LENGTH];
    int counter,i;

    one_argument( argument, arg );

    strcpy(string,ch->in_room->area->name);
    counter=0;
    for(counter=0;string[counter]!='}' && string[counter]!='\0';counter++);
    counter++;
    for(;string[counter]==' ';counter++);
    counter++;
    for(;string[counter]!=' ';counter++);
    counter++;
    for(;string[counter]==' ';counter++);
    for(i=0;i<strlen(string)-counter;i++)
	string2[i]=string[i+counter];
    string2[i]='\0';

    if ( arg[0] == '\0' )
    {
	sprintf(buf, "Players near you in the area %s:\n\r",string2);
	send_to_char(buf,  ch );
	found = FALSE;
	for ( d = descriptor_list; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    && ( victim = d->character ) != NULL
	    &&   !IS_NPC(victim)
	    &&   victim->in_room != NULL
	    &&   victim->in_room->area == ch->in_room->area
	    &&   can_see( ch, victim ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
		    victim->name, victim->in_room->name );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "None\n\r", ch );
    }
    else
    {
	found = FALSE;
	for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	    if ( victim->in_room != NULL
	    &&   victim->in_room->area == ch->in_room->area
	    &&   !IS_AFFECTED(victim, AFF_HIDE)
	    &&   !IS_AFFECTED(victim, AFF_SNEAK)
	    &&   can_see( ch, victim )
	    &&   is_name( arg, victim->name ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
		    PERS(victim, ch), victim->in_room->name );
		send_to_char( buf, ch );
		break;
	    }
	}
	if ( !found )
	    act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }

    return;
}




void do_consider( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg;
    int diff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Consider killing whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }

    if (is_safe(ch,victim))
    {
	send_to_char("Don't even think about it.\n\r",ch);
	return;
    }

    diff = victim->level - ch->level;

         if ( diff <= -10 ) msg = "You can kill $N naked and weaponless.";
    else if ( diff <=  -5 ) msg = "$N is no match for you.";
    else if ( diff <=  -2 ) msg = "$N looks like an easy kill.";
    else if ( diff <=   1 ) msg = "The perfect match!";
    else if ( diff <=   4 ) msg = "$N says 'Do you feel lucky, punk?'.";
    else if ( diff <=   9 ) msg = "$N laughs at you mercilessly.";
    else                    msg = "Death will thank you for your gift.";

    act( msg, ch, NULL, victim, TO_CHAR );
    return;
}



void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?' )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
    {
	strcpy( buf, title );
    }

    free_string( ch->pcdata->title );
    ch->pcdata->title = str_dup( buf );
    return;
}



void do_title( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your title to what?\n\r", ch );
	return;
    }

    if ( strlen(argument) > 45 )
	argument[45] = '\0';

    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char( "Ok.\n\r", ch );
}



void do_description( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] != '\0' )
    {
	buf[0] = '\0';
	smash_tilde( argument );
	if ( argument[0] == '+' )
	{
	    if ( ch->description != NULL )
		strcat( buf, ch->description );
	    argument++;
	    while ( isspace(*argument) )
		argument++;
	}

	if ( strlen(buf) + strlen(argument) >= MAX_STRING_LENGTH - 2 )
	{
	    send_to_char( "Description too long.\n\r", ch );
	    return;
	}

	strcat( buf, argument );
	strcat( buf, "\n\r" );
	free_string( ch->description );
	ch->description = str_dup( buf );
    }

    send_to_char( "Your description is:\n\r", ch );
    send_to_char( ch->description ? ch->description : "(None).\n\r", ch );
    return;
}



void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    sprintf( buf,
	"You say '^BPI have %d/%d hp %d/%d mana %d/%d mv %ld xp.^NW'\n\r",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    send_to_char( buf, ch );

    sprintf( buf, "$n says '^BPI have %d/%d hp %d/%d mana %d/%d mv %ld xp.^NW'",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    act( buf, ch, NULL, NULL, TO_ROOM );

    return;
}



void do_practice( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int sn;

    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	int col;

	col    = 0;
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    if ( ch->level < skill_table[sn].skill_level[ch->class] 
	      || ch->pcdata->learned[sn] < 1 /* skill is not known */)
		continue;

	    sprintf( buf, "%-18s %3d%%  ",
		skill_table[sn].name, ch->pcdata->learned[sn] );
	    send_to_char( buf, ch );
	    if ( ++col % 3 == 0 )
		send_to_char( "\n\r", ch );
	}

	if ( col % 3 != 0 )
	    send_to_char( "\n\r", ch );

	sprintf( buf, "You have %d practice sessions left.\n\r",
	    ch->practice );
	send_to_char( buf, ch );
    }
    else
    {
	CHAR_DATA *mob;
	int adept;

	if ( !IS_AWAKE(ch) )
	{
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    return;
	}

	for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
	{
	    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
		break;
	}

	if ( mob == NULL )
	{
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

	if ( ch->practice <= 0 )
	{
	    send_to_char( "You have no practice sessions left.\n\r", ch );
	    return;
	}

	if ( ( sn = skill_lookup( argument ) ) < 0
	|| ( !IS_NPC(ch)
	&&   (ch->level < skill_table[sn].skill_level[ch->class] 
 	||    ch->pcdata->learned[sn] < 1 /* skill is not known */
	||    skill_table[sn].rating[ch->class] == 0)))
	{
	    send_to_char( "You can't practice that.\n\r", ch );
	    return;
	}

	adept = IS_NPC(ch) ? 100 : class_table[ch->class].skill_adept;

	if ( ch->pcdata->learned[sn] >= adept )
	{
	    sprintf( buf, "You are already learned at %s.\n\r",
		skill_table[sn].name );
	    send_to_char( buf, ch );
	}
	else
	{
	    ch->practice--;
	    ch->pcdata->learned[sn] += 
		int_app[get_curr_stat(ch,STAT_INT)].learn / 
	        skill_table[sn].rating[ch->class];
	    if ( ch->pcdata->learned[sn] < adept )
	    {
		act( "You practice $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR );
		act( "$n practices $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM );
	    }
	    else
	    {
		ch->pcdata->learned[sn] = adept;
		act( "You are now learned at $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR );
		act( "$n is now learned at $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM );
	    }
	}
    }
    return;
}



/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	wimpy = ch->max_hit / 5;
    else
	wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
	send_to_char( "Your courage exceeds your wisdom.\n\r", ch );
	return;
    }

    if ( wimpy > ch->max_hit/2 )
    {
	send_to_char( "Such cowardice ill becomes you.\n\r", ch );
	return;
    }

    ch->wimpy	= wimpy;
    sprintf( buf, "Wimpy set to %d hit points.\n\r", wimpy );
    send_to_char( buf, ch );
    return;
}



void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: password <old> <new>.\n\r", ch );
	return;
    }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(
		"New password not acceptable, try again.\n\r", ch );
	    return;
	}
    }

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    save_char_obj( ch );
    send_to_char( "Ok.\n\r", ch );
    return;
}



/* RT configure command SMASHED */
