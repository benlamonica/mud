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
#include <time.h>
#include "merc.h"

/* command procedures needed */
DECLARE_DO_FUN(do_quit	);

char *format_string( CHAR_DATA *ch, char *argument )
{
   char *formatted=NULL;
    unsigned long length;
   int orig,format,i,x,LastWord;
   char buf[MAX_STRING_LENGTH];

   length=strlen(argument);

   if(length+UMAX(strlen(IS_NPC(ch)?ch->short_descr:ch->name),7)+7 > 77)
   {
	orig=format=0;
    	formatted = (char *)malloc(length+5*(length/71)+15);
	for(i=0;i<=length/71;i++)
        {
	    LastWord = 0;
	    for(x=1;x<=71;x++)
            {
		*(formatted+format)=*(argument+orig);
		sprintf(buf,"X: %d, Letter: %c",x,*(argument+orig));
		log_string(buf);
		if(*(argument+orig-1)=='\0')
		{
		    *(formatted+format+1)='\0';
		    LastWord = -1;
		    break;
		}
		if(*(formatted+format)==' ')
		{
		    LastWord = x;
		    sprintf(buf,"LastWord: %d",LastWord);
		    log_string(buf);
		}
		format++;
		orig++;
	    }
	    *(formatted+format) = '\0';

	    /*If LastWord is -1, then the loop broke, 
	      and you are done formatting the string.*/
	    if(LastWord == -1)
		break;
	    else if(LastWord == 71)
	    {
		*(formatted+format+1)='\0';
		strcat(formatted,"\n\r    ");
		format+=6;
		continue;
	    }
	    else if(LastWord == 0)
	    {
		orig--;
		*(formatted+format)='-';
		format++;
		*(formatted+format)='\0';
	    }
	    else
	    {
		orig-=(71-LastWord);
		format-=(71-LastWord);
		*(formatted+format)='\0';
	    }
	    strcat(formatted,"\n\r    ");
	    format+=6;
	}
    }
    else
	formatted=argument;	
    return formatted;
}

void do_afk( CHAR_DATA *ch, char *argument )
{
   if(IS_SET(ch->comm,COMM_AFK))
   {
	REMOVE_BIT(ch->comm,COMM_AFK);
	return;
   }
   else
   {
	SET_BIT(ch->comm,COMM_AFK);
	return;
   }
}

void save_pos(CHAR_DATA *ch)
{
    char buf[100];

    sprintf(buf,"%c[s",27);
    write_to_buffer2(ch->desc,buf,0);
}

void restore_pos(CHAR_DATA *ch)
{
    char buf[100];

    sprintf(buf,"%c[u",27);
    write_to_buffer2(ch->desc,buf,0);
}

void erase_line(CHAR_DATA *ch)
{
    char buf[100];

    sprintf(buf,"%c[K",27);
    write_to_buffer2(ch->desc,buf,0);
}

void go_home(CHAR_DATA *ch)
{
    char buf[100];

    sprintf(buf,"%c[H",27);
    write_to_buffer2(ch->desc,buf,0);
}

void goto_xy(int x, int y, CHAR_DATA *ch)
{
    char buf[100];

    sprintf(buf,"%c[%d;%dH",27,y,x);
    write_to_buffer2(ch->desc,buf,0);
}

void clear_screen(CHAR_DATA *ch)
{
    char buf[100];

    sprintf(buf,"%c[2J%c[s",27,27);
    write_to_buffer2(ch->desc,buf,0);
}


void do_lines( CHAR_DATA *ch, char *argument )
{
    int new_lines;


 
    new_lines = atoi(argument);

    if(new_lines < 4 || new_lines > 60)
    {
	send_to_char("The number of lines on your screen must be at least 4 and less than 60.\n\r",ch);
	return;
    }

    ch->screen_height = new_lines;
}

void set_scroll( int top, int bottom, CHAR_DATA *ch)
{
    char buf[100];

    sprintf(buf,"%c[%d;%dr",27,top,bottom);
    write_to_buffer2(ch->desc,buf,0);
}

void reset_vt100( CHAR_DATA *ch )
{
    char buf[100];

    sprintf(buf,"%cc",27);
    write_to_buffer2(ch->desc,buf,0);
}

void do_vt100( CHAR_DATA *ch, char *argument )
{
    char buf[100];


    if(IS_SET(ch->comm,COMM_VT100))
    {
	REMOVE_BIT(ch->comm, COMM_VT100);
	clear_screen(ch);
	go_home(ch);
	reset_vt100(ch);
	send_to_char("VT100 mode is now off.\n\r",ch);
	return;

    }

    clear_screen(ch);
    go_home(ch);
    set_scroll(0,ch->screen_height-4,ch);

    write_to_buffer1(ch->desc,"VT100 mode is now on. Use the LINES command to tell the mud how many rows\n\r",0);
    sprintf(buf,"you have on your display (Currently: %d lines).\n\r",ch->screen_height);
    write_to_buffer1(ch->desc,buf,0);
    save_pos(ch);

    SET_BIT(ch->comm, COMM_VT100);
}

/*Beep function*/
void do_beep( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    
    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
	send_to_char( "Your beep didn't get through.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch);
	return;
    }

    if ( argument[0]=='\0' )
    {
    	if ( IS_SET( ch->comm, COMM_BEEP ) )
    	{
    	    REMOVE_BIT( ch->comm, COMM_BEEP );
    	    send_to_char( "You no longer accept beeps.\n\r", ch );
    	}
    	else
    	{
    	    SET_BIT( ch->comm, COMM_BEEP );
    	    send_to_char( "You now accept beeps.\n\r", ch );
    	}
        return;
    }
    
    if ( !IS_SET( ch->comm, COMM_BEEP ) )
    {
    	send_to_char( "You have to turn on the beep channel first.\n\r", ch);
    	return;
    }
    
    if ( ( victim=get_char_world( ch, argument ) )==NULL )
    {
        send_to_char( "Nobody like that.\n\r", ch );
        return;
    }
    
    if ( !IS_SET( victim->comm, COMM_BEEP ) )
    {
    	act_new("$N is not receiving beeps.", ch, NULL, victim, TO_CHAR, 
POS_DEAD);
    	return;
    }
    
    act_new( "\aYou beep to $N.", ch, NULL, victim, TO_CHAR, POS_DEAD );
    act_new( "\a$n beeps you.", ch, NULL, victim, TO_VICT, POS_DEAD );

    return;
}


