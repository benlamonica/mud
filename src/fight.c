
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
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

#define MAX_DAMAGE_MESSAGE 32

/* command procedures needed */
DECLARE_DO_FUN(do_emote		);
DECLARE_DO_FUN(do_berserk	);
DECLARE_DO_FUN(do_bash		);
DECLARE_DO_FUN(do_crush		);
DECLARE_DO_FUN(do_trip		);
DECLARE_DO_FUN(do_dirt		);
DECLARE_DO_FUN(do_flee		);
DECLARE_DO_FUN(do_kick		);
DECLARE_DO_FUN(do_disarm	);
DECLARE_DO_FUN(do_get		);
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_sacrifice	);


/*
 * Local functions.
 */
void	check_assist	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_dodge	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_duck      args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_roll      args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_shield_block	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_blink	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	check_killer	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_parry	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	dam_message	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt, bool immune ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	group_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	xp_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim, 
			    int total_levels ) );
bool	is_safe		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	make_corpse	args( ( CHAR_DATA *ch ) );
void	 one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim,int dt, bool secondary ) ); 
void    mob_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	raw_kill	args( ( CHAR_DATA *victim ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	disarm		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );



/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	ch_next	= ch->next;

	if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
	    continue;

	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
	    multi_hit( ch, victim, TYPE_UNDEFINED );
	else
	    stop_fighting( ch, FALSE );

	if ( ( victim = ch->fighting ) == NULL )
	    continue;

	/*
	 * Fun for the whole family!
	 */
	check_assist(ch,victim);
    }

    return;
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
    CHAR_DATA *rch, *rch_next;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;
	
	if (IS_AWAKE(rch) && rch->fighting == NULL)
	{

	    /* quick check for ASSIST_PLAYER */
	    if (!IS_NPC(ch) && IS_NPC(rch) 
	    && IS_SET(rch->off_flags,ASSIST_PLAYERS)
	    &&  rch->level + 6 > victim->level)
	    {
		do_emote(rch,"screams and attacks!");
		multi_hit(rch,victim,TYPE_UNDEFINED);
		continue;
	    }

	    /* PCs next */
	    if (!IS_NPC(ch) || IS_AFFECTED(ch,AFF_CHARM))
	    {
		if ( ( (!IS_NPC(rch) && IS_SET(rch->act,PLR_AUTOASSIST))
		||     IS_AFFECTED(rch,AFF_CHARM)) 
		&&   is_same_group(ch,rch) )
		    multi_hit (rch,victim,TYPE_UNDEFINED);
		
		continue;
	    }
  	
	    /* now check the NPC cases */
	    
 	    if (IS_NPC(ch) && !IS_AFFECTED(ch,AFF_CHARM))
	
	    {
		if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))

		||   (IS_NPC(rch) && rch->race == ch->race 
		   && IS_SET(rch->off_flags,ASSIST_RACE))

		||   (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALIGN)
		   &&   ((IS_GOOD(rch)    && IS_GOOD(ch))
		     ||  (IS_EVIL(rch)    && IS_EVIL(ch))
		     ||  (IS_NEUTRAL(rch) && IS_NEUTRAL(ch)))) 

		||   (rch->pIndexData == ch->pIndexData 
		   && IS_SET(rch->off_flags,ASSIST_VNUM)))

	   	{
		    CHAR_DATA *vch;
		    CHAR_DATA *target;
		    int number;

		    if (number_bits(1) == 0)
			continue;
		
		    target = NULL;
		    number = 0;
		    for (vch = ch->in_room->people; vch; vch = vch->next)
		    {
			if (can_see(rch,vch)
			&&  is_same_group(vch,victim)
			&&  number_range(0,number) == 0)
			{
			    target = vch;
			    number++;
			}
		    }

		    if (target != NULL)
		    {
			do_emote(rch,"screams and attacks!");
			multi_hit(rch,target,TYPE_UNDEFINED);
		    }
		}	
	    }
	}
    }
}


/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    int     chance;

    /* decrement the wait */
    if (ch->desc == NULL)
	ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);


    /* no attacks for stunnies -- just a check */
    if (ch->position < POS_RESTING)
	return;

    if (IS_NPC(ch))
    {
	mob_hit(ch,victim,dt);
	return;
    }

    one_hit( ch, victim, dt, FALSE );


/* in multi_hit, add this right after the first hit is resolved */
    if (get_eq_char (ch, WEAR_SECONDARY))
    {
    	if(IS_NPC(victim))
 	    chance = get_skill(ch,gsn_dual_wield);
    	else
    	    chance = get_skill(ch,gsn_dual_wield)/2;
    	if ( number_percent( ) < chance )
        {
	    one_hit( ch, victim, dt, TRUE );
	    check_improve(ch,gsn_dual_wield,TRUE,5);
	    if ( ch->fighting != victim )
	    	return;
        }
    }

    if (ch->fighting != victim)
	return;

    if (IS_AFFECTED(ch,AFF_HASTE))
    {
	one_hit(ch,victim,dt,FALSE);
    }

    if ( ch->fighting != victim || dt == gsn_backstab )
	return;

    if(IS_NPC(victim))
 	chance = get_skill(ch,gsn_second_attack);
    else
    	chance = get_skill(ch,gsn_second_attack)/2;
    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt,FALSE );
	check_improve(ch,gsn_second_attack,TRUE,5);
	if ( ch->fighting != victim )
	    return;
    }
    if ( ch->fighting != victim || dt == gsn_circle )
	return;
    
    if(IS_NPC(victim))
	chance = get_skill(ch,gsn_third_attack);
    else
    	chance = get_skill(ch,gsn_third_attack)/3;
    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt,FALSE );
	check_improve(ch,gsn_third_attack,TRUE,6);
	if ( ch->fighting != victim )
	    return;
    }
    if(IS_NPC(victim))
	chance = get_skill(ch,gsn_fourth_attack);
    else
    	chance = get_skill(ch,gsn_fourth_attack)/4;
    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt,FALSE );
	check_improve(ch,gsn_fourth_attack,TRUE,7);
	if ( ch->fighting != victim )
	    return;
    }
    if(IS_NPC(victim))
	chance = get_skill(ch,gsn_fifth_attack);
    else
    	chance = get_skill(ch,gsn_fifth_attack)/4;
    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt,FALSE );
	check_improve(ch,gsn_fifth_attack,TRUE,8);
	if ( ch->fighting != victim )
	    return;
    }


    return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
    int chance,number;
    CHAR_DATA *vch, *vch_next;

    one_hit(ch,victim,dt,FALSE);

    if (ch->fighting != victim)
	return;

    /* Area attack -- BALLS nasty! */
 
    if (IS_SET(ch->off_flags,OFF_AREA_ATTACK))
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	    vch_next = vch->next;
	    if ((vch != ch && vch->fighting == ch))
		one_hit(ch,vch,dt,FALSE);
	}
    }

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_SET(ch->off_flags,OFF_FAST))
	one_hit(ch,victim,dt,FALSE);

    if (ch->fighting != victim || dt == gsn_backstab)
	return;

    chance = get_skill(ch,gsn_second_attack)/2;
    if (number_percent() < chance)
    {
	one_hit(ch,victim,dt,FALSE);
	if (ch->fighting != victim)
	    return;
    }

    chance = get_skill(ch,gsn_third_attack)/4;
    if (number_percent() < chance)
    {
	one_hit(ch,victim,dt,FALSE);
	if (ch->fighting != victim)
	    return;
    } 

    /* oh boy!  Fun stuff! */

    if (ch->wait > 0)
	return;

    number = number_range(0,2);

    if (number == 1 && IS_SET(ch->act,ACT_MAGE))
	/*  { mob_cast_mage(ch,victim); return; } */ ;

    if (number == 2 && IS_SET(ch->act,ACT_CLERIC))
	/* { mob_cast_cleric(ch,victim); return; } */ ;

    /* now for the skills */

    number = number_range(0,7);

    switch(number) 
    {
    case (0) :
	if (IS_SET(ch->off_flags,OFF_BASH))
	    do_bash(ch,"");
	break;

    case (1) :
	if (IS_SET(ch->off_flags,OFF_BERSERK) && !IS_AFFECTED(ch,AFF_BERSERK))
	    do_berserk(ch,"");
	break;


    case (2) :
	if (IS_SET(ch->off_flags,OFF_DISARM) 
	|| (get_weapon_sn(ch) != gsn_hand_to_hand 
	&& (IS_SET(ch->act,ACT_WARRIOR)
   	||  IS_SET(ch->act,ACT_THIEF))))
	    do_disarm(ch,"");
	break;

    case (3) :
	if (IS_SET(ch->off_flags,OFF_KICK))
	    do_kick(ch,"");
	break;

    case (4) :
	if (IS_SET(ch->off_flags,OFF_KICK_DIRT))
	    do_dirt(ch,"");
	break;

    case (5) :
	if (IS_SET(ch->off_flags,OFF_TAIL))
	    /* do_tail(ch,"") */ ;
	break; 

    case (6) :
	if (IS_SET(ch->off_flags,OFF_TRIP))
	    do_trip(ch,"");
	break;

    case (7) :
	if (IS_SET(ch->off_flags,OFF_CRUSH))
	    do_crush(ch,"") ;
	break;
    }
}
	