void do_guildtalk( CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if(IS_NPC(ch))
	return;

    if(ch->pcdata->guild==0)
    {
	send_to_char("But you aren't a member of any guild!\n\r",ch);
	return;
    }

    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch);
	return;
    }

    if (IS_SET(ch->comm,COMM_NOCHANNELS))
    {
        send_to_char("You cannot use channels.\n\r",ch);
	return;
    }

    if(argument[0] == 0)
    {
	for(d=descriptor_list;d!=NULL;d=d->next)
    	{
            if ( d->connected != CON_PLAYING )
            	continue;
	    if ( IS_NPC(d->character) )
		continue;
	    if ( d->character->pcdata->guild != ch->pcdata->guild )
	    	continue;
            if ( IS_SET(d->character->comm,COMM_QUIET))
	    	continue;
	    if (get_trust(d->character) > 90 && !can_see(ch,d->character))
		continue;

	    sprintf(buf,"(%d)%s %s%s\n\r",d->character->pcdata->guild_rank,guild_table[ch->pcdata->guild].screen,d->character->name,d->character->pcdata->title);
 	    send_to_char(buf,ch);
    	}
        return;
    }
	

    for(d=descriptor_list;d!=NULL;d=d->next)
    {
        if ( d->connected != CON_PLAYING )
            continue;
	if ( IS_NPC(d->character) )
	    continue;
	if ( d->character->pcdata->guild != ch->pcdata->guild )
	    continue;
        if ( IS_SET(d->character->comm,COMM_QUIET))
	    continue;
	
	if(ch->pcdata->guild_rank==0)
	sprintf(buf,"[^NRLeader^NW] %s: '^NG%s^NW'\n\r",ch->name,argument);
	else sprintf(buf,"%s (%d)%s: '^NG%s^NW'\n\r",guild_table[ch->pcdata->guild].screen,ch->pcdata->guild_rank,ch->name,argument);
 	send_to_char(buf,d->character);
    }
}

/* RT code to delete yourself */

void do_delet( CHAR_DATA *ch, char *argument)
{
    send_to_char("You must type the full command to delete yourself.\n\r",ch);
}

void do_delete( CHAR_DATA *ch, char *argument)
{
   char strsave[MAX_INPUT_LENGTH];

   if (IS_NPC(ch))
	return;
  
   if (ch->pcdata->confirm_delete)
   {
	if (argument[0] != '\0')
	{
	    send_to_char("Delete status removed.\n\r",ch);
	    ch->pcdata->confirm_delete = FALSE;
	    return;
	}
	else
	{
    	    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
	    wiznet("$N turns $Mself into line noise.",ch,NULL,0,0,0);
	    quit(ch);
	    unlink(strsave);
	    return;
 	}
    }

    if (argument[0] != '\0')
    {
	send_to_char("Just type delete. No argument.\n\r",ch);
	return;
    }

    send_to_char("Type delete again to confirm this command.\n\r",ch);
    send_to_char("WARNING: this command is irreversible.\n\r",ch);
    send_to_char("Typing delete with an argument will undo delete status.\n\r",
	ch);
    ch->pcdata->confirm_delete = TRUE;
    wiznet("$N is contemplating deletion.",ch,NULL,0,0,get_trust(ch));
}
	    

/* RT code to display channel status */

void do_channels( CHAR_DATA *ch, char *argument)
{
    /* lists all channels and their status */
    send_to_char("   channel     status\n\r",ch);
    send_to_char("---------------------\n\r",ch);
 
    send_to_char("gossip         ",ch);
    if (!IS_SET(ch->comm,COMM_NOGOSSIP))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("auction        ",ch);
    if (!IS_SET(ch->comm,COMM_NOAUCTION))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("music          ",ch);
    if (!IS_SET(ch->comm,COMM_NOMUSIC))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("newbie          ",ch);
    if (!IS_SET(ch->comm,COMM_NONEW))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("OOC          ",ch);
    if (!IS_SET(ch->comm,COMM_NOOOC))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("Q/A            ",ch);
    if (!IS_SET(ch->comm,COMM_NOQUESTION))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    if (IS_HERO(ch))
    {
      send_to_char("god channel    ",ch);
      if(!IS_SET(ch->comm,COMM_NOWIZ))
        send_to_char("ON\n\r",ch);
      else
        send_to_char("OFF\n\r",ch);
    }
    /* imp channel */
    if (IS_IMP(ch))
    {
	send_to_char("imp channel    ",ch);
	if(!IS_SET(ch->comm,COMM_NOIMPT))
	  send_to_char("ON\n\r",ch);
	else
	  send_to_char("OFF\n\r",ch);
    }
    send_to_char("shouts         ",ch);
    if (!IS_SET(ch->comm,COMM_DEAF))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("quiet mode     ",ch);
    if (IS_SET(ch->comm,COMM_QUIET))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);
   
    if (ch->lines != PAGELEN)
    {
	char buf[100];
	if (ch->lines)
	{
	    sprintf(buf,"You display %d lines of scroll.\n\r",ch->lines+2);
	    send_to_char(buf,ch);
 	}
	else
	    send_to_char("Scroll buffering is off.\n\r",ch);
    }

    if (IS_SET(ch->comm,COMM_NOSHOUT))
      send_to_char("You cannot shout.\n\r",ch);
  
    if (IS_SET(ch->comm,COMM_NOTELL))
      send_to_char("You cannot use tell.\n\r",ch);
 
    if (IS_SET(ch->comm,COMM_NOCHANNELS))
     send_to_char("You cannot use channels.\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOEMOTE))
      send_to_char("You cannot show emotions.\n\r",ch);

}

/* RT deaf blocks out all shouts */

void do_deaf( CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_NOSHOUT))
    {
      send_to_char("The gods have taken away your ability to shout.\n\r",ch);
      return;
    }
    
   if (IS_SET(ch->comm,COMM_DEAF))
   {
     send_to_char("You can now hear shouts again.\n\r",ch);
     REMOVE_BIT(ch->comm,COMM_DEAF);
   }
   else 
   {
     send_to_char("From now on, you won't hear shouts.\n\r",ch);
     SET_BIT(ch->comm,COMM_DEAF);
   }
}

/* RT quiet blocks out all communication */

void do_quiet ( CHAR_DATA *ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_QUIET))
    {
      send_to_char("Quiet mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_QUIET);
    }
   else
   {
     send_to_char("From now on, you will only hear says and emotes.\n\r",ch);
     SET_BIT(ch->comm,COMM_QUIET);
   }
}

/* RT chat replaced with ROM gossip */
void do_gossip( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOGOSSIP))
      {
        send_to_char("Gossip channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOGOSSIP);
      }
      else
      {
        send_to_char("Gossip channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOGOSSIP);
      }
    }
    else  /* gossip message sent, turn gossip on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel privileges.\n\r",ch);
          return;
 
       	}

      REMOVE_BIT(ch->comm,COMM_NOGOSSIP);
 
      sprintf( buf, "You gossip '^BC%s^nw'\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
 
        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOGOSSIP) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
          act_new( "$n gossips '^BC$t^nw'", 
		   ch,argument, d->character, TO_VICT,POS_SLEEPING );
        }
      }
    }
}

/* RT quest channel */
void do_question( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
	send_to_char("Say what on the quest channel?\n\r",ch);
	return;
    }

    else  /* question sent, turn Q/A on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel privileges.\n\r",ch);
          return;
	}
 
      sprintf( buf, "You quest '^NR%s^NW'\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
 
        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             IS_SET(victim->act,PLR_IMMQUEST) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
	  act_new("$n quest '^NR$t^NW'",
	 	  ch,argument,d->character,TO_VICT,POS_SLEEPING);
        }
      }
    }
}

/* RT newbie channel - uses same */
void do_newbie( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NONEW))
      {
        send_to_char("Newbie channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NONEW);
      }
      else
      {
        send_to_char("Newbie channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NONEW);
      }
    }
    else  /* answer sent, turn newbie on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel privileges.\n\r",ch);
          return;
	}
 
        REMOVE_BIT(ch->comm,COMM_NONEW);
 
      sprintf( buf, "You NEWBIE: '^BG%s^nw'\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
 
        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOQUESTION) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
	  act_new("$n NEWBIE: '^BG$t^nw'",
		  ch,argument,d->character,TO_VICT,POS_SLEEPING);
        }
      }
    }
}

void do_music( CHAR_DATA *ch, char *argument )
{
    unsigned long length;
    char *formatted=argument;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    length = strlen(argument);

    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOMUSIC))
      {
        send_to_char("Music channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOMUSIC);
      }
      else
      {
        send_to_char("Music channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOMUSIC);
      }
    }
    else  /* music sent, turn music on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel privileges.\n\r",ch);
          return;
	}
     
        REMOVE_BIT(ch->comm,COMM_NOMUSIC);

    if(length+UMAX(strlen(IS_NPC(ch)?ch->short_descr:ch->name),7)+7 > 77)
    {
	send_to_char("You MUSIC:\n\r   '^NC",ch);
	formatted=format_string(ch,argument);
	send_to_char(formatted,ch);
	send_to_char("^NW'\n\r",ch);
    }
    else
    {
	send_to_char("You MUSIC: '^NC",ch);
	send_to_char(argument,ch);
	send_to_char("^NW'\n\r",ch);
    }
	   
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
           CHAR_DATA *victim;
 
           victim = d->original ? d->original : d->character;
 
           if ( d->connected == CON_PLAYING &&
                d->character != ch &&
                !IS_SET(victim->comm,COMM_NOMUSIC) &&
                !IS_SET(victim->comm,COMM_QUIET) )
           {
        if(length+UMAX(strlen(IS_NPC(ch)?ch->short_descr:ch->name),7)+7 > 77)
	    {
	      if(can_see(victim,ch))
                 sprintf(buf,"%s MUSIC:\n\r   '^NC%s^NW'\n\r",IS_NPC(ch)?ch->short_descr:ch->name,formatted);
	      else
                 sprintf(buf,"Someone MUSIC:\n\r   '^NC%s^NW'\n\r",formatted);
	      send_to_char(buf,victim);
           }
	   else
	   {
		if(can_see(victim,ch))
		    sprintf(buf,"%s MUSIC: '^NC%s^NW'\n\r",IS_NPC(ch)?ch->short_descr:ch->name,argument);
		else
		    sprintf(buf,"Someone MUSIC: '^NC%s^NW'\n\r",argument);
		send_to_char(buf,victim);
	    }
	}
    }
}
	if(formatted!=argument && formatted!=NULL)
	    free(formatted);
}	    

void do_auction( CHAR_DATA *ch, char *argument )
{
    unsigned long length;
    char *formatted=argument;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    length = strlen(argument);

    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOAUCTION))
      {
        send_to_char("Auction channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOAUCTION);
      }
      else
      {
        send_to_char("Auction channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOAUCTION);
      }
    }
    else  /* music sent, turn music on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel privileges.\n\r",ch);
          return;
	}
     
        REMOVE_BIT(ch->comm,COMM_NOAUCTION);

    if(length+UMAX(strlen(IS_NPC(ch)?ch->short_descr:ch->name),7)+7 > 77)
    {
	send_to_char("You auction:\n\r   '^NY",ch);
	formatted=format_string(ch,argument);
	send_to_char(formatted,ch);
	send_to_char("^NW'\n\r",ch);
    }
    else
    {
	send_to_char("You auction: '^NY",ch);
	send_to_char(argument,ch);
	send_to_char("^NW'\n\r",ch);
    }
	   
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
           CHAR_DATA *victim;
 
           victim = d->original ? d->original : d->character;
 
           if ( d->connected == CON_PLAYING &&
                d->character != ch &&
                !IS_SET(victim->comm,COMM_NOAUCTION) &&
                !IS_SET(victim->comm,COMM_QUIET) )
           {
        if(length+UMAX(strlen(IS_NPC(ch)?ch->short_descr:ch->name),7)+7 > 77)
	    {
	      if(can_see(victim,ch))
                 sprintf(buf,"%s auction:\n\r '^NY%s^NW'\n\r",IS_NPC(ch)?ch->short_descr:ch->name,formatted);
	      else
                 sprintf(buf,"Someone auction:\n\r '^NY%s^NW'\n\r",formatted);
	      send_to_char(buf,victim);
           }
	   else
	   {
		if(can_see(victim,ch))
		    sprintf(buf,"%s auction: '^NY%s^NW'\n\r",IS_NPC(ch)?ch->short_descr:ch->name,argument);
		else
		    sprintf(buf,"Someone auction: '^NY%s^NW'\n\r",argument);
		send_to_char(buf,victim);
	    }
	}
    }
}
	if(formatted!=argument && formatted!=NULL)
	    free(formatted);
}	    

void do_ooc( CHAR_DATA *ch, char *argument )
{
    unsigned long length;
    char *formatted=argument;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    length = strlen(argument);

    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOOOC))
      {
        send_to_char("OOC channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOOOC);
      }
      else
      {
        send_to_char("OOC channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOOOC);
      }
    }
    else  /* music sent, turn music on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel privileges.\n\r",ch);
          return;
	}
     
        REMOVE_BIT(ch->comm,COMM_NOOOC);

    if(length+UMAX(strlen(IS_NPC(ch)?ch->short_descr:ch->name),7)+7 > 77)
    {
	send_to_char("You OOC:\n\r   '^BG",ch);
	formatted=format_string(ch,argument);
	send_to_char(formatted,ch);
	send_to_char("^NW'\n\r",ch);
    }
    else
    {
	send_to_char("You OOC: '^BG",ch);
	send_to_char(argument,ch);
	send_to_char("^NW'\n\r",ch);
    }
	   
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
           CHAR_DATA *victim;
 
           victim = d->original ? d->original : d->character;
 
           if ( d->connected == CON_PLAYING &&
                d->character != ch &&
                !IS_SET(victim->comm,COMM_NOOOC) &&
                !IS_SET(victim->comm,COMM_QUIET) )
           {
        if(length+UMAX(strlen(IS_NPC(ch)?ch->short_descr:ch->name),7)+7 > 77)
	    {
	      if(can_see(victim,ch))
                 sprintf(buf,"%s OOC:\n\r   '^BG%s^NW'\n\r",IS_NPC(ch)?ch->short_descr:ch->name,formatted);
	      else
                 sprintf(buf,"Someone OOC:\n\r   '^BG%s^NW'\n\r",formatted);
	      send_to_char(buf,victim);
           }
	   else
	   {
		if(can_see(victim,ch))
		    sprintf(buf,"%s OOC: '^BG%s^NW'\n\r",IS_NPC(ch)?ch->short_descr:ch->name,argument);
		else
		    sprintf(buf,"Someone OOC: '^BG%s^NW'\n\r",argument);
		send_to_char(buf,victim);
	    }
	}
    }
}
	if(formatted!=argument && formatted!=NULL)
	    free(formatted);
}

void do_immtalk( CHAR_DATA *ch, char *argument )
{
    unsigned long length;
    char *formatted=argument;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    length = strlen(argument);

    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOWIZ))
      {
        send_to_char("Immortal channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOWIZ);
      }
      else
      {
        send_to_char("Immortal channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOWIZ);
      }
    }
    else  /* music sent, turn music on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel privileges.\n\r",ch);
          return;
	}
     
        REMOVE_BIT(ch->comm,COMM_NOWIZ);
/*
    if(length+UMAX(strlen(IS_NPC(ch)?ch->short_descr:ch->name),7)+7 > 77)
    {
	sprintf(buf,"%s:\n\r   '^BW",IS_NPC(ch)?ch->short_descr:ch->name);
	send_to_char(buf,ch);
	formatted=format_string(ch,argument);
	send_to_char(formatted,ch);
	send_to_char("^NW'\n\r",ch);
    }
    else
    {
	sprintf(buf,"%s: '^BW",IS_NPC(ch)?ch->short_descr:ch->name);
	send_to_char(buf,ch);
	send_to_char(argument,ch);
	send_to_char("^NW'\n\r",ch);
    }*/
	   
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
           CHAR_DATA *victim;
 
           victim = d->original ? d->original : d->character;

	   if ( d->connected == CON_PLAYING && 
	        IS_HERO(d->character) && 
                !IS_SET(d->character->comm,COMM_NOWIZ) )
           {
        if(length+UMAX(strlen(IS_NPC(ch)?ch->short_descr:ch->name),7)+7 > 77)
	    {
	      if(can_see(victim,ch))
                 sprintf(buf,"%s:\n\r '^BW%s^NW'\n\r",IS_NPC(ch)?ch->short_descr:ch->name,formatted);
	      else
                 sprintf(buf,"Someone:\n\r   '^BW%s^NW'\n\r",formatted);
	      send_to_char(buf,victim);
           }
	   else
	   {
		if(can_see(victim,ch))
		    sprintf(buf,"%s: '^BW%s^NW'\n\r",IS_NPC(ch)?ch->short_descr:ch->name,argument);
		else
		    sprintf(buf,"Someone: '^BW%s^NW'\n\r",argument);
		send_to_char(buf,victim);
	    }
	}
    }
}
	if(formatted!=argument && formatted!=NULL)
	    free(formatted);
}	    
	   