/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt , bool secondary)
{
    OBJ_DATA *wield;
    int chance;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int diceroll;
    int sn,skill;
    int dam_type;
    AFFECT_DATA af;

    sn = -1;


    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
	return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;

    /*
     * Figure out the type of damage message.
     * if secondary == true, use the second weapon
     */
    if(!secondary)
	 wield = get_eq_char( ch, WEAR_WIELD );
    else
	 wield = get_eq_char(ch, WEAR_SECONDARY);

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
	else 
	    dt += ch->dam_type;
    }

    /*For some reason, this number is occasionally a really big negative
      number.*/
    if (dt < -1)
	dt = TYPE_HIT;

    if (dt < TYPE_HIT)
    	if (wield != NULL)
    	    dam_type = attack_table[wield->value[3]].damage;
    	else
    	    dam_type = attack_table[ch->dam_type].damage;
    else
    	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
	dam_type = DAM_BASH;

    /* get the weapon skill */
    sn = get_weapon_sn(ch);
    skill = 20 + get_weapon_skill(ch,sn);

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC(ch) )
    {
	thac0_00 = 20;
	thac0_32 = -4;   /* as good as a thief */ 
	if (IS_SET(ch->act,ACT_WARRIOR))
	    thac0_32 = -10;
	else if (IS_SET(ch->act,ACT_THIEF))
	    thac0_32 = -4;
	else if (IS_SET(ch->act,ACT_CLERIC))
	    thac0_32 = 2;
	else if (IS_SET(ch->act,ACT_MAGE))
	    thac0_32 = 6;
    }
    else
    {
	thac0_00 = class_table[ch->class].thac0_00;
	thac0_32 = class_table[ch->class].thac0_32;
    }

    thac0  = interpolate( ch->level, thac0_00, thac0_32 );

    thac0 -= GET_HITROLL(ch) * skill/100;
    thac0 += 5 * (100 - skill) / 100;

    if (dt == gsn_backstab)
	thac0 -= 10 * (100 - get_skill(ch,gsn_backstab));

    switch(dam_type)
    {
	case(DAM_PIERCE):victim_ac = GET_AC(victim,AC_PIERCE)/10;	break;
	case(DAM_BASH):	 victim_ac = GET_AC(victim,AC_BASH)/10;		break;
	case(DAM_SLASH): victim_ac = GET_AC(victim,AC_SLASH)/10;	break;
	default:	 victim_ac = GET_AC(victim,AC_EXOTIC)/10;	break;
    }; 
	
    if (victim_ac < -15)
	victim_ac = (victim_ac + 15) / 5 - 15;
     
    if ( !can_see( ch, victim ) )
	victim_ac -= 4;

    if ( victim->position < POS_FIGHTING)
	victim_ac += 4;
 
    if (victim->position < POS_RESTING)
	victim_ac += 6;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
	;

    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
	/* Miss. */
	damage( ch, victim, 0, dt, dam_type );
	tail_chain( );
	return;
    }

    /*
     * Hit.
     * Calc damage.
     */
    if ( IS_NPC(ch) && (!ch->pIndexData->new_format || wield == NULL))
	if (!ch->pIndexData->new_format)
	{
	    dam = number_range( ch->level / 2, ch->level * 3 / 2 );
	    if ( wield != NULL )
	    	dam += dam / 2;
	}
	else
	    dam = dice(ch->damage[DICE_NUMBER],ch->damage[DICE_TYPE]);
	
    else
    {
	if (sn != -1)
	    check_improve(ch,sn,TRUE,5);

	if (sn == gsn_hand_to_hand)
	    dam = number_range(1 + 6 * skill/100,4 * ch->level/3 * skill/100);
	else	 
	{
	if ( wield != NULL )
	{
	    if (wield->pIndexData->new_format)
		dam = dice(wield->value[1],wield->value[2]) * skill/100;
	    else
	    	dam = number_range( wield->value[1] * skill/100, 
				wield->value[2] * skill/100);

	    if (get_eq_char(ch,WEAR_SHIELD) == NULL)  /* no shield = more */
		dam = dam * 21/20;
	}
	else
	    dam = number_range( 1 + 4 * skill/100, 2 * ch->level/3 * skill/100);
	}
    }

    /*
     * Bonuses.
     */
    if ( get_skill(ch,gsn_enhanced_damage) > 0 )
    {
        diceroll = number_percent();
        if (diceroll <= get_skill(ch,gsn_enhanced_damage))
        {
            check_improve(ch,gsn_enhanced_damage,TRUE,6);
            dam += dam * diceroll/100;
        }
    }

    if ( !IS_AWAKE(victim) )
	dam *= 2;
     else if (victim->position < POS_FIGHTING)
	dam = dam * 3 / 2;

    if ( dt == gsn_backstab && wield != NULL) 
    	if ( wield->value[0] != 2 )
	    dam *= 3 + ch->level / 10; 
	else 
	    dam *= 3 + ch->level / 8;

    dam += GET_DAMROLL(ch) * UMIN(100,skill) /100;

    if(wield!=NULL && wield->value[4]!=0)
    {
	if(IS_SET(wield->value[4],WEAPON_SHARP))
	    dam+=(int) dam*.15;
        if(IS_SET(wield->value[4],WEAPON_VORPAL))
            dam+=(int) dam*.25;
    }

    if ( dam <= 0 )
	dam = 1;

    if ( dam > 800 )
	dam = 800;

    if(damage( ch, victim, dam, dt, dam_type )!=FALSE)
    {
    	/*Weapon Flags - flaming frost vampiric vorpal sharp*/
    	if(wield!=NULL && wield->value[4]!=0)
    	{
            if(IS_SET(wield->value[4],WEAPON_FLAMING))
            {
	    	chance = (victim->level <= wield->level) ? (wield->level - victim->level) : 5;
		chance = URANGE(5,chance,95);
		if(number_percent() < chance)
                {
		    act("Smoke fills your eyes, blinding you.",ch,NULL,victim,TO_VICT);
		    act("Smoke fills $N's eyes, blinding $m.",ch,NULL,victim,TO_CHAR);
		    act("Smoke fills $N's eyes, blinding $m.",ch,NULL,victim,TO_NOTVICT);
		    af.type      = skill_lookup("blind");
    		    af.level     = wield->level;
    		    af.location  = APPLY_HITROLL;
    		    af.modifier  = -1;
    		    af.duration  = 2;
    		    af.bitvector = AFF_BLIND;
    		    affect_to_char( victim, &af );
		}
		sn = skill_lookup("fire breath");
	    	(*skill_table[sn].spell_fun) ( sn, 0, ch, victim );
            }
            if(IS_SET(wield->value[4],WEAPON_FROST))
            {
	    	chance = (victim->level <= wield->level) ? (wield->level - victim->level) : 5;
		chance = URANGE(5,chance,95);
		if(number_percent() < chance)
                {
		    act("You are surrounded by a block of ice!",ch,NULL,victim,TO_VICT);
		    act("$N is surrounded by a block of ice!",ch,NULL,victim,TO_CHAR);
		    act("$N is surrounded by a block of ice!",ch,NULL,victim,TO_NOTVICT);
		    af.type      = skill_lookup("chill touch");
    		    af.level     = wield->level;
    		    af.location  = APPLY_STR;
    		    af.modifier  = -1;
    		    af.duration  = 6;
    		    af.bitvector = 0;
    		    affect_to_char( victim, &af );
		}
		sn = skill_lookup("frost breath");
	    	(*skill_table[sn].spell_fun) ( sn, 0, ch, victim );
            }
            if(IS_SET(wield->value[4],WEAPON_VAMPIRIC))
            {
	    	sn = skill_lookup("energy drain");
	    	(*skill_table[sn].spell_fun) ( sn, 0, ch, victim );
            }
        }
    
        affect_get(victim, skill_lookup("flame shield"), &af);
        if(af.type!=-1)
    	{
            damage( victim, ch, dam/2, af.type, DAM_FIRE );
	    if (af.duration-1 <= 0)
            {
	    	send_to_char("Your magic fails and your flame shield vanishes.\n\r",victim);
	    	affect_strip(victim,af.type);
            }
	    else
            {
	   	af.duration=-1;
	    	affect_join(victim,&af);
	    }
        }
    }	
    tail_chain( );
    return;
}



/*
 * Inflict damage from a hit.
 */
bool damage2( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type )
{
    bool immune;

    if ( victim->position == POS_DEAD )
	return FALSE;

    /*
     * Stop up any residual loopholes.
     */
    if ( dam > 1000 && !IS_IMMORTAL(ch))
    {
	bug( "Damage: %d: more than 1000 points!", dam );
	dam = 1000;
	if (!IS_IMMORTAL(ch))
	{
	    send_to_char("You really shouldn't cheat.\n\r",ch);
	}
    }

    ch->pk_timer = time(NULL);
    victim->pk_timer = time(NULL);    

	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	check_killer( ch, victim );

    /*
     * Damage modifiers.
     */
    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
	dam /= 2;

    if ( IS_AFFECTED(victim, AFF_PROTECT) && IS_EVIL(ch) )
	dam -= dam / 4;

    if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD) && !IS_EVIL(ch) )
	dam -= dam / 4;

    immune = FALSE;

    switch(check_immune(victim,dam_type))
    {
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam = 0;
	    break;
	case(IS_RESISTANT):	
	    dam -= dam/3;
	    break;
	case(IS_VULNERABLE):
	    dam += dam/2;
	    break;
    }

    dam_message( ch, victim, dam, dt, immune );

    if (dam <= 0)
	return FALSE;

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	act( "$n is mortally wounded, and will die soon, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char( 
	    "You are mortally wounded, and will die soon, if not aided.\n\r",
	    victim );
	break;

    case POS_INCAP:
	act( "$n is incapacitated and will slowly die, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char(
	    "You are incapacitated and will slowly die, if not aided.\n\r",
	    victim );
	break;

    case POS_STUNNED:
	act( "$n is stunned, but will probably recover.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char("You are stunned, but will probably recover.\n\r",
	    victim );
	break;

    case POS_DEAD:
	act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
	send_to_char( "You have been KILLED!!\n\r\n\r", victim );
        if(!IS_NPC(ch) && !IS_NPC(victim))
 	{
	    ch->pkstat[1]++;
	    ch->pkstat[2]+=(victim->level-ch->level)+1;
	    victim->pkstat[0]++;

            
	    if((ch->level-7 > victim->level && ch->level > victim->pcdata->virtual_level[0]+7 ) 
              && !(IS_SET(ch->act,PLR_IMMQUEST) && IS_SET(victim->act,PLR_IMMQUEST))) 
            {
		send_to_char("You are now a ^NRKILLER^NW. May the Gods have mercy on your soul.\n\r",ch);
		SET_BIT(ch->act,PLR_KILLER);
	    }

	}
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char( "That really did HURT!\n\r", victim );
	if ( victim->hit < victim->max_hit / 4 )
	    send_to_char( "You sure are BLEEDING!\n\r", victim );
	break;
    }

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
	group_gain( ch, victim );

	sprintf( log_buf, "%s%s(%d) killed by %s%s(%d) at [%d].",(IS_NPC(victim))?"(MOB) ":"",
	victim->name, victim->level, (IS_NPC(ch)?"(MOB)":""),
	(IS_NPC(ch) ? ch->short_descr : ch->name), ch->level,
	victim->in_room->vnum );

	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "%s(%d) killed by %s%s(%d) at [%d].",
		victim->name, victim->level, (IS_NPC(ch)?"(MOB)":""),
		(IS_NPC(ch) ? ch->short_descr : ch->name), ch->level,
		victim->in_room->vnum );
	    log_string( log_buf );

	    /*
	     * Dying penalty:
	     * 1/2 way back to previous level.
	     */
	    if ( victim->exp > exp_per_level(victim,victim->points) 
			       * victim->level )
		gain_exp( victim, (exp_per_level(victim,victim->points)
			         * victim->level - victim->exp)/2 );
	}

        if (IS_NPC(victim))
            wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
        else
            wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

	raw_kill( victim );

	return TRUE;
    }

    if ( victim == ch )
	return TRUE;

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_recall( victim, "" );
	    return TRUE;
	}
    }

    tail_chain( );
    return TRUE;
}