void do_imptalk( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOIMPT))
      {
	send_to_char("Imp channel is now ON\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOIMPT);
      }
      else
      {
	send_to_char("Imp channel is now OFF\n\r",ch);
	SET_BIT(ch->comm,COMM_NOIMPT);
      } 
      return;
    }

    REMOVE_BIT(ch->comm,COMM_NOIMPT);

    act_new("$n IMP] $t",ch,argument,NULL,TO_CHAR,POS_DEAD);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING && 
	     IS_IMP(d->character) && 
             !IS_SET(d->character->comm,COMM_NOIMPT) )
	{
	    act_new("$n IMP] $t",ch,argument,d->character,TO_VICT,POS_DEAD);
	}
    }

    return;
}

void do_say( CHAR_DATA *ch, char *argument )
{
    unsigned long length;
    char *formatted=NULL;
    CHAR_DATA *victim;
    char buf[MAX_INPUT_LENGTH];

    length=strlen(argument);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Say what?\n\r", ch );
	return;
    }

    if (ch->position<POS_RESTING)
    {
	send_to_char( "You can't speak!\n\r",ch);
	return;
    }

    if(length+UMAX(strlen(IS_NPC(ch)?ch->short_descr:ch->name),7)+7 > 77)
    {
    	formatted=format_string(ch,argument);
	sprintf(buf,"You say\n\r   '^BP%s^NW'\n\r",formatted);
	send_to_char(buf,ch);
        sprintf(buf,"%s says,\n\r   '^BP%s^NW'\n\r",IS_NPC(ch)?ch->short_descr:ch->name,formatted);
    }
    else
    {
	sprintf(buf,"You say '^BP%s^NW'\n\r",argument);
	send_to_char(buf,ch);
        sprintf(buf,"%s says '^BP%s^NW'\n\r",IS_NPC(ch)?ch->short_descr:ch->name,argument);
    }

    for(victim=ch->in_room->people;victim!=NULL;victim=victim->next_in_room)
    {
        if((ch==victim) || (victim->position<POS_RESTING)) continue;
	send_to_char(buf,victim);
    }

    if(formatted!=argument && formatted!=NULL)
	free(formatted);
}



void do_shout( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
	send_to_char( "You can't shout.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_DEAF))
    {
	send_to_char( "Deaf people can't shout.\n\r",ch);
        return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Shout what?\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 12 );

    act( "You shout '^NR$T^nw'", ch, NULL, argument, TO_CHAR );
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm, COMM_DEAF) &&
	     !IS_SET(victim->comm, COMM_QUIET) ) 
	{
	    act("$n shouts '^NR$t^nw'",ch,argument,d->character,TO_VICT);
	}
    }

    return;
}



void do_tell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_QUIET) )
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch);
	return;
    }



    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Tell whom what?\n\r", ch );
	return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    || ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_AFK) )
    {
	send_to_char("They are currently AFK. Please try again later.\n\r",ch);
	return;
    }


    if ( victim->desc == NULL && !IS_NPC(victim))
    {
	act("$N seems to have misplaced $S link...try again later.",
	    ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }
  
    if ( IS_SET(victim->comm,COMM_QUIET) && !IS_IMMORTAL(ch))
    {
	act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
  	return;
    }

    act( "You tell $N '^BY$t^nw'", ch, argument, victim, TO_CHAR );
    act_new("$n tells you '^BY$t^nw'",ch,argument,victim,TO_VICT,POS_DEAD);
    victim->reply	= ch;

    return;
}



void do_reply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( ( victim = ch->reply ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
        act("$N seems to have misplaced $S link...try again later.",
            ch,NULL,victim,TO_CHAR);
        return;
    }

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    if ( IS_SET(victim->comm,COMM_QUIET) && !IS_IMMORTAL(ch))
    {
        act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
        return;
    }

    act("You tell $N '^BY$t^nw'",ch,argument,victim,TO_CHAR);
    act_new("$n tells you '^BY$t^nw'",ch,argument,victim,TO_VICT,POS_DEAD);
    victim->reply	= ch;

    return;
}



void do_yell( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
        send_to_char( "You can't yell.\n\r", ch );
        return;
    }
 
    if ( argument[0] == '\0' )
    {
	send_to_char( "Yell what?\n\r", ch );
	return;
    }


    act("You yell '^NR$t^nw'",ch,argument,NULL,TO_CHAR);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character != ch
	&&   d->character->in_room != NULL
	&&   d->character->in_room->area == ch->in_room->area 
        &&   !IS_SET(d->character->comm,COMM_QUIET) )
	{
	    act("$n yells '^NR$t^nw'",ch,argument,d->character,TO_VICT);
	}
    }

    return;
}



void do_emote( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
	send_to_char( "You can't show your emotions.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Emote what?\n\r", ch );
	return;
    }

    act( "$n $T", ch, NULL, argument, TO_ROOM );
    act( "$n $T", ch, NULL, argument, TO_CHAR );
    return;
}