bool damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type )
{

    OBJ_DATA *corpse;
    bool immune;

    if ( victim->position == POS_DEAD )
	return FALSE;

    /*
     * Stop up any residual loopholes.
     */
    if ( dam > 1000 && !IS_IMMORTAL(ch))
    {
	bug( "Damage: %d: more than 1000 points!", dam );
	dam = 1000;
	if (!IS_IMMORTAL(ch))
	{
	    send_to_char("You really shouldn't cheat.\n\r",ch);
	}

    }

    
    /* damage reduction */
    if ( dam > 30)
	dam = (dam - 30)/2 + 30;
    if ( dam > 75)
	dam = (dam - 75)/2 + 75;



   
    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( is_safe( ch, victim ) )
	    return FALSE;
	check_killer( ch, victim );

	ch->pk_timer=time(NULL);
	victim->pk_timer=time(NULL);

	if ( victim->position > POS_STUNNED )
	{
	    if ( victim->fighting == NULL )
		set_fighting( victim, ch );
	    if (victim->timer <= 4)
	    	victim->position = POS_FIGHTING;
	}

	if ( victim->position > POS_STUNNED )
	{
	    if ( ch->fighting == NULL )
		set_fighting( ch, victim );

	    /*
	     * If victim is charmed, ch might attack victim's master.
	     */
	    if ( IS_NPC(ch)
	    &&   IS_NPC(victim)
	    &&   IS_AFFECTED(victim, AFF_CHARM)
	    &&   victim->master != NULL
	    &&   victim->master->in_room == ch->in_room
	    &&   number_bits( 3 ) == 0 )
	    {
		stop_fighting( ch, FALSE );
		multi_hit( ch, victim->master, TYPE_UNDEFINED );
		return FALSE;
	    }
	}

	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );
    }

    /*
     * Inviso attacks ... not.
     */
    if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
	affect_strip( ch, gsn_invis );
	affect_strip( ch, gsn_mass_invis );
	REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
    }

    /*
     * Damage modifiers.
     */
    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
	dam /= 2;

    if ( IS_AFFECTED(victim, AFF_PROTECT) && IS_EVIL(ch) )
	dam -= dam / 4;

    if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD) && !IS_EVIL(ch) )
	dam -= dam / 4;

    immune = FALSE;


    /*
     * Check for parry, and dodge, duck and roll, and blink
     */
    if ( dt >= TYPE_HIT && ch != victim)
    {
        if ( check_parry( ch, victim ) )
	    return FALSE;
	if ( check_dodge( ch, victim ) )
	    return FALSE;
        if ( check_duck ( ch, victim ) )
            return FALSE;
        if ( check_roll ( ch, victim ) )
            return FALSE;
        if ( check_shield_block( ch, victim ) )
	    return FALSE;
 	if ( check_blink( ch, victim ) )
	    return FALSE;
    }

    switch(check_immune(victim,dam_type))
    {
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam = 0;
	    break;
	case(IS_RESISTANT):	
	    dam -= dam/3;
	    break;
	case(IS_VULNERABLE):
	    dam += dam/2;
	    break;
    }

    dam_message( ch, victim, dam, dt, immune );

    if (dam == 0)
	return FALSE;

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	act( "$n is mortally wounded, and will die soon, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char( 
	    "You are mortally wounded, and will die soon, if not aided.\n\r",
	    victim );
	break;

    case POS_INCAP:
	act( "$n is incapacitated and will slowly die, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char(
	    "You are incapacitated and will slowly die, if not aided.\n\r",
	    victim );
	break;

    case POS_STUNNED:
	act( "$n is stunned, but will probably recover.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char("You are stunned, but will probably recover.\n\r",
	    victim );
	break;

    case POS_DEAD:
	act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
	send_to_char( "You have been KILLED!!\n\r\n\r", victim );
	victim->pk_timer = time(NULL)-90;
        if(!IS_NPC(ch) && !IS_NPC(victim))
 	{
	    ch->pkstat[1]++;
	    ch->pkstat[2]+=(victim->level-ch->level)+1;
	    victim->pkstat[0]++;

	    if((ch->level-7 > victim->level && ch->level > victim->pcdata->virtual_level[0]+7 ) 
              && !(IS_SET(ch->act,PLR_IMMQUEST) && IS_SET(victim->act,PLR_IMMQUEST))) 
            {
		send_to_char("You are now a ^NRKILLER^NW. May the Gods have mercy on your soul.\n\r",ch);
		SET_BIT(ch->act,PLR_KILLER);
	    }

	}
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char( "That really did HURT!\n\r", victim );
	if ( victim->hit < victim->max_hit / 4 )
	    send_to_char( "You sure are BLEEDING!\n\r", victim );
	break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE(victim) )
	stop_fighting( victim, FALSE );

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
	group_gain( ch, victim );

	sprintf( log_buf, "%s%s(%d) killed by %s%s(%d) at [%d].",(IS_NPC(victim))?"(MOB) ":"",
	victim->name, victim->level, (IS_NPC(ch)?"(MOB)":""),
	(IS_NPC(ch) ? ch->short_descr : ch->name), ch->level,
	victim->in_room->vnum );

	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "%s(%d) killed by %s%s(%d) at [%d].",
		victim->name, victim->level, (IS_NPC(ch)?"(MOB)":""),
		(IS_NPC(ch) ? ch->short_descr : ch->name), ch->level,
		victim->in_room->vnum );
	    log_string( log_buf );

	    /*
	     * Dying penalty:
	     * 1/2 way back to previous level.
	     */
	    if ( victim->exp > exp_per_level(victim,victim->points) 
			       * victim->level )
		gain_exp( victim, (exp_per_level(victim,victim->points)
			         * victim->level - victim->exp)/2 );
	}

        if (IS_NPC(victim))
            wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
        else
            wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

	raw_kill( victim );

        /* RT new auto commands */

	if ( !IS_NPC(ch) && IS_NPC(victim) )
	{
	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents ); 

	    if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
		 corpse && corpse->contains) /* exists and not empty */
		do_get( ch, "all corpse" );

 	    if (IS_SET(ch->act,PLR_AUTOGOLD) &&
	        corpse && corpse->contains  && /* exists and not empty */
		!IS_SET(ch->act,PLR_AUTOLOOT))
	      do_get(ch, "gold corpse");
            
	    if ( IS_SET(ch->act, PLR_AUTOSAC) )
       	      if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
		return TRUE;  /* leave if corpse has treasure */
	      else
		do_sacrifice( ch, "corpse" );
	}

	return FALSE;
    }

    if ( victim == ch )
	return TRUE;

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_recall( victim, "" );
	    return TRUE;
	}
    }

    /*
     * Wimp out?
     */
    if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
	&&   victim->hit < victim->max_hit / 5) 
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	    do_flee( victim, "" );
    }

    if ( !IS_NPC(victim)
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait < PULSE_VIOLENCE / 2 )
	do_flee( victim, "" );

    tail_chain( );
    return TRUE;
}



bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim )
{

    /* no killing in shops hack */
    if (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    {
	send_to_char("The shopkeeper wouldn't like that.\n\r",ch); 
        return TRUE;
    }
    /* no killing healers, adepts, etc */
    if (IS_NPC(victim) 
    && (IS_SET(victim->act,ACT_TRAIN)
    ||  IS_SET(victim->act,ACT_PRACTICE)
    ||  IS_SET(victim->act,ACT_IS_HEALER)
    ||  IS_SET(victim->act,ACT_BANKER)))
    {
	send_to_char("I don't think the gods would approve.\n\r",ch);
	return TRUE;
    }

    if (IS_SET(ch->act,PLR_KILLER))
    {
	send_to_char("You are not allowed to commit more crimes.\n\r",ch);
	return TRUE;
    }

    /* no fighting in safe rooms */
    if (IS_SET(ch->in_room->room_flags,ROOM_SAFE))
    {
	send_to_char("Not in this room.\n\r",ch);
	return TRUE;
    }

    if (victim->fighting == ch)
	return FALSE;

    if (IS_NPC(ch))
    {
 	/* charmed mobs and pets cannot attack players */
	if (!IS_NPC(victim) && (IS_AFFECTED(ch,AFF_CHARM)
			    ||  IS_SET(ch->act,ACT_PET)))
	    return TRUE;

      	return FALSE;
     }

     else /* Not NPC */
     {	
	if (IS_IMMORTAL(ch))
	    return FALSE;

	/* no pets */
	if (IS_NPC(victim) && IS_SET(victim->act,ACT_PET))
	{
            act("But $N looks so cute and cuddly...",ch,NULL,victim,TO_CHAR);
            return TRUE;
	}

	/* no charmed mobs unless char is the the owner */
	if (IS_AFFECTED(victim,AFF_CHARM) && ch != victim->master)
	{
            send_to_char("You don't own that monster.\n\r",ch);
	    return TRUE;
	}

	if (!IS_NPC(victim) && victim->level < 16)
        {
	    send_to_char("Wait until they are level 15 before you try to kill them\n\r",ch);
	    return TRUE;
        }

        if (!IS_NPC(victim) && ch->level < 16)
   	{
	    send_to_char("Doing this would be an agressive action. Please wait until you are level 16 to do this.\n\r",ch);
	    return TRUE;
        }

        /* we want player killing */
/*	if (!IS_NPC(victim))
	{
	    send_to_char("Sorry, player killing is not permitted.\n\r",ch);
	    return TRUE;
	}*/

	return FALSE;
    }
}

bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area )
{
    /* can't zap self (crash bug) */
    if (ch == victim)
	return TRUE;
    /* immortals not hurt in area attacks */
    if (IS_IMMORTAL(victim) &&  area)
	return TRUE;

    /* no killing in shops hack */
    if (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
        return TRUE;

    /* no killing healers, adepts, etc */
    if (IS_NPC(victim)
    && (IS_SET(victim->act,ACT_TRAIN)
    ||  IS_SET(victim->act,ACT_PRACTICE)
    ||  IS_SET(victim->act,ACT_IS_HEALER)))
	return TRUE;

    /* no fighting in safe rooms */
    if (IS_SET(ch->in_room->room_flags,ROOM_SAFE))
        return TRUE;

    if (victim->fighting == ch)
	return FALSE;
 
    if (IS_NPC(ch))
    {
        /* charmed mobs and pets cannot attack players */
        if (!IS_NPC(victim) && (IS_AFFECTED(ch,AFF_CHARM)
                            ||  IS_SET(ch->act,ACT_PET)))
            return TRUE;
	
	/* area affects don't hit other mobiles */
        if (IS_NPC(victim) && area)
            return TRUE;
 
        return FALSE;
    }
 
    else /* Not NPC */
    {
        if (IS_IMMORTAL(ch) && !area)
            return FALSE;
 
        /* no pets */
        if (IS_NPC(victim) && IS_SET(victim->act,ACT_PET))
            return TRUE;
 
        /* no charmed mobs unless char is the the owner */
/*        if (IS_AFFECTED(victim,AFF_CHARM) && ch != victim->master)
            return TRUE;
*/
 
	/* cannot use spells if not in same group */
/*	if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
	    return TRUE;
*/
        return FALSE;
    }
}

/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
    /*
     * Follow charm thread to responsible character.
     * Attacking someone's charmed char is hostile!
     */
    while ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL )
	victim = victim->master;

    /*
     * NPC's are fair game.
     * So are killers and thieves.
     */
    if ( IS_NPC(victim)
    ||   IS_SET(victim->act, PLR_KILLER)
    ||   IS_SET(victim->act, PLR_THIEF) )
	return;

    /*
     * Charm-o-rama.
     */
    if ( IS_SET(ch->affected_by, AFF_CHARM) )
    {
	if ( ch->master == NULL )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Check_killer: %s bad AFF_CHARM",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
	    affect_strip( ch, gsn_charm_person );
	    REMOVE_BIT( ch->affected_by, AFF_CHARM );
	    return;
	}
/*
	send_to_char( "*** You are now a KILLER!! ***\n\r", ch->master );
  	SET_BIT(ch->master->act, PLR_KILLER);
*/
	stop_follower( ch );
	return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch)
    ||   ch == victim
    ||   ch->level >= LEVEL_IMMORTAL
    ||   IS_SET(ch->act, PLR_KILLER) )
	return;

    if( victim->level+7 >= ch->level ) return;

    save_char_obj( ch );
    return;
}



/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( IS_NPC(victim) )
    {
	chance	= UMIN( 30, victim->level );
    }
    else
    {
	if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
	    return FALSE;
	chance	= victim->pcdata->learned[gsn_parry] / 2;
    }

    if ( number_percent( ) >= chance + victim->level - ch->level )
	return FALSE;

    act( "You parry $n's attack.",  ch, NULL, victim, TO_VICT    );
    act( "$N parries your attack.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_parry,TRUE,6);
    return TRUE;
}



/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( IS_NPC(victim) )
        chance  = UMIN( 30, victim->level );
    else
        chance  = victim->pcdata->learned[gsn_dodge] / 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "You dodge $n's attack.", ch, NULL, victim, TO_VICT    );
    act( "$N dodges your attack.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_dodge,TRUE,6);
    return TRUE;
}
bool check_roll( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( IS_NPC(victim) )
        return FALSE;

    chance  = victim->pcdata->learned[gsn_roll] / 3;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "You roll away from $n's attack.", ch, NULL, victim, TO_VICT );
    act( "$N rolls away from your attack.", ch, NULL, victim, TO_CHAR );
    check_improve(victim,gsn_roll,TRUE,8);
    return TRUE;
}
bool check_duck( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( IS_NPC(victim) )
        return FALSE;
        chance  = victim->pcdata->learned[gsn_duck] / 3;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "You duck, causing $n to miss you.", ch, NULL, victim, TO_VICT );
    act( "$N ducks, causing you to miss $M.", ch, NULL, victim, TO_CHAR );
    check_improve(victim,gsn_duck,TRUE,8);
    return TRUE;
}

/*
 * Check for shield block.
 */
bool check_shield_block( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( get_eq_char(victim,WEAR_SHIELD) == NULL)
	return FALSE;

    if ( IS_NPC(victim) )
        chance  = UMIN( 30, victim->level );
    else
        chance  = victim->pcdata->learned[gsn_shield_block] / 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "You block $n's attack with your shield.", ch, NULL, victim, TO_VICT );
    act( "$N blocks your attack with $S shield.", ch, NULL, victim, TO_CHAR);
    check_improve(victim,gsn_shield_block,TRUE,6);
    return TRUE;
}

/*
 * Check for blink.
 */
bool check_blink( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if (!IS_SET(victim->affected_by,AFF_BLINK))
	return FALSE;

    if ( IS_NPC(victim) )
        chance  = 0;
    else
        chance  = victim->pcdata->learned[skill_lookup("blink")] / 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "You blink out of reality, avoiding  $n's attack.", ch, NULL, victim, TO_VICT    );
    act( "$N blinks out of reality, avoiding your attack.", ch, NULL, victim, TO_CHAR    );
    return TRUE;
}



/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( victim->hit > 0 )
    {
    	if ( victim->position <= POS_STUNNED )
	    victim->position = POS_STANDING;
	return;
    }

    if ( IS_NPC(victim) && victim->hit < 1 )
    {
	victim->position = POS_DEAD;
	return;
    }

    if ( victim->hit <= -11 )
    {
	victim->position = POS_DEAD;
	return;
    }

         if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    return;
}



/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fighting != NULL )
    {
	bug( "Set_fighting: already fighting", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
	affect_strip( ch, gsn_sleep );

    ch->fighting = victim;
    ch->position = POS_FIGHTING;

    return;
}



/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch == ch || ( fBoth && fch->fighting == ch ) )
	{
	    if(!IS_NPC(fch))
 		fch->pk_timer = time(NULL);
	    fch->fighting	= NULL;
	    fch->position	= IS_NPC(fch) ? ch->default_pos : POS_STANDING;
	    update_pos( fch );
	}
    }

    return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;

    if ( IS_NPC(ch) )
    {
	name		= ch->short_descr;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer	= number_range( 3, 6 );
	if ( ch->gold > 0 )
	{
	    obj_to_obj( create_money( ch->gold ), corpse );
	    ch->gold = 0;
	}
	corpse->cost = 0;
    }
    else
    {
	name		= ch->name;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer	= number_range( 25, 40 );
	REMOVE_BIT(ch->act,PLR_CANLOOT);
	if (!IS_SET(ch->act,PLR_KILLER) && !IS_SET(ch->act,PLR_THIEF))
	    corpse->owner = str_dup(ch->name);
	else
	    corpse->owner = NULL;
	corpse->cost = ch->level;
    }

    corpse->level = ch->level;

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

    if(!IS_SET(ch->act,PLR_KILLER))
    {
    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
        obj_from_char( obj );
	if (obj->item_type == ITEM_POTION)
	    obj->timer = number_range(500,1000);
	if (obj->item_type == ITEM_SCROLL)
	    obj->timer = number_range(1000,2500);
	if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH))
	    obj->timer = number_range(5,10);
	REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);
	REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);

	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) || IS_SET( obj->wear_flags, ITEM_WEAR_TATTOO))
	    extract_obj( obj );
	else
	    obj_to_obj( obj, corpse );
    }
    }

    obj_to_room( corpse, ch->in_room );
    return;
}



/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "You hear $n's death cry.";

    switch ( number_bits(4))
    {
    case  0: msg  = "$n hits the ground ... DEAD.";			break;
    case  1: 
	if (ch->material == 0)
	{
	    msg  = "$n splatters blood on your armor.";		
	    break;
	}
    case  2: 							
	if (IS_SET(ch->parts,PART_GUTS))
	{
	    msg = "$n spills $s guts all over the floor.";
	    vnum = OBJ_VNUM_GUTS;
	}
	break;
    case  3: 
	if (IS_SET(ch->parts,PART_HEAD))
	{
	    msg  = "$n's severed head plops on the ground.";
	    vnum = OBJ_VNUM_SEVERED_HEAD;				
	}
	break;
    case  4: 
	if (IS_SET(ch->parts,PART_HEART))
	{
	    msg  = "$n's heart is torn from $s chest.";
	    vnum = OBJ_VNUM_TORN_HEART;				
	}
	break;
    case  5: 
	if (IS_SET(ch->parts,PART_ARMS))
	{
	    msg  = "$n's arm is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_ARM;				
	}
	break;
    case  6: 
	if (IS_SET(ch->parts,PART_LEGS))
	{
	    msg  = "$n's leg is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_LEG;				
	}
	break;
    case 7:
	if (IS_SET(ch->parts,PART_BRAINS))
	{
	    msg = "$n's head is shattered, and $s brains splash all over you.";
	    vnum = OBJ_VNUM_BRAINS;
	}
    }

    act( msg, ch, NULL, NULL, TO_ROOM );

    if ( vnum != 0 )
    {
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;

	name		= IS_NPC(ch) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ), 0 );
	obj->timer	= number_range( 4, 7 );

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

	if (obj->item_type == ITEM_FOOD)
	{
	    if (IS_SET(ch->form,FORM_POISON))
		obj->value[3] = 1;
	    else if (!IS_SET(ch->form,FORM_EDIBLE))
		obj->item_type = ITEM_TRASH;
	}

	obj_to_room( obj, ch->in_room );
    }

    if ( IS_NPC(ch) )
	msg = "You hear something's death cry.";
    else
	msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   pexit->u1.to_room != was_in_room )
	{
	    ch->in_room = pexit->u1.to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;

    return;
}