/*
 * All the posing stuff.
 */
struct	pose_table_type
{
    char *	message[2*MAX_CLASS];
};

const	struct	pose_table_type	pose_table	[]	=
{
    {
	{
	    "You sizzle with energy.",
	    "$n sizzles with energy.",
	    "You feel very holy.",
	    "$n looks very holy.",
	    "You perform a small card trick.",
	    "$n performs a small card trick.",
	    "You show your bulging muscles.",
	    "$n shows $s bulging muscles."
	}
    },

    {
	{
	    "You turn into a butterfly, then return to your normal shape.",
	    "$n turns into a butterfly, then returns to $s normal shape.",
	    "You nonchalantly turn wine into water.",
	    "$n nonchalantly turns wine into water.",
	    "You wiggle your ears alternately.",
	    "$n wiggles $s ears alternately.",
	    "You crack nuts between your fingers.",
	    "$n cracks nuts between $s fingers."
	}
    },

    {
	{
	    "Blue sparks fly from your fingers.",
	    "Blue sparks fly from $n's fingers.",
	    "A halo appears over your head.",
	    "A halo appears over $n's head.",
	    "You nimbly tie yourself into a knot.",
	    "$n nimbly ties $mself into a knot.",
	    "You grizzle your teeth and look mean.",
	    "$n grizzles $s teeth and looks mean."
	}
    },

    {
	{
	    "Little red lights dance in your eyes.",
	    "Little red lights dance in $n's eyes.",
	    "You recite words of wisdom.",
	    "$n recites words of wisdom.",
	    "You juggle with daggers, apples, and eyeballs.",
	    "$n juggles with daggers, apples, and eyeballs.",
	    "You hit your head, and your eyes roll.",
	    "$n hits $s head, and $s eyes roll."
	}
    },

    {
	{
	    "A slimy green monster appears before you and bows.",
	    "A slimy green monster appears before $n and bows.",
	    "Deep in prayer, you levitate.",
	    "Deep in prayer, $n levitates.",
	    "You steal the underwear off every person in the room.",
	    "Your underwear is gone!  $n stole it!",
	    "Crunch, crunch -- you munch a bottle.",
	    "Crunch, crunch -- $n munches a bottle."
	}
    },

    {
	{
	    "You turn everybody into a little pink elephant.",
	    "You are turned into a little pink elephant by $n.",
	    "An angel consults you.",
	    "An angel consults $n.",
	    "The dice roll ... and you win again.",
	    "The dice roll ... and $n wins again.",
	    "... 98, 99, 100 ... you do pushups.",
	    "... 98, 99, 100 ... $n does pushups."
	}
    },

    {
	{
	    "A small ball of light dances on your fingertips.",
	    "A small ball of light dances on $n's fingertips.",
	    "Your body glows with an unearthly light.",
	    "$n's body glows with an unearthly light.",
	    "You count the money in everyone's pockets.",
	    "Check your money, $n is counting it.",
	    "Arnold Schwarzenegger admires your physique.",
	    "Arnold Schwarzenegger admires $n's physique."
	}
    },

    {
	{
	    "Smoke and fumes leak from your nostrils.",
	    "Smoke and fumes leak from $n's nostrils.",
	    "A spot light hits you.",
	    "A spot light hits $n.",
	    "You balance a pocket knife on your tongue.",
	    "$n balances a pocket knife on your tongue.",
	    "Watch your feet, you are juggling granite boulders.",
	    "Watch your feet, $n is juggling granite boulders."
	}
    },

    {
	{
	    "The light flickers as you rap in magical languages.",
	    "The light flickers as $n raps in magical languages.",
	    "Everyone levitates as you pray.",
	    "You levitate as $n prays.",
	    "You produce a coin from everyone's ear.",
	    "$n produces a coin from your ear.",
	    "Oomph!  You squeeze water out of a granite boulder.",
	    "Oomph!  $n squeezes water out of a granite boulder."
	}
    },

    {
	{
	    "Your head disappears.",
	    "$n's head disappears.",
	    "A cool breeze refreshes you.",
	    "A cool breeze refreshes $n.",
	    "You step behind your shadow.",
	    "$n steps behind $s shadow.",
	    "You pick your teeth with a spear.",
	    "$n picks $s teeth with a spear."
	}
    },

    {
	{
	    "A fire elemental singes your hair.",
	    "A fire elemental singes $n's hair.",
	    "The sun pierces through the clouds to illuminate you.",
	    "The sun pierces through the clouds to illuminate $n.",
	    "Your eyes dance with greed.",
	    "$n's eyes dance with greed.",
	    "Everyone is swept off their foot by your hug.",
	    "You are swept off your feet by $n's hug."
	}
    },

    {
	{
	    "The sky changes color to match your eyes.",
	    "The sky changes color to match $n's eyes.",
	    "The ocean parts before you.",
	    "The ocean parts before $n.",
	    "You deftly steal everyone's weapon.",
	    "$n deftly steals your weapon.",
	    "Your karate chop splits a tree.",
	    "$n's karate chop splits a tree."
	}
    },

    {
	{
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "A thunder cloud kneels to you.",
	    "A thunder cloud kneels to $n.",
	    "The Grey Mouser buys you a beer.",
	    "The Grey Mouser buys $n a beer.",
	    "A strap of your armor breaks over your mighty thews.",
	    "A strap of $n's armor breaks over $s mighty thews."
	}
    },

    {
	{
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "The Burning Man speaks to you.",
	    "The Burning Man speaks to $n.",
	    "Everyone's pocket explodes with your fireworks.",
	    "Your pocket explodes with $n's fireworks.",
	    "A boulder cracks at your frown.",
	    "A boulder cracks at $n's frown."
	}
    },

    {
	{
	    "Everyone's clothes are transparent, and you are laughing.",
	    "Your clothes are transparent, and $n is laughing.",
	    "An eye in a pyramid winks at you.",
	    "An eye in a pyramid winks at $n.",
	    "Everyone discovers your dagger a centimeter from their eye.",
	    "You discover $n's dagger a centimeter from your eye.",
	    "Mercenaries arrive to do your bidding.",
	    "Mercenaries arrive to do $n's bidding."
	}
    },

    {
	{
	    "A black hole swallows you.",
	    "A black hole swallows $n.",
	    "Valentine Michael Smith offers you a glass of water.",
	    "Valentine Michael Smith offers $n a glass of water.",
	    "Where did you go?",
	    "Where did $n go?",
	    "Four matched Percherons bring in your chariot.",
	    "Four matched Percherons bring in $n's chariot."
	}
    },

    {
	{
	    "The world shimmers in time with your whistling.",
	    "The world shimmers in time with $n's whistling.",
	    "The great god Tybalt gives you a staff.",
	    "The great god Tybalt gives $n a staff.",
	    "Click.",
	    "Click.",
	    "Atlas asks you to relieve him.",
	    "Atlas asks $n to relieve him."
	}
    }
};



void do_pose( CHAR_DATA *ch, char *argument )
{
    int level;
    int pose;

    if ( IS_NPC(ch) )
	return;

    level = UMIN( ch->level, sizeof(pose_table) / sizeof(pose_table[0]) - 1 );
    pose  = number_range(0, level);

    act( pose_table[pose].message[2*ch->class+0], ch, NULL, NULL, TO_CHAR );
    act( pose_table[pose].message[2*ch->class+1], ch, NULL, NULL, TO_ROOM );

    return;
}



void do_bug( CHAR_DATA *ch, char *argument )
{
    append_file( ch, BUG_FILE, argument );
    send_to_char( "Bug logged.\n\r", ch );
    return;
}



void do_idea( CHAR_DATA *ch, char *argument )
{
    append_file( ch, IDEA_FILE, argument );
    send_to_char( "Idea logged. This is NOT an identify command.\n\r", ch );
    return;
}



void do_typo( CHAR_DATA *ch, char *argument )
{
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "Typo logged.\n\r", ch );
    return;
}



void do_rent( CHAR_DATA *ch, char *argument )
{
    send_to_char( "There is no rent here.  Just save and quit.\n\r", ch );
    return;
}



void do_qui( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}



void do_quit( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

    if ( time(NULL) - ch->pk_timer < 90 && get_trust(ch) < 91)
    {
	sprintf(buf,"You are not safe yet! You need to wait %d more seconds.\n\r",90 - (int)(time(NULL) - ch->pk_timer));
	send_to_char(buf,ch);
	return;
    }
    
    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "No way! You are fighting.\n\r", ch );
	return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
	send_to_char( "You're not DEAD yet.\n\r", ch );
	return;
    }
    quit(ch);
}

void quit(CHAR_DATA *ch)
{
    DESCRIPTOR_DATA *d;

    if(IS_SET(ch->act, PLR_QUESTOR))
    {
	send_to_char("You quit in the middle of the quest. Your remaining time will be added to your wait.\n\r",ch);
        REMOVE_BIT(ch->act, PLR_QUESTOR);
        ch->questgiver = NULL;
        ch->questmob = 0;
        ch->questobj = 0;
        ch->nextquest = 30+ch->countdown;
        ch->countdown = 0;
    }

    REMOVE_BIT(ch->act,PLR_IMMQUEST);

    send_to_char( 
	"Alas, all good things must come to an end.\n\r",ch);
    act( "$n has left the game.", ch, NULL, NULL, TO_ROOM );
    sprintf( log_buf, "%s has quit.", ch->name );
    log_string( log_buf );
    wiznet("$N rejoins the real world.",ch,NULL,WIZ_LOGINS,0,get_trust(ch));
    if(IS_SET(ch->comm,COMM_AFK)) REMOVE_BIT(ch->comm,COMM_AFK);
    /*
     * After extract_char the ch is no longer valid!
     */

    if( ch->pcdata->locker!=NULL )
	do_storage(ch, "Close");

    save_char_obj( ch );
    d = ch->desc;

    reset_vt100(ch);

 	/* Free note that might be there somehow */
	if (ch->pcdata->in_progress)
	    free_note (ch->pcdata->in_progress);

    extract_char( ch, TRUE );
    if ( d != NULL )
	close_socket( d );

    return;
}


void do_save( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    save_char_obj( ch );
    send_to_char("Saving. Remember that ROM has automatic saving now\n\r", ch );
    WAIT_STATE(ch,5 * PULSE_VIOLENCE);
    return;
}



void do_follow( CHAR_DATA *ch, char *argument )
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Follow whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
    {
	act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	if ( ch->master == NULL )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return;
	}
	stop_follower(ch);
	return;
    }

    if (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOFOLLOW) && !IS_HERO(ch))
    {
	act("$N doesn't seem to want any followers.\n\r",
             ch,NULL,victim, TO_CHAR);
        return;
    }

    REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    
    if ( ch->master != NULL )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
}


void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master != NULL )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
	act( "$n now follows you.", ch, NULL, master, TO_VICT );

    act( "You now follow $N.",  ch, NULL, master, TO_CHAR );

    return;
}



void stop_follower( CHAR_DATA *ch )
{
    if ( ch->master == NULL )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	REMOVE_BIT( ch->affected_by, AFF_CHARM );
	affect_strip( ch, gsn_charm_person );
    }

    if ( can_see( ch->master, ch ) && ch->in_room != NULL)
    {
	act( "$n stops following you.",     ch, NULL, ch->master, TO_VICT    );
    	act( "You stop following $N.",      ch, NULL, ch->master, TO_CHAR    );
    }
    if (ch->master->pet == ch)
	ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;
    return;
}