void raw_kill( CHAR_DATA *victim )
{
    OBJ_DATA *obj;
    int i;

    obj = NULL;
    stop_fighting( victim, TRUE );
    death_cry( victim );
    if(!(!IS_NPC(victim) && IS_SET(victim->act,PLR_IMMQUEST)))
	make_corpse( victim );


    if ( IS_NPC(victim) )
    {
	victim->pIndexData->killed++;
	kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
	extract_char( victim, TRUE );
	return;
    }

    if(!(!IS_NPC(victim) && IS_SET(victim->act,PLR_IMMQUEST)))
    {
    	extract_char( victim, FALSE );
	if(guild_table[victim->pcdata->guild].guild_vnum!=0)
        {
	    obj = create_object(get_obj_index(guild_table[victim->pcdata->guild].guild_vnum),96);
	    obj_to_char(obj,victim);
       	    equip_char(victim,obj,WEAR_TATTOO);
   	}
    	while ( victim->affected )
	affect_remove( victim, victim->affected );
    	victim->affected_by	= 0;
    	for (i = 0; i < 4; i++)
    	    victim->armor[i]= 100;
    	victim->position	= POS_RESTING;
    	victim->hit		= UMAX( 1, victim->hit  );
    	victim->mana	= UMAX( 1, victim->mana );
    	victim->move	= UMAX( 1, victim->move );
    /* RT added to prevent infinite deaths */
    	REMOVE_BIT(victim->act, PLR_KILLER);
	REMOVE_BIT(victim->act, PLR_THIEF);
	REMOVE_BIT(victim->act, PLR_BOUGHT_PET);
	save_char_obj( victim ); 
	return;
    }
    else
    {
	char_from_room(victim);
	char_to_room( victim, get_room_index(ROOM_VNUM_ALTAR));
    	while ( victim->affected )
	affect_remove( victim, victim->affected );
    	victim->affected_by	= 0;

	victim->position	= POS_RESTING;
	victim->hit		= UMAX( 1, victim->hit );
	victim->mana 		= UMAX( 1, victim->mana );
   	victim->move		= UMAX( 1, victim->move );
	return;
    }
}



void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp;
    int members;
    int group_levels;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( victim == ch )
	return;
    
    members = 0;
    group_levels = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
        {
	    members++;
	    group_levels += gch->level;
	}
    }
    if (IS_SET(ch->act, PLR_QUESTOR)&&IS_NPC(victim))
    {
	if(ch->questmob == victim->pIndexData->vnum)
	{
		send_to_char("You have almost completed your QUEST!!\n\r",ch);
		send_to_char("Return to the quest master before the time runs out!\n\r",ch);
		ch->questmob = -1;
	}
    }

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
	group_levels = ch->level ;
    }

    lch = (ch->leader != NULL) ? ch->leader : ch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if ( !is_same_group( gch, ch ) || IS_NPC(gch))
	    continue;

/*
	if ( gch->level - lch->level >= 9 )
	{
	    send_to_char( "You are too high for this group.\n\r", gch );
	    continue;
	}

	if ( gch->level - lch->level <= -9 )
	{
	    send_to_char( "You are too low for this group.\n\r", gch );
	    continue;
	}

*/
	xp = xp_compute( gch, victim, group_levels );  
	sprintf( buf, "You receive %d experience points.\n\r", xp );
	send_to_char( buf, gch );
	gain_exp( gch, xp );

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE )
		continue;

	    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
	    {
		act( "You are zapped by $p.", ch, obj, NULL, TO_CHAR );
		act( "$n is zapped by $p.",   ch, obj, NULL, TO_ROOM );
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
	    }
	}
    }

    return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels )
{
    int xp,base_exp;
    int align,level_range;
    int change;
//    int time_per_level;

    level_range = victim->level - gch->level;
 
    /* compute the base exp */
    switch (level_range)
    {
 	default : 	base_exp =   0;		break;
	case -9 :	base_exp =   1;		break;
	case -8 :	base_exp =   2;		break;
	case -7 :	base_exp =   5;		break;
	case -6 : 	base_exp =   9;		break;
	case -5 :	base_exp =  21;		break;
	case -4 :	base_exp =  33;		break;
	case -3 :	base_exp =  50;		break;
	case -2 :	base_exp =  65;		break;
	case -1 :	base_exp =  80;		break;
	case  0 :	base_exp =  95;		break;
	case  1 :	base_exp = 110;		break;
	case  2 :	base_exp = 140;		break;
	case  3 :	base_exp = 160;		break;
	case  4 :	base_exp = 180;		break;
    } 
    
    if (level_range > 4)
	base_exp = 300 + 20 * (level_range - 4);

    /* do alignment computations */
   
    align = victim->alignment - gch->alignment;

    if (IS_SET(victim->act,ACT_NOALIGN))
    {
	/* no change */
    }

    else if (align > 500) /* monster is more good than slayer */
    {
	change = (align - 500) * base_exp / 500 * gch->level/total_levels; 
	change = UMAX(1,change);
        gch->alignment = UMAX(-1000,gch->alignment - change);
    }

    else if (align < -500) /* monster is more evil than slayer */
    {
	change =  ( -1 * align - 500) * base_exp/500 * gch->level/total_levels;
	change = UMAX(1,change);
	gch->alignment = UMIN(1000,gch->alignment + change);
    }

    else /* improve this someday */
    {
	change =  gch->alignment * base_exp/500 * gch->level/total_levels;  
	gch->alignment -= change;
    }
    
    /* calculate exp multiplier */
    if (IS_SET(victim->act,ACT_NOALIGN))
	xp = base_exp;

    else if (gch->alignment > 500)  /* for goodie two shoes */
    {
	if (victim->alignment < -750)
	    xp = base_exp * 4/3;
   
 	else if (victim->alignment < -500)
	    xp = base_exp * 5/4;

        else if (victim->alignment > 250)
	    xp = base_exp * 3/4; 

        else if (victim->alignment > 750)
	    xp = base_exp / 4;

   	else if (victim->alignment > 500)
	    xp = base_exp / 2;

	else
	    xp = base_exp;
    }

    else if (gch->alignment < -500) /* for baddies */
    {
	if (victim->alignment > 750)
	    xp = base_exp * 5/4;
	
  	else if (victim->alignment > 500)
	    xp = base_exp * 11/10; 

   	else if (victim->alignment < -750)
	    xp = base_exp * 1/2;

	else if (victim->alignment < -500)
	    xp = base_exp * 3/4;

	else if (victim->alignment < -250)
	    xp = base_exp * 9/10;

	else
	    xp = base_exp;
    }

    else if (gch->alignment > 200)  /* a little good */
    {

	if (victim->alignment < -500)
	    xp = base_exp * 6/5;

 	else if (victim->alignment > 750)
	    xp = base_exp * 1/2;

	else if (victim->alignment > 0)
	    xp = base_exp * 3/4; 
	
	else
	    xp = base_exp;
    }

    else if (gch->alignment < -200) /* a little bad */
    {
	if (victim->alignment > 500)
	    xp = base_exp * 6/5;
 
	else if (victim->alignment < -750)
	    xp = base_exp * 1/2;

	else if (victim->alignment < 0)
	    xp = base_exp * 3/4;

	else
	    xp = base_exp;
    }

    else /* neutral */
    {

	if (victim->alignment > 500 || victim->alignment < -500)
	    xp = base_exp * 4/3;

	else if (victim->alignment < 200 || victim->alignment > -200)
	    xp = base_exp * 1/2;

 	else
	    xp = base_exp;
    }

    /* more exp at the low levels */
//    if (gch->level < 6)
//   	xp = 10 * xp / (gch->level + 4);

    /* less at high */
//    if (gch->level > 35 && gch->level < 45)
//	xp =  15 * xp / (gch->level - 5 );

/*    if (gch->level >= 45 && gch->level < 55)
        xp = 15 * xp / (gch->level - 7 );

    if (gch->level >= 55 && gch->level < 73)
	xp =  15 * xp / (gch->level - 10 );

    if (gch->level >= 73 && gch->level < 75)
        xp = 15 * xp / (gch->level - 12);

    if (gch->level >= 75 && gch->level < 80)
	xp =  15 * xp / (gch->level - 15 );

    if (gch->level >= 80 && gch->level < 85)
        xp =  15 * xp / (gch->level - 17 );

    if (gch->level >= 85 && gch->level < 88)
	xp =  15 * xp / (gch->level - 20 );

    if (gch->level >= 88 && gch->level < 90)
        xp = 15 * xp / (gch->level - 23 );
*/

	xp = xp*2;
	xp = xp>1000?1000:xp;
 
    if (gch->level >= 90)
        xp = 0;
    

    /* reduce for playing time */
//    else
//    {
	/* compute quarter-hours per level */
//	time_per_level = 4 *
//			 (gch->played + (int) (current_time - gch->logon))/3600
//			 / gch->level;
//	time_per_level = URANGE(2,time_per_level,12);
//	if (gch->level < 15)  /* make it a curve */
//	    time_per_level = UMAX(time_per_level,(15 - gch->level));
//	xp = xp * time_per_level / 12;
//    }
   
    /* randomize the rewards */
    xp = number_range (xp * 3/4, xp * 5/4);

    /* adjust for grouping */
    xp = xp * gch->level/total_levels;

    return xp;
}


void dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune )
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;

	 if ( dam ==   0 ) { vs = "^BWmiss^NW";    vp = "^BWmisses^NW"; }
    else if ( dam <=   4 ) { vs = "^BWscratch^NW"; vp = "^BWscratches^NW";}
    else if ( dam <=   8 ) { vs = "^BWgraze^NW"; vp = "^BWgrazes^NW";	}
    else if ( dam <=  12 ) { vs = "^BWhit^NW";	vp = "^BWhits^NW";	}
    else if ( dam <=  16 ) { vs = "^BWinjure^NW";vp = "^BWinjures^NW";	}
    else if ( dam <=  20 ) { vs = "^BWwound^NW"; vp = "^BWwounds^NW";	}
    else if ( dam <=  24 ) { vs = "^BWmaul^NW";  vp = "^BWmauls^NW";	}
    else if ( dam <=  28 ) { vs = "^BWdecimate^NW";vp = "^BWdecimates^NW";}
    else if ( dam <=  32 ) { vs = "^BWdevastate^NW";vp = "^BWdevastates^NW";}
    else if ( dam <=  36 ) { vs = "^BWmaim^NW";	vp = "^BWmaims^NW";	}
    else if ( dam <=  40 ) { vs = "^BRMUTILATE^NW";vp = "^BRMUTILATES^NW";}
    else if ( dam <=  44 ) { vs = "^BRDISEMBOWEL^NW";vp = "^BRDISEMBOWELS^NW";	}
    else if ( dam <=  48 ) { vs = "^BRDISMEMBER^NW";vp = "^BRDISMEMBERS^NW";}
    else if ( dam <=  52 ) { vs = "^BRMASSACRE^NW";vp = "^BRMASSACRES^NW";	}
    else if ( dam <=  56 ) { vs = "^BRMANGLE^NW";vp = "^BRMANGLES^NW";		}
    else if ( dam <=  60 ) { vs = "^NR*** DEMOLISH ***^NW";
			     vp = "^NR*** DEMOLISHES ***^NW";			}
    else if ( dam <=  75 ) { vs = "^NR*** DEVASTATE ***^NW";
			     vp = "^NR*** DEVASTATES ***^NW";			}
    else if ( dam <= 100)  { vs = "^NR=== OBLITERATE ===^NW";
			     vp = "^NR=== OBLITERATES ===^NW";		}
    else if ( dam <= 125)  { vs = "^NR>>> ANNIHILATE <<<^NW";
			     vp = "^NR>>> ANNIHILATES <<<^NW";		}
    else if ( dam <= 150)  { vs = "^NR<<< ERADICATE >>>^NW";
			     vp = "^NR<<< ERADICATES >>>^NW";			}
    else if ( dam <= 400)  { vs = "^NR-*- DESOLATE -*-^NW";
			     vp = "^NR-*- DESOLATES -*-^NW";			}
    else if ( dam <= 700)  { vs = "^NR-=< RAVAGE >=-^NW";
			     vp = "^NR-=< RAVAGES >=-^NW";	}
    else if ( dam <= 900)  { vs = "^NR!!! SLAUGHTER !!!^NW";
			     vp = "^NR!!! SLAUGHTERS !!!^NW";		}
    else		   { vs = "^NR-=( MORTIFY )=-^NW";
			     vp = "^NR-=( MORTIFIES )=-^NW";	}

    punct   = (dam <= 24) ? '.' : '!';

    if ( dt == TYPE_HIT )
    {
	if (ch  == victim)
	{
	    sprintf( buf1, "$n %s $melf%c",vp,punct);
	    sprintf( buf2, "You %s yourself%c",vs,punct);
	}
	else
	{
	    sprintf( buf1, "$n %s $N%c",  vp, punct );
	    sprintf( buf2, "You %s $N%c [%d]", vs, punct, dam );
	    sprintf( buf3, "$n %s you%c", vp, punct );
	}
    }
    else
    {
	if ( dt >= 0 && dt < MAX_SKILL )
	    attack	= skill_table[dt].noun_damage;
	else if ( dt >= TYPE_HIT
	&& dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE) 
	    attack	= attack_table[dt - TYPE_HIT].noun;
	else
	{
	    bug( "Dam_message: bad dt %d.", dt );
	    dt  = TYPE_HIT;
	    attack  = attack_table[0].noun;
	}

	if (immune)
	{
	    if (ch == victim)
	    {
		sprintf(buf1,"$n is unaffected by $s own %s.",attack);
		sprintf(buf2,"Luckily, you are immune to that.");
	    } 
	    else
	    {
	    	sprintf(buf1,"$N is unaffected by $n's %s!",attack);
	    	sprintf(buf2,"$N is unaffected by your %s!",attack);
	    	sprintf(buf3,"$n's %s is powerless against you.",attack);
	    }
	}
	else
	{
	    if (ch == victim)
	    {
		sprintf( buf1, "$n's %s %s $m%c",attack,vp,punct);
		sprintf( buf2, "Your %s %s you%c",attack,vp,punct);
	    }
	    else
	    {
	    	sprintf( buf1, "$n's %s %s $N%c",  attack, vp, punct );
	    	sprintf( buf2, "Your %s %s $N%c [%d]",  attack, vp, punct, dam );
	    	sprintf( buf3, "$n's %s %s you%c", attack, vp, punct );
	    }
	}
    }

    if (ch == victim)
    {
	act(buf1,ch,NULL,NULL,TO_ROOM);
	act(buf2,ch,NULL,NULL,TO_CHAR);
    }
    else
    {
    	act( buf1, ch, NULL, victim, TO_NOTVICT );
    	act( buf2, ch, NULL, victim, TO_CHAR );
    	act( buf3, ch, NULL, victim, TO_VICT );
    }

    return;
}



/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
	return;

    if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
	act("$S weapon won't budge!",ch,NULL,victim,TO_CHAR);
	act("$n tries to disarm you, but your weapon won't budge!",
	    ch,NULL,victim,TO_VICT);
	act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
	return;
    }

    act( "$n DISARMS you and sends your weapon flying!", 
	 ch, NULL, victim, TO_VICT    );
    act( "You disarm $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n disarms $N!",  ch, NULL, victim, TO_NOTVICT );

    obj_from_char( obj );
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) )
	obj_to_char( obj, victim );
    else
    {
	obj_to_room( obj, victim->in_room );
	if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
	    get_obj(victim,obj,NULL);
    }

    return;
}

void do_berserk( CHAR_DATA *ch, char *argument)
{
    int chance, hp_percent;

    if ((chance = get_skill(ch,gsn_berserk)) == 0
    ||  (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BERSERK))
    ||  (!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_berserk].skill_level[ch->class]))
    {
	send_to_char("You turn red in the face, but nothing happens.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_BERSERK) || is_affected(ch,gsn_berserk)
    ||  is_affected(ch,skill_lookup("frenzy")))
    {
	send_to_char("You get a little madder.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
	send_to_char("You're feeling to mellow to berserk.\n\r",ch);
	return;
    }

    if (ch->mana < 50)
    {
	send_to_char("You can't get up enough energy.\n\r",ch);
	return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit/ch->max_hit;
    chance += 25 - hp_percent/2;

    if (number_percent() < chance)
    {
	AFFECT_DATA af;

	WAIT_STATE(ch,PULSE_VIOLENCE);
	ch->mana -= 50;
	ch->move /= 2;

	/* heal a little damage */
	ch->hit += ch->level * 2;
	ch->hit = UMIN(ch->hit,ch->max_hit);

	send_to_char("Blood, BLOOD there must be MORE BLOOD!!\n\r",ch);
	act("$n gets a wild look in $s eyes.",ch,NULL,NULL,TO_ROOM);
	check_improve(ch,gsn_berserk,TRUE,2);

	af.type		= gsn_berserk;
	af.level	= ch->level;
	af.duration	= number_fuzzy(ch->level / 8);
	af.modifier	= UMAX(1,ch->level/5);
	af.bitvector 	= AFF_BERSERK;

	af.location	= APPLY_HITROLL;
	affect_to_char(ch,&af);

	af.location	= APPLY_DAMROLL;
	affect_to_char(ch,&af);

	af.modifier	= UMAX(10,10 * (ch->level/5));
	af.location	= APPLY_AC;
	affect_to_char(ch,&af);
    }

    else
    {
	WAIT_STATE(ch,3 * PULSE_VIOLENCE);
	ch->mana -= 25;
	ch->move /= 2;

	send_to_char("Your pulse speeds up, but nothing happens.\n\r",ch);
	check_improve(ch,gsn_berserk,FALSE,2);
    }
}

void do_bash( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);
 
    if ( (chance = get_skill(ch,gsn_bash)) == 0
    ||	 (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BASH))
    ||	 (!IS_NPC(ch)
    &&	  ch->level < skill_table[gsn_bash].skill_level[ch->class]))
    {	
	send_to_char("Bashing? What's that?\n\r",ch);
	return;
    }
 
    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
	return;
    } 

    if (victim == ch)
    {
	send_to_char("You try to bash your brains out, but fail.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;
/*
    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }
*/
    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (IS_AFFECTED(victim,AFF_PASS_DOOR))
    {
	act("$n isn't solid. You can't bash $s.",victim,NULL,ch,TO_VICT);
	return;
    }

    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 25;
    chance -= victim->carry_weight / 20;

    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 25;
    else
	chance += (ch->size - victim->size) * 10; 


    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= get_curr_stat(victim,STAT_DEX) * 4/3;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST))
	chance += 10;
    if (IS_SET(ch->off_flags,OFF_FAST))
	chance -= 20;

    /* level */
    chance += (ch->level - victim->level);

    /* now the attack */
    if (number_percent() < chance)
    {

        chance = (int) get_skill(victim,gsn_evade)*.25;
        if( number_percent() < chance )
        {
	    check_improve(victim,gsn_evade,TRUE,2);
	    act("You evade $n's bash, sending $m to the ground.",
	        ch,NULL,victim,TO_VICT);
            act("$N evades your bash and sends you to the ground.",
		ch,NULL,victim,TO_CHAR);
	    act("$N evades $n's bash, sending $m to the ground.",
		ch,NULL,victim,TO_NOTVICT);
	    WAIT_STATE(ch,skill_table[gsn_bash].beats);
	    ch->position=POS_RESTING;
	    return;
        }
            
	act("$n sends you sprawling with a powerful bash!",
		ch,NULL,victim,TO_VICT);
	act("You slam into $N, and send $M flying!",ch,NULL,victim,TO_CHAR);
	act("$n sends $N sprawling with a powerful bash.",
		ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_bash,TRUE,1);

	WAIT_STATE(victim, skill_table[gsn_bash].beats);
	WAIT_STATE(ch,skill_table[gsn_bash].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(2,2 + 2 * ch->size + chance/20),gsn_bash,
	    DAM_BASH);
	
    }
    else
    {
	damage(ch,victim,0,gsn_bash,DAM_BASH);
	act("You fall flat on your face!",
	    ch,NULL,victim,TO_CHAR);
	act("$n falls flat on $s face.",
	    ch,NULL,victim,TO_NOTVICT);
	act("You evade $n's bash, causing $m to fall flat on $s face.",
	    ch,NULL,victim,TO_VICT);
	check_improve(ch,gsn_bash,FALSE,1);
	ch->position = POS_RESTING;
	WAIT_STATE(ch,skill_table[gsn_bash].beats * 2/3); 
    }
}