/* nukes charmed monsters and pets */
void nuke_pets( CHAR_DATA *ch )
{    
    CHAR_DATA *pet;

    if ((pet = ch->pet) != NULL)
    {
    	stop_follower(pet);
    	if (pet->in_room != NULL)
    	    act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
    	extract_char(pet,TRUE);
    }
    ch->pet = NULL;

    return;
}



void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;

    if ( ch->master != NULL )
    {
    	if (ch->master->pet == ch)
    	    ch->master->pet = NULL;
	stop_follower( ch );
    }

    ch->leader = NULL;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = fch;
    }

    return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    argument = one_argument( argument, arg );
    one_argument(argument,arg2);

    if (!str_cmp(arg2,"delete"))
    {
        send_to_char("That will NOT be done.\n\r",ch);
        return;
    }

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Order whom to do what?\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( !IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}
    }

    found = FALSE;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
	och_next = och->next_in_room;

	if ( IS_AFFECTED(och, AFF_CHARM)
	&&   och->master == ch
	&& ( fAll || och == victim ) )
	{
            if( !str_cmp(argument, "rem all") || 
		!str_cmp(argument, "remo all") || 
		!str_cmp(argument, "remov all") || 
		!str_cmp(argument, "remove all") )
	    {
		send_to_char("Nope, you can't order them to do that.\n\r",ch);
		return;
	    }
		
	    found = TRUE;
	    sprintf( buf, "$n orders you to '%s'.", argument );
	    act( buf, ch, NULL, och, TO_VICT );
	    interpret( och, argument );
	}
    }

    if ( found )
	send_to_char( "Ok.\n\r", ch );
    else
	send_to_char( "You have no followers here.\n\r", ch );
    return;
}



void do_group( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = (ch->leader != NULL) ? ch->leader : ch;
	sprintf( buf, "%s's group:\n\r", PERS(leader, ch) );
	send_to_char( buf, ch );

	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	    if ( is_same_group( gch, ch ) )
	    {
		sprintf( buf,
		"[%2d %s] %-16s %4d/%4d hp %4d/%4d mana %4d/%4d mv %5ld xp\n\r",
		    gch->level,
		    IS_NPC(gch) ? "Mob" : class_table[gch->class].who_name,
		    capitalize( PERS(gch, ch) ),
		    gch->hit,   gch->max_hit,
		    gch->mana,  gch->max_mana,
		    gch->move,  gch->max_move,
		    gch->exp    );
		send_to_char( buf, ch );
	    }
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
    {
	send_to_char( "But you are following someone else!\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
	act( "$N isn't following you.", ch, NULL, victim, TO_CHAR );
	return;
    }
    
    if (IS_AFFECTED(victim,AFF_CHARM))
    {
        send_to_char("You can't remove charmed mobs from your group.\n\r",ch);
        return;
    }
    
    if (IS_AFFECTED(ch,AFF_CHARM))
    {
    	act("You like your master too much to leave $m!",ch,NULL,victim,TO_VICT);
    	return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
	victim->leader = NULL;
	act( "$n removes $N from $s group.",   ch, NULL, victim, TO_NOTVICT );
	act( "$n removes you from $s group.",  ch, NULL, victim, TO_VICT    );
	act( "You remove $N from your group.", ch, NULL, victim, TO_CHAR    );
	return;
    }
/*
    if ( ch->level - victim->level < -8
    ||   ch->level - victim->level >  8 )
    {
	act( "$N cannot join $n's group.",     ch, NULL, victim, TO_NOTVICT );
	act( "You cannot join $n's group.",    ch, NULL, victim, TO_VICT    );
	act( "$N cannot join your group.",     ch, NULL, victim, TO_CHAR    );
	return;
    }
*/

    victim->leader = ch;
    act( "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
    act( "You join $n's group.", ch, NULL, victim, TO_VICT    );
    act( "$N joins your group.", ch, NULL, victim, TO_CHAR    );
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount;
    int share;
    int extra;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Split how much?\n\r", ch );
	return;
    }
    
    amount = atoi( arg );

    if ( amount < 0 )
    {
	send_to_char( "Your group wouldn't like that.\n\r", ch );
	return;
    }

    if ( amount == 0 )
    {
	send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
	return;
    }

    if ( ch->gold < amount )
    {
	send_to_char( "You don't have that much gold.\n\r", ch );
	return;
    }
  
    members = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
	    members++;
    }

    if ( members < 2 )
    {
	send_to_char( "Just keep it all.\n\r", ch );
	return;
    }
	    
    share = amount / members;
    extra = amount % members;

    if ( share == 0 )
    {
	send_to_char( "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    ch->gold -= amount;
    ch->gold += share + extra;

    sprintf( buf,
	"You split %d gold coins.  Your share is %d gold coins.\n\r",
	amount, share + extra );
    send_to_char( buf, ch );

    sprintf( buf, "$n splits %d gold coins.  Your share is %d gold coins.",
	amount, share );

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
	{
	    act( buf, ch, NULL, gch, TO_VICT );
	    gch->gold += share;
	}
    }

    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Tell your group what?\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
	send_to_char( "Your message didn't get through!\n\r", ch );
	return;
    }

    /*
     * Note use of send_to_char, so gtell works on sleepers.
     */
    sprintf( buf, "%s tells the group '%s'.\n\r", ch->name, argument );
    for ( gch = char_list; gch != NULL; gch = gch->next )
    {
	if ( is_same_group( gch, ch ) )
	    send_to_char( buf, gch );
    }

    return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach->leader != NULL ) ach = ach->leader;
    if ( bch->leader != NULL ) bch = bch->leader;
    return ach == bch;
}