void do_dirt( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    char buf[MAX_STRING_LENGTH];

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_dirt)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK_DIRT))
    ||   (!IS_NPC(ch)
    &&    ch->level < skill_table[gsn_dirt].skill_level[ch->class]))
    {
	send_to_char("You get your feet dirty.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't in combat!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(victim,AFF_BLIND))
    {
	act("$e's already been blinded.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;
/*
    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }
*/
    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    if(!IS_NPC(ch) && !IS_NPC(victim) && ch->fighting == NULL)
    {
    if(( ch->level+7 < victim->level ) 
      && !(IS_SET(ch->act,PLR_IMMQUEST) && IS_SET(ch->act,PLR_IMMQUEST)))
    {
	ch->pcdata->virtual_level[0] = victim->level;
	ch->pcdata->virtual_level[1] = 250;
    }

    if(!IS_NPC(ch) && !IS_NPC(victim))
    {
    	sprintf(buf,"$N(%d) is attempting to murder %s(%d).",ch->level,victim->name,victim->level);
    	wiznet(buf,ch,NULL,WIZ_ATTACKS,0,0);
    }
    }




    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,OFF_FAST))
	chance -= 25;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* terrain */

    switch(ch->in_room->sector_type)
    {
	case(SECT_INSIDE):		chance -= 20;	break;
	case(SECT_CITY):		chance -= 10;	break;
	case(SECT_FIELD):		chance +=  5;	break;
	case(SECT_FOREST):				break;
	case(SECT_HILLS):				break;
	case(SECT_MOUNTAIN):		chance -= 10;	break;
	case(SECT_WATER_SWIM):		chance  =  0;	break;
	case(SECT_WATER_NOSWIM):	chance  =  0;	break;
	case(SECT_AIR):			chance  =  0;  	break;
	case(SECT_DESERT):		chance += 10;   break;
    }

    if (chance == 0)
    {
	send_to_char("There isn't any dirt to kick.\n\r",ch);
	return;
    }

    /* now the attack */
    if (number_percent() < chance)
    {
	AFFECT_DATA af;
	act("$n is blinded by the dirt in $s eyes!",victim,NULL,NULL,TO_ROOM);
        damage(ch,victim,number_range(2,5),gsn_dirt,DAM_NONE);
	send_to_char("You can't see a thing!\n\r",victim);
	check_improve(ch,gsn_dirt,TRUE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);

	af.type 	= gsn_dirt;
	af.level 	= ch->level;
	af.duration	= 0;
	af.location	= APPLY_HITROLL;
	af.modifier	= -4;
	af.bitvector 	= AFF_BLIND;

	affect_to_char(victim,&af);
    }
    else
    {
	damage(ch,victim,0,gsn_dirt,DAM_NONE);
	check_improve(ch,gsn_dirt,FALSE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);
    }
}

void do_trip( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    char buf[MAX_STRING_LENGTH];

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_trip)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TRIP))
    ||   (!IS_NPC(ch) 
	  && ch->level < skill_table[gsn_trip].skill_level[ch->class]))
    {
	send_to_char("Tripping?  What's that?\n\r",ch);
	return;
    }


    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
 	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;
/*
    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
	send_to_char("Kill stealing is not permitted.\n\r",ch);
	return;
    }
  */  
    if (IS_AFFECTED(victim,AFF_FLYING))
    {
	act("$S feet aren't on the ground.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("$N is already down.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You fall flat on your face!\n\r",ch);
	WAIT_STATE(ch,2 * skill_table[gsn_trip].beats);
	act("$n trips over $s own feet!",ch,NULL,NULL,TO_ROOM);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
	return;
    }


    if((!IS_NPC(ch) && !IS_NPC(victim) && ch->fighting == NULL)
      && !(IS_SET(ch->act,PLR_IMMQUEST) && IS_SET(ch->act,PLR_IMMQUEST)))
    {
    if( ch->level+7 < victim->level )
    {
	ch->pcdata->virtual_level[0] = victim->level;
	ch->pcdata->virtual_level[1] = 250;
    }

    if(!IS_NPC(ch) && !IS_NPC(victim))
    {
    	sprintf(buf,"$N(%d) is attempting to murder %s(%d).",ch->level,victim->name,victim->level);
    	wiznet(buf,ch,NULL,WIZ_ATTACKS,0,0);
    }
    }



    /* modifiers */

    /* size */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10;  /* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 20;

    /* level */
    chance += (ch->level - victim->level);


    /* now the attack */
    if (number_percent() < chance)
    {

        chance = (int) get_skill(victim,gsn_jump)*.25;
        if( number_percent() < chance )
        {
	    check_improve(victim,gsn_jump,TRUE,2);
	    act("You jump over $n's leg, making $m miss $s trip.",
	        ch,NULL,victim,TO_VICT);
            act("$N jumps over your leg, causing you to miss your trip.",
		ch,NULL,victim,TO_CHAR);
	    act("$N jumps over $n's leg, causing $m to miss $s trip.",
		ch,NULL,victim,TO_NOTVICT);
	    WAIT_STATE(ch,skill_table[gsn_trip].beats);
	    return;
        }

	act("$n trips you and you go down!",ch,NULL,victim,TO_VICT);
	act("You trip $N and $N goes down!",ch,NULL,victim,TO_CHAR);
	act("$n trips $N, sending $M to the ground.",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_trip,TRUE,1);

	WAIT_STATE(victim,skill_table[gsn_trip].beats);
        WAIT_STATE(ch,skill_table[gsn_trip].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(2, 2 +  2 * victim->size),gsn_trip,
	    DAM_BASH);
    }
    else
    {
	damage(ch,victim,0,gsn_trip,DAM_BASH);
	WAIT_STATE(ch,skill_table[gsn_trip].beats*2/3);
	check_improve(ch,gsn_trip,FALSE,1);
    } 
}



void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

//We don't need a murder command 
/*   if ( !IS_NPC(victim) )
    {
        if ( !IS_SET(victim->act, PLR_KILLER)
        &&   !IS_SET(victim->act, PLR_THIEF) )
        {
            send_to_char( "You must MURDER a player.\n\r", ch );
            return;
        }
    }*/
    if (!IS_NPC(victim) && ch->level < 16)
    {
        send_to_char("You have to be level 16 to attack other players.\n\r",ch);
        return;
    }

    if( !IS_NPC(victim) && victim->level < 16 && !IS_IMMORTAL(ch))
    {
	send_to_char( "You may not kill them until they reach level 16.\n\r",ch);
	return;
    }


    if ( victim == ch )
    {
	send_to_char( "You hit yourself.  Ouch!\n\r", ch );
//	multi_hit( ch, ch, TYPE_UNDEFINED );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

/*    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }*/

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    if(!IS_NPC(ch) && !IS_NPC(victim) && ch->fighting == NULL)
    {
    if(( ch->level+7 < victim->level )
      && !(IS_SET(ch->act,PLR_IMMQUEST) && IS_SET(ch->act,PLR_IMMQUEST)))
    {
	ch->pcdata->virtual_level[0] = victim->level;
	ch->pcdata->virtual_level[1] = 250;
    }

    if(!IS_NPC(ch) && !IS_NPC(victim))
    {
    	sprintf(buf,"$N(%d) is attempting to murder %s(%d).",ch->level,victim->name,victim->level);
    	wiznet(buf,ch,NULL,WIZ_ATTACKS,0,0);
    }
    }


    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    check_killer( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}

//Getting rid of the murder command.
/*
void do_murde( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to MURDER, spell it out.\n\r", ch );
    return;
}



void do_murder( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Murder whom?\n\r", ch );
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) || (IS_NPC(ch) && IS_SET(ch->act,ACT_PET)))
	return;

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    if (IS_NPC(ch))
	sprintf(buf, "Help! I am being attacked by %s!",ch->short_descr);
    else
    	sprintf( buf, "Help!  I am being attacked by %s!", ch->name );
    do_yell( victim, buf );
    check_killer( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}
*/


void do_backstab( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Backstab whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
      return;
/*
    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
      $ return;
    }
*/
    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
	send_to_char( "You need to wield a weapon to backstab.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
    {
	send_to_char( "You can't backstab a fighting person.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && victim->level>15 && ch->level<16)
    {
        send_to_char("You have to be level 16 to do that!\n\r", ch );
        return;
    }
    if ( victim->hit < (int) victim->max_hit - victim->max_hit/3 )
    {
	act( "$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }

    if(!IS_NPC(ch) && !IS_NPC(victim) && ch->fighting == NULL)
    {
    if(( ch->level+7 < victim->level )
      && !(IS_SET(ch->act,PLR_IMMQUEST) && IS_SET(ch->act,PLR_IMMQUEST)))
    {
	ch->pcdata->virtual_level[0] = victim->level;
	ch->pcdata->virtual_level[1] = 250;
    }

    if(!IS_NPC(ch) && !IS_NPC(victim))
    {
    	sprintf(buf,"$N(%d) is attempting to murder %s(%d).",ch->level,victim->name,victim->level);
    	wiznet(buf,ch,NULL,WIZ_ATTACKS,0,0);
    }
    }



    check_killer( ch, victim );
    WAIT_STATE( ch, skill_table[gsn_backstab].beats );
    if ( !IS_AWAKE(victim)
    ||   IS_NPC(ch)
    ||   number_percent( ) < ch->pcdata->learned[gsn_backstab] )
    {
	check_improve(ch,gsn_backstab,TRUE,1);
	multi_hit( ch, victim, gsn_backstab );
    }
    else
    {
	check_improve(ch,gsn_backstab,FALSE,1);
	damage( ch, victim, 0, gsn_backstab,DAM_NONE );
    }

    return;
}
void do_circle( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim=ch->fighting;
    OBJ_DATA *obj;

    if (victim==NULL)
    {
        send_to_char( "You must be fighting to circle.\n\r", ch );
	return;
    }     

    if(IS_NPC(ch))
   	return;

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
	send_to_char( "You need to wield a weapon to circle.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_circle].beats );

    if  (number_percent( ) < ch->pcdata->learned[gsn_circle] )
    {
	check_improve(ch,gsn_circle,TRUE,3);
	multi_hit( ch, victim, gsn_circle );
    }
    else
    {
	check_improve(ch,gsn_circle,FALSE,3);
	damage( ch, victim, 0, gsn_circle,DAM_NONE );
    }

    return;
}



void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    int attempt;

    if ( ( victim = ch->fighting ) == NULL )
    {
        if ( ch->position == POS_FIGHTING )
            ch->position = POS_STANDING;
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;

	door = number_door( );
	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
	||   IS_SET(pexit->exit_info, EX_CLOSED)
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

	move_char( ch, door, FALSE );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act( "$n pulled up their skirt and ran like a little girl!", ch,
NULL, NULL, TO_ROOM );
	ch->in_room = now_in;

	if ( !IS_NPC(ch) )
	{
	    send_to_char( "You hike up your skirt and run like a little girl! You lose 10 exps.\n\r", ch );
	    gain_exp( ch, -10 );
	}

	stop_fighting( ch, TRUE );
	return;
    }

    send_to_char( "PANIC! You couldn't escape! Ut-Oh! \n\r", ch );
    return;
}



void do_rescue( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Rescue whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "What about fleeing instead?\n\r", ch );
	return;
    }
/*
    if ( !is_same_group(ch,victim))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }
*/
    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	send_to_char( "Doesn't need your help!\n\r", ch );
	return;
    }

    if ( ch->fighting == victim )
    {
	send_to_char( "Too late.\n\r", ch );
	return;
    }

    if ( ( fch = victim->fighting ) == NULL )
    {
	send_to_char( "That person is not fighting right now.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_rescue].beats );
    if ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[gsn_rescue] )
    {
	send_to_char( "You fail the rescue.\n\r", ch );
	check_improve(ch,gsn_rescue,FALSE,1);
	return;
    }

    if(!IS_NPC(ch) && !IS_NPC(fch))
    {
    if(( ch->level+7 < fch->level )
      && !(IS_SET(ch->act,PLR_IMMQUEST) && IS_SET(ch->act,PLR_IMMQUEST)))
    {
	ch->pcdata->virtual_level[0] = fch->level;
	ch->pcdata->virtual_level[1] = 250;
    }

    if(!IS_NPC(ch) && !IS_NPC(fch))
    {
    	sprintf(buf,"$N(%d) is attempting to murder %s(%d).",ch->level,fch->name,fch->level);
    	wiznet(buf,ch,NULL,WIZ_ATTACKS,0,0);
    }
    }

    act( "You rescue $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n rescues you!", ch, NULL, victim, TO_VICT    );
    act( "$n rescues $N!",  ch, NULL, victim, TO_NOTVICT );
    check_improve(ch,gsn_rescue,TRUE,1);

    stop_fighting( fch, FALSE );
    stop_fighting( victim, FALSE );

    check_killer( ch, fch );
    set_fighting( ch, fch );
    set_fighting( fch, ch );
    return;
}



void do_kick( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_kick].skill_level[ch->class] )
    {
	send_to_char(
	    "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }
   
    if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK))
	return;

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if(!IS_NPC(ch) && !IS_NPC(victim))
    {
    if(( ch->level+7 < victim->level )
      && !(IS_SET(ch->act,PLR_IMMQUEST) && IS_SET(ch->act,PLR_IMMQUEST)))
    {
	ch->pcdata->virtual_level[0] = victim->level;
	ch->pcdata->virtual_level[1] = 250;
    }

    if(!IS_NPC(ch) && !IS_NPC(victim))
    {
    	sprintf(buf,"$N(%d) is attempting to murder %s(%d).",ch->level,victim->name,victim->level);
    	wiznet(buf,ch,NULL,WIZ_ATTACKS,0,0);
    }
    }

    WAIT_STATE( ch, skill_table[gsn_kick].beats );
    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_kick] )
    {
	damage( ch, victim, number_range( 1, ch->level ), gsn_kick,DAM_BASH );
	check_improve(ch,gsn_kick,TRUE,1);
    }
    else
    {
	damage( ch, victim, 0, gsn_kick,DAM_BASH );
	check_improve(ch,gsn_kick,FALSE,1);
    }

    return;
}

void do_thrash( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_kick].skill_level[ch->class] )
    {
	send_to_char(
	    "You try to thrash about, but it looks like you're having an epilectic seizure instead.\n\r", ch );
	return;
    }
   
    if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK))
	return;

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if(!IS_NPC(ch) && !IS_NPC(victim))
    {
    if(( ch->level+7 < victim->level )
      && !(IS_SET(ch->act,PLR_IMMQUEST) && IS_SET(ch->act,PLR_IMMQUEST)))
    {
	ch->pcdata->virtual_level[0] = victim->level;
	ch->pcdata->virtual_level[1] = 250;
    }

    if(!IS_NPC(ch) && !IS_NPC(victim))
    {
    	sprintf(buf,"$N(%d) is attempting to murder %s(%d).",ch->level,victim->name,victim->level);
    	wiznet(buf,ch,NULL,WIZ_ATTACKS,0,0);
    }
    }

    WAIT_STATE( ch, skill_table[gsn_kick].beats );
    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_kick] )
    {
	damage( ch, victim, number_range( 1, ch->level ), gsn_kick,DAM_BASH );
	check_improve(ch,gsn_kick,TRUE,1);
    }
    else
    {
	damage( ch, victim, 0, gsn_kick,DAM_BASH );
	check_improve(ch,gsn_kick,FALSE,1);
    }

    return;
}




void do_disarm( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon;

    hth = 0;

    if ((chance = get_skill(ch,gsn_disarm)) == 0)
    {
	send_to_char( "You don't know how to disarm opponents.\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL 
    &&   ((hth = get_skill(ch,gsn_hand_to_hand)) == 0
    ||    (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_DISARM))))
    {
	send_to_char( "You must wield a weapon to disarm.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
    {
	send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
	return;
    }


    /* find weapon skills */
    ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch));
    vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim));
    ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim));

    /* modifiers */

    /* skill */
    if ( get_eq_char(ch,WEAR_WIELD) == NULL)
	chance = chance * hth/150;
    else
	chance = chance * ch_weapon/100;

    chance += (ch_vict_weapon/2 - vict_weapon) / 2; 

    /* dex vs. strength */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_STR);

    chance -= get_skill(victim,gsn_grip);
    check_improve(victim,gsn_grip,TRUE,1);

    /* level */
    chance += (ch->level - victim->level) * 2;
 
    /* and now the attack */
    if (number_percent() < chance)
    {
    	WAIT_STATE( ch, skill_table[gsn_disarm].beats );
	disarm( ch, victim );
	check_improve(ch,gsn_disarm,TRUE,1);
    }
    else
    {
	WAIT_STATE(ch,skill_table[gsn_disarm].beats);
	act("You fail to disarm $N.",ch,NULL,victim,TO_CHAR);
	act("$n tries to disarm you, but fails.",ch,NULL,victim,TO_VICT);
	act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_disarm,FALSE,1);
    }
    return;
}



void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}



void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slay whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && victim->level >= get_trust(ch) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    act( "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR    );
    act( "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
    act( "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );


    sprintf(buf,"%s(%d) slew %s%s(%d) at [%d].",ch->name,ch->trust,IS_NPC(victim)?"MOB ":"",victim->name,victim->level,ch->in_room->vnum);

    if (IS_NPC(victim))
        wiznet(buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
    else
        wiznet(buf,NULL,NULL,WIZ_DEATHS,0,0);

    raw_kill( victim );
    return;
}

void do_crush( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

return;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_crush)) == 0
	 || ch->race != race_lookup("giant") )
    {	
	send_to_char("Crush? You aren't nearly big enough.\n\r",ch);
	return;
    }
 
    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("You'll have to let $M recover first.",ch,NULL,victim,TO_CHAR);
	return;
    } 

    if (victim == ch)
    {
	send_to_char("You try to crush yourself, but can't seem to fit your arms around yourself.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (IS_AFFECTED(victim,AFF_PASS_DOOR))
    {
	act("$n isn't solid. You can't crush $s.",victim,NULL,ch,TO_VICT);
	return;
    }

    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 25;
    chance -= victim->carry_weight / 20;

    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 25;
    else
	chance += (ch->size - victim->size) * 10; 


    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= get_curr_stat(victim,STAT_DEX) * 4/3;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST))
	chance += 10;
    if (IS_SET(ch->off_flags,OFF_FAST))
	chance -= 20;

    /* level */
    chance += (ch->level - victim->level);

    /* now the attack */
    if (number_percent() < chance)
    {
	act("$n grabs you and crushes the life from your limbs.",
		ch,NULL,victim,TO_VICT);
	act("You grab $N and crush the life out of $M.",ch,NULL,victim,TO_CHAR);
	act("$n grabs $N and crushes the life out of $M.",
		ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_crush,TRUE,1);

	WAIT_STATE(victim, 2 * PULSE_VIOLENCE);
	WAIT_STATE(ch,skill_table[gsn_crush].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(5,(2 + 2 * ch->size + chance/20)*2),gsn_crush,DAM_BASH);
    }
    else
    {
	damage(ch,victim,0,gsn_crush,DAM_BASH);
	act("You grab at $N and miss!",
	    ch,NULL,victim,TO_CHAR);
	act("$n tries to grab $N but misses.",
	    ch,NULL,victim,TO_NOTVICT);
	act("You evade $n's crush, causing $m flail wildly about.",
	    ch,NULL,victim,TO_VICT);
	check_improve(ch,gsn_crush,FALSE,1);
	WAIT_STATE(ch,skill_table[gsn_crush].beats * 2/3); 
    }



}

