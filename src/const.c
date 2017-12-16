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
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "interp.h"

const 	struct guild_type	guild_table	[]		=
{
    {   "None",		"",		   0,   0  },
    {	"Coder",	"[^BGCODER^NW]",   0,   0  },
    {   "Admin",        "[^BRADMIN^NW]",   0,   0  },
    {	"Builder",	"[^BCBUILDER^NW]", 0,	0  },
    {   "Quest",	"[^NPQUEST^NW]",   0,   0  },
    {   "Manifest",     "[^NBM^BBAN^BWIF^BBES^NBT^NW]", 0, 52 },
    {   "Outcast",      "[^BBOUTCAST^NW]", 0,	54  },
    {   "Bloodsworn",   "[^BRBLOODSWORN^NW]",0, 53  },
    {	NULL,		NULL,		   0,	0  }
};

const   struct  weapon_type     weapon_table    []      =
{
   { "sword",   OBJ_VNUM_SCHOOL_SWORD,  WEAPON_SWORD,   &gsn_sword      },
   { "mace",    OBJ_VNUM_SCHOOL_MACE,   WEAPON_MACE,    &gsn_mace       },
   { "dagger",  OBJ_VNUM_SCHOOL_DAGGER, WEAPON_DAGGER,  &gsn_dagger     },
   { "axe",     0,    WEAPON_AXE,     &gsn_axe        },
   { "staff",   0,  WEAPON_SPEAR,   &gsn_spear      },
   { "flail",   0,  WEAPON_FLAIL,   &gsn_flail      },
   { "whip",    0,   WEAPON_WHIP,    &gsn_whip       },
   { "polearm", 0   ,WEAPON_POLEARM, &gsn_polearm    },
   { NULL,      0,                              0,      NULL            }
};

/* for sex */
const struct sex_type sex_table[] =
{
   {    "none"          },
   {    "male"          },
   {    "female"        },
   {    "either"        },
   {    NULL            }
};

/* for position */
const struct position_type position_table[] =
{
    {   "dead",                 "dead"  },
    {   "mortally wounded",     "mort"  },
    {   "incapacitated",        "incap" },
    {   "stunned",              "stun"  },
    {   "sleeping",             "sleep" },
    {   "resting",              "rest"  },
    {   "sitting",              "sit"   },
    {   "fighting",             "fight" },
    {   "standing",             "stand" },
    {   NULL,                   NULL    }
};

/* for sizes */
const struct size_type size_table[] =
{
    {   "tiny"          },
    {   "small"         },
    {   "medium"        },
    {   "large"         },
    {   "huge",         },
    {   "giant"         },
    {   NULL            }
};


/*Item lookup list*/
const struct item_lookup_type item_table      []      =
{
    {   ITEM_LIGHT,     "light"         },
    {   ITEM_SCROLL,    "scroll"        },
    {   ITEM_WAND,      "wand"          },
    {   ITEM_STAFF,     "staff"         },
    {   ITEM_WEAPON,    "weapon"        },
    {   ITEM_TREASURE,  "treasure"      },
    {   ITEM_ARMOR,     "armor"         },
    {   ITEM_POTION,    "potion"        },
    {   ITEM_CLOTHING,  "clothing"      },
    {   ITEM_FURNITURE, "furniture"     },
    {   ITEM_TRASH,     "trash"         },
    {   ITEM_CONTAINER, "container"     },
    {   ITEM_DRINK_CON, "drink"         },
    {   ITEM_KEY,       "key"           },
    {   ITEM_FOOD,      "food"          },
    {   ITEM_MONEY,     "money"         },
    {   ITEM_BOAT,      "boat"          },
    {   ITEM_CORPSE_NPC,"npc_corpse"    },
    {   ITEM_CORPSE_PC, "pc_corpse"     },
    {   ITEM_FOUNTAIN,  "fountain"      },
    {   ITEM_PILL,      "pill"          },
    {   ITEM_PROTECT,   "protect"       },
    {   ITEM_MAP,       "map"           },
    {   ITEM_PORTAL,    "portal"        },
    {	ITEM_TRAP,	"trap"		},
    {   0,              NULL            }
};


const   struct disguise_type	disguise_table  []		=
{
    {"a fish",    "A fish flops around, gasping for breath."},
    {"a bunny",	"A six foot pink bunny chews on your sneaker." },
    {"a fly",     "A small fly tries to find some stool to land on."},
    {"a cockroach","An ugly black cockroach hisses and wiggles his antennae."},
    {"a rabbit",	"A six foot rabid white rabbit dribbles blood from its mouth." }, 
    {"a kitten",  "A huge kitten plays around with invisible prey."},
    {"a snake",   "A snake slithers around, flickering his tongue in and out of his mouth."},
    {"a mouse",   "A small grey mouse squeaks and wiggles his whiskers."},
    {"a pigeon",  "A little grey pigeon coos and bobs his head as he walks."},
    {"a spider",  "A small, black spider tries to spin a web."},
    {"a squirrel","A grey, flying squirrel can't quite get up in the air."},
    {"a cat",     "An ugly alley cat hobbles around on three legs."},
    {"a chicken", "A chicken clucks and ruffles her feathers."},
    {"a puppy",   "A large, playful puppy drools all over the ground."},
    {"a dog",	"A large dog with floppy ears stares hungrily at your shoe." },
    {"a duck",    "A rather large duck quacks and waddles around."},
    {"a wolf",    "A puny wolf licks his rear." },
    {"a pig",     "A pink pig munches on some slop." },
    {"a horse",   "A large sturdy horse flips you with his tail." },
    {"a frog",    "A little green frog croaks and gives a large hop."},
    {"a hog",     "A huge, ugly hog scruffles around in the mud." },
    {"a cow",     "A large cow dribbles milk from her udder." },
    {"a fox",     "A quick brown fox looks for a lazy dog to jump over."},
    {"a deer",    "A flighty deer blinks with adoring doe eyes." },
    {"a baby",    "A cute baby in a basket googoos and gagas happily."},
    {"an elk",     "A large, hairy elk chews on some grass."},
    {"a moose",   "A large moose pulls a rabbit out of his hat." },
    {"a hippo",   "A miniature hippo yawns lazily and stretches."},
    {"a bear",    "A large brown bear growls and shows his large white teeth."},
    {"an elephant","A wrinkly, grey elephant sprays water on himself with his trunk."},
    {"a child",   "A small child sucks on his thumb and whimpers pitifully."},
    {"a girl",    "A little girl with pigtails scuffs her toe in the ground."},
    {"a boy",     "A boy with freckles runs around anxiously."},
    {"a man",     "A man with strange glasses and a moustache looks around suspiciously."},
    {"a jester",  "A court jester hops around crazily."},
    {"a man",     "An ordinary man looks rather ordinary."},
    {"a queen",   "A beautiful drag queen looks just too good to be true."},
    {"a woman",   "A haggard old woman hobbles around on her cane."},
    {"a lady",    "A fine, fat lady breaks a heel under her weight."},
    {"a woman",   "A tall, sexy woman tugs on her miniskirt."},
    {"a princess","A beautiful princess glances around shyly."},
    {"a citizen", "A normal citizen carries on with his normal job."},
    {"a crier",   "A town crier takes a break from his job."},
    {"a baker",   "A baker puffs up his white hat and shakes some flour from his hands."},
    {"a mayor",   "The mayor of the town wants everyone to vote for him."},
    {"a blacksmith","A blacksmith wipes some coal dust from his face."},
    {"a bodybuilder","A bodybuilder flexes his muscles and flips his hair."},
    {"an apple"    "A sweet, juicy apple rests tastefully on the ground."},
    {"a mad scientist","A mad scientist cackles evilly and fluffs his wacky hair."},
    {"a nerdy boy","A nerdy boy hitches up his highwater pants."},
    {"a banana",  "A large yellow banana lies still on the ground."},
    {"a cityguard","A cityguard keeps an eye on things."},
    {"a thief",   "A generic thief glances around mysteriously."},
    {"a wrestler","A professional wrestler glares and screams angrily."},
    {"a prostitute","A prostitute winks at people, looking for a trick."},
    {"a pear"     "A large, pear-shaped pear rocks around on the ground."},
    {"a stripper","A sexy stripper starts taking off her clothes."},
    {"a salesman","An obnoxious salesman tries to sell things to everyone who passes."},
    {"a bride",   "A beautiful bride blushes under her veil."},
    {"a lawyer",  "A lawyer in a nice suit keeps an eye out for rabid fidoes."},
    {"a unicorn", "A white unicorn flashes light magically from his horn."},
    {"a pegasus", "A black pegasus flaps his wings and flares his nostrils."},
    {"a goblin",  "A little green goblin drools and snickers to himself."},
    {"a wyvern",  "A weird looking wyvern twists and turns his neck."},
    {"a gremlin", "An ugly green goblin drinks a glass full of water."},
    {"a vampire", "A pale vampire in dark clothes peers around evilly."},
    {"a spirit",  "A pale, transparent spirit whispers sweet nothings into the air."},
    {"a ghoul",   "An evil-looking ghoul drools green slime from the side of his mouth."},
    {"a dragon",  "A huge, green dragon blows smoke from his maw."},
    {"a griffin", "A large, brown griffin ruffles his wings and squawks."},
    {"Tybalt",  "Tybalt, the IMP of Rivers of Blood looks awesome in his power."},
    {"cookie monster",  "A large blue monster screams \"COOKIE! COOKIE!\""},
    {"",""}   
    };


const   struct attack_type      attack_table    []    =
{
    {   "none",         "hit",          -1              },  /*  0 */
    {   "slice",        "slice",        DAM_SLASH       },
    {   "stab",         "stab",         DAM_PIERCE      },
    {   "slash",        "slash",        DAM_SLASH       },
    {   "whip",         "whip",         DAM_SLASH       },
    {   "claw",         "claw",         DAM_SLASH       },  /*  5 */
    {   "blast",        "blast",        DAM_BASH        },
    {   "pound",        "pound",        DAM_BASH        },
    {   "crush",        "crush",        DAM_BASH        },
    {   "grep",         "grep",         DAM_SLASH       },
    {   "bite",         "bite",         DAM_PIERCE      },  /* 10 */
    {   "pierce",       "pierce",       DAM_PIERCE      },
    {   "suction",      "suction",      DAM_BASH        },
    {   "beating",      "beating",      DAM_BASH        },
    {   "digestion",    "digestion",    DAM_ACID        },
    {   "charge",       "charge",       DAM_BASH        },  /* 15 */
    {   "slap",         "slap",         DAM_BASH        },
    {   "punch",        "punch",        DAM_BASH        },
    {   "wrath",        "wrath",        DAM_ENERGY      },
    {   "magic",        "magic",        DAM_ENERGY      },
    {   "divine",       "divine power", DAM_HOLY        },  /* 20 */
    {   "cleave",       "cleave",       DAM_SLASH       },
    {   "scratch",      "scratch",      DAM_PIERCE      },
    {   "peck",         "peck",         DAM_PIERCE      },
    {   "peckb",        "peck",         DAM_BASH        },
    {   "chop",         "chop",         DAM_SLASH       },  /* 25 */
    {   "sting",        "sting",        DAM_PIERCE      },
    {   "smash",         "smash",       DAM_BASH        },
    {   "shbite",       "shocking bite",DAM_LIGHTNING   },
    {   "flbite",       "flaming bite", DAM_FIRE        },
    {   "frbite",       "freezing bite", DAM_COLD       },  /* 30 */
    {   "acbite",       "acidic bite",  DAM_ACID        },
    {   "chomp",        "chomp",        DAM_PIERCE      },
    {   "drain",        "life drain",   DAM_NEGATIVE    },
    {   "thrust",       "thrust",       DAM_PIERCE      },
    {   "slime",        "slime",        DAM_ACID        },
    {   "shock",        "shock",        DAM_LIGHTNING   },
    {   "thwack",       "thwack",       DAM_BASH        },
    {   "flame",        "flame",        DAM_FIRE        },
    {   "chill",        "chill",        DAM_COLD        },
    {   NULL,           NULL,           0               }
};



/* attack table  -- not very organized :(
const 	struct attack_type	attack_table	[]		=
{
    { 	"hit",		-1		}, 
    {	"slice", 	DAM_SLASH	},	
    {   "stab",		DAM_PIERCE	},
    {	"slash",	DAM_SLASH	},
    {	"whip",		DAM_SLASH	},
    {   "claw",		DAM_SLASH	}, 
    {	"blast",	DAM_BASH	},
    {   "pound",	DAM_BASH	},
    {	"crush",	DAM_BASH	},
    {   "grep",		DAM_SLASH	},
    {	"bite",		DAM_PIERCE	},
    {   "pierce",	DAM_PIERCE	},
    {   "suction",	DAM_BASH	},
    {	"beating",	DAM_BASH	},
    {   "digestion",	DAM_ACID	},
    {	"charge",	DAM_BASH	},
    { 	"slap",		DAM_BASH	},
    {	"punch",	DAM_BASH	},
    {	"wrath",	DAM_ENERGY	},
    {	"magic",	DAM_ENERGY	},
    {   "divine power",	DAM_HOLY	},
    {	"cleave",	DAM_SLASH	},
    {	"scratch",	DAM_PIERCE	},
    {   "peck",		DAM_PIERCE	},
    {   "peck",		DAM_BASH	},
    {   "chop",		DAM_SLASH	},
    {   "sting",	DAM_PIERCE	},
    {   "smash",	DAM_BASH	},
    {   "shocking bite",DAM_LIGHTNING	},
    {	"flaming bite", DAM_FIRE	},
    {	"freezing bite", DAM_COLD	},
    {	"acidic bite", 	DAM_ACID	},
    {	"chomp",	DAM_PIERCE	}
};
*/

/* wiznet table and prototype for future flag setting */
const   struct wiznet_type      wiznet_table    []              =
{
   {    "on",           WIZ_ON,         IM },
   {    "prefix",	WIZ_PREFIX,	IM },
   {    "ticks",        WIZ_TICKS,      IM },
   {    "logins",       WIZ_LOGINS,     IM },
   {    "sites",        WIZ_SITES,      L3 },
   {    "links",        WIZ_LINKS,      L7 },
   {	"newbies",	WIZ_NEWBIE,	IM },
   {	"spam",		WIZ_SPAM,	L5 },
   {    "deaths",       WIZ_DEATHS,     IM },
   {    "resets",       WIZ_RESETS,     L4 },
   {    "mobdeaths",    WIZ_MOBDEATHS,  L4 },
   {    "attacks",      WIZ_ATTACKS,	L5 },
   {    "flags",        WIZ_FLAGS,	L5 },
   {	"penalties",	WIZ_PENALTIES,	L5 },
   {	"saccing",	WIZ_SACCING,	L5 },
   {	"levels",	WIZ_LEVELS,	IM },
   {	"load",		WIZ_LOAD,	L2 },
   {	"restore",	WIZ_RESTORE,	L2 },
   {	"snoops",	WIZ_SNOOPS,	L2 },
   {	"switches",	WIZ_SWITCHES,	L2 },
   {	"secure",	WIZ_SECURE,	L1 },
   {	NULL,		0,		0  }
};


/* race table */
const 	struct	race_type	race_table	[]		=
{
/*
    {
	name,		pc_race?,
	act bits,	aff_by bits,	off bits,
	imm,		res,		vuln,
	form,		parts 
    },
*/
    { "unique",		FALSE, 0, 0, 0, 0, 0, 0, 0, 0 },

    { 
	"human",		TRUE, 
	0,		0, 		0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"elf",			TRUE,
	0,		AFF_INFRARED,	0,
	0,		RES_CHARM,	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"dwarf",		TRUE,
	0,		AFF_INFRARED,	0,
	0,		RES_POISON|RES_DISEASE, 0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"giant",		TRUE,
	0,		0,		0,
	0,		RES_FIRE|RES_COLD,	VULN_MENTAL|VULN_LIGHTNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"orc",		TRUE,
	0,		AFF_INFRARED,	0,
	0,		RES_DISEASE,	VULN_HOLY,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"draconian",	TRUE,
	0,		0,		0,
	0,		RES_FIRE|RES_COLD,	VULN_MENTAL|VULN_LIGHTNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"bat",			FALSE,
	0,		AFF_FLYING|AFF_DARK_VISION,	OFF_DODGE|OFF_FAST,
	0,		0,		VULN_LIGHT,
	A|G|W,		A|C|D|E|F|H|J|K|P
    },

    {
	"bear",			FALSE,
	0,		0,		OFF_CRUSH|OFF_DISARM|OFF_BERSERK,
	0,		RES_BASH|RES_COLD,	0,
	A|G|V,		A|B|C|D|E|F|H|J|K|U|V
    },

    {
	"cat",			FALSE,
	0,		AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|Q|U|V
    },

    {
	"centipede",		FALSE,
	0,		AFF_DARK_VISION,	0,
	0,		RES_PIERCE|RES_COLD,	VULN_BASH
    },

    {
	"dog",			FALSE,
	0,		0,		OFF_FAST,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|U|V
    },

    {
	"doll",			FALSE,
	0,		0,		0,
	IMM_MAGIC,	RES_BASH|RES_LIGHT,
	VULN_SLASH|VULN_FIRE|VULN_ACID|VULN_LIGHTNING|VULN_ENERGY,
	E|J|M|cc,	A|B|C|G|H|K
    },

    {
	"fido",			FALSE,
	0,		0,		OFF_DODGE|ASSIST_RACE,
	0,		0,			VULN_MAGIC,
	B|G|V,		A|C|D|E|F|H|J|K|Q|V
    },		
   
    {
	"fox",			FALSE,
	0,		AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|Q|V
    },

    {
	"goblin",		FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_DISEASE,	VULN_MAGIC,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"hobgoblin",		FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_DISEASE|RES_POISON,	0
    },

    {
	"kobold",		FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_POISON,	VULN_MAGIC,
	A|B|H|M|V,	A|B|C|D|E|F|G|H|I|J|K|Q
    },

    {
	"lizard",		FALSE,
	0,		0,		0,
	0,		RES_POISON,	VULN_COLD,
	A|G|X|cc,	A|C|D|E|F|H|K|Q|V
    },

    {
	"modron",		FALSE,
	0,		AFF_INFRARED,		ASSIST_RACE|ASSIST_ALIGN,
	IMM_CHARM|IMM_DISEASE|IMM_MENTAL|IMM_HOLY|IMM_NEGATIVE,
			RES_FIRE|RES_COLD|RES_ACID,	0,
	H,		A|B|C|G|H|J|K
    },

    {
	"pig",			FALSE,
	0,		0,		0,
	0,		0,		0,
	A|G|V,	 	A|C|D|E|F|H|J|K
    },	

    {
	"rabbit",		FALSE,
	0,		0,		OFF_DODGE|OFF_FAST,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K
    },
    
    {
	"school monster",	FALSE,
	ACT_NOALIGN,		0,		0,
	IMM_CHARM|IMM_SUMMON,	0,		VULN_MAGIC,
	A|M|V,		A|B|C|D|E|F|H|J|K|Q|U
    },	

    {
	"snake",		FALSE,
	0,		0,		0,
	0,		RES_POISON,	VULN_COLD,
	A|G|R|X|Y|cc,	A|D|E|F|K|L|Q|V|X
    },
 
    {
	"song bird",		FALSE,
	0,		AFF_FLYING,		OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|W,		A|C|D|E|F|H|K|P
    },

    {
	"troll",		FALSE,
	0,		AFF_REGENERATION|AFF_INFRARED|AFF_DETECT_HIDDEN,
	OFF_BERSERK,
 	0,	RES_CHARM|RES_BASH,	VULN_FIRE|VULN_ACID,
	B|M|V,		A|B|C|D|E|F|G|H|I|J|K|U|V
    },

    {
	"water fowl",		FALSE,
	0,		AFF_FLYING,		0,
	0,		RES_DROWNING,		0,
	A|G|W,		A|C|D|E|F|H|K|P
    },		
  
    {
	"wolf",			FALSE,
	0,		AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
	0,		0,		0,	
	A|G|V,		A|C|D|E|F|J|K|Q|V
    },

    {
	"wyvern",		FALSE,
	0,		AFF_FLYING|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN,
	OFF_BASH|OFF_FAST|OFF_DODGE,
	IMM_POISON,	0,	VULN_LIGHT,
	B|Z|cc,		A|C|D|E|F|H|J|K|Q|V|X
    },

    {
	NULL, 0, 0, 0, 0, 0, 0
    }
};

const	struct	pc_race_type	pc_race_table	[]	=
{
    { "null race", "", 0, { 100, 100, 100, 100 },
      { "" }, {FALSE, FALSE, FALSE, FALSE}, 
      { 13, 13, 13, 13, 13 }, { 18, 18, 18, 18, 18 }, 0 },
 
/*
    {
	"race name", 	short name, 	points,	{ class multipliers },
	{ bonus skills }, { FALSE,FALSE,TRUE,FALSE classes } 
	{ base stats },		{ max stats },		size 
    },
*/

    {
	"human",	"Human",	0,	
	{ 300,300,300, 300,300,300, 300,300,300, 300,300,300 },
	{ "" }, { TRUE, TRUE, TRUE, TRUE },
	{ 13, 13, 13, 13, 13 },	{ 18, 18, 18, 18, 18 },	SIZE_MEDIUM
    },

    { 	
	"elf",		" Elf ",	5,
	{ 300,300,300, 300,300,300, 300,300,300, 300,300,300 },
	{ "" }, { TRUE, TRUE, TRUE, FALSE },
	{ 12, 15, 13, 14, 11 },	{ 16, 19, 18, 19, 17 }, SIZE_MEDIUM
    },

    {
	"dwarf",	"Dwarf",	5,
	{ 300,300,300, 300,300,300, 300,300,300, 300,300,300 },
	{ "" }, { FALSE, TRUE, TRUE, TRUE },
	{ 13, 12, 14, 11, 15 },	{ 18, 15, 15, 15, 19 }, SIZE_MEDIUM
    },

    {
	"giant",	"Giant",	6,
	{ 300,300,300, 300,300,300, 300,300,300, 300,300,300 },
	{ "" }, { FALSE, FALSE, FALSE, TRUE },
	{ 16, 11, 13, 11, 14 },	{ 20, 12, 13, 13, 20 }, SIZE_HUGE
    },

    {
	"orc",		" Orc ",	6,
	{ 300,300,300, 300,300,300, 300,300,300, 300,300,300 },
	{ "" }, {FALSE, FALSE, TRUE, TRUE},
	{ 14, 12, 13, 12, 14 },	{ 19, 14, 13, 18, 19 }, SIZE_LARGE
    },

    {
	"draconian",	"Dracn",	20,
	{ 300,300,300, 300,300,300, 300,300,300, 300,300,300 },
	{ "" }, { TRUE, FALSE, FALSE, TRUE},
	{ 14, 12, 14, 10, 15 },	{ 18, 17, 15, 16, 19 }, SIZE_MEDIUM
    }

};

	
      	

/*
 * Class table.
 */
const	struct	class_type	class_table	[MAX_CLASS]	=
{
    {
	"magician", "Mgn",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 3018, 9618 },  75,  18, 6,  8,  10, TRUE,
	"magician basics", "magician basics"
    },
    {
	"mage", "Mag",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 3018, 9618 },  80,  18, 6,  9,  10, TRUE,
	"magician basics", "magician basics"
    },
    {
	"sorcerer", "Sor",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 3018, 9618 },  85,  18, 6,  10,  10, TRUE,
	"magician basics", "magician basics"
    },

    {
	"acolyte", "Acl",  STAT_WIS,  OBJ_VNUM_SCHOOL_MACE,
	{ 3003, 9619 },  75,  18, 2, 10, 12, TRUE,
	"acolyte basics", "acolyte basics"
    },
    {
	"cleric", "Cle",  STAT_WIS,  OBJ_VNUM_SCHOOL_MACE,
	{ 3003, 9619 },  80,  18, 2, 11, 12, TRUE,
	"acolyte basics", "acolyte basics"
    },
    {
	"paladin", "Pal",  STAT_WIS,  OBJ_VNUM_SCHOOL_MACE,
	{ 3003, 9619 },  85,  18, 2,  12, 12, TRUE,
	"acolyte basics", "acolyte basics"
    },

    {
	"pickpocket", "Pck",  STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 3028, 9639 }, 75,  18,  -4,  9, 11, FALSE,
	"pickpocket basics", "pickpocket basics"
    },
    {
	"thief", "Thi",  STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 3028, 9639 },  80,  18,  -4,  10, 11, FALSE,
	"pickpocket basics", "pickpocket basics"
    },
    {
	"rogue", "Rog",  STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 3028, 9639 },  85,  18,  -4,  11, 11, FALSE,
	"pickpocket basics", "pickpocket basics"
    },

    {
	"swordpupil", "Swd",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
	{ 3022, 9633 },  75,  18,  -10,  12, 14, FALSE,
	"swordpupil basics", "swordpupil basics"
    },

    {
	"warrior", "War",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
	{ 3022, 9633 },  80,  18,  -10,  13, 14, FALSE,
	"swordpupil basics", "swordpupil basics"
    },

    {
	"knight", "Knt",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
	{ 3022, 9633 },  85,  18,  -10,  14, 14, FALSE,
	"swordpupil basics", "swordpupil basics"
    }
};





/*
 * Attribute bonus tables.
 */
const	struct	str_app_type	str_app		[31]		=
{
    { -5, -4,   0,  0 },  /* 0  */
    { -5, -4,   3,  1 },  /* 1  */
    { -3, -2,   3,  2 },
    { -3, -1,  10,  3 },  /* 3  */
    { -2, -1,  25,  4 },
    { -2, -1,  55,  5 },  /* 5  */
    { -1,  0,  80,  6 },
    { -1,  0,  90,  7 },
    {  0,  0, 100,  8 },
    {  0,  0, 100,  9 },
    {  0,  0, 115, 10 }, /* 10  */
    {  0,  0, 115, 11 },
    {  0,  0, 130, 12 },
    {  0,  0, 130, 13 }, /* 13  */
    {  0,  1, 140, 14 },
    {  1,  1, 150, 15 }, /* 15  */
    {  1,  2, 165, 16 },
    {  2,  3, 180, 22 },
    {  2,  3, 200, 25 }, /* 18  */
    {  3,  4, 225, 30 },
    {  3,  5, 250, 35 }, /* 20  */
    {  4,  6, 300, 40 },
    {  4,  6, 350, 45 },
    {  5,  7, 400, 50 },
    {  5,  8, 450, 55 },
    {  6,  9, 500, 60 },  /* 25   */
    {  6,  10, 550, 65 },  
    {  7,  11, 600, 70 },  
    {  7,  12, 650, 75 },  
    {  8,  13, 700, 80 },
    {  8,  14, 750, 85 }  /* 30   */
};



const	struct	int_app_type	int_app		[31]		=
{
    {  3 },	/*  0 */
    {  5 },	/*  1 */
    {  7 },
    {  8 },	/*  3 */
    {  9 },
    { 10 },	/*  5 */
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },	/* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 28 },
    { 31 },	/* 15 */
    { 34 },
    { 37 },
    { 40 },	/* 18 */
    { 44 },
    { 49 },	/* 20 */
    { 55 },
    { 60 },
    { 70 },
    { 80 },
    { 85 },	/* 25 */
    { 87 },
    { 89 },
    { 90 },
    { 91 },	/* 29 */
    { 93 }
};



const	struct	wis_app_type	wis_app		[31]		=
{
    { 0 },	/*  0 */
    { 0 },	/*  1 */
    { 0 },
    { 0 },	/*  3 */
    { 0 },
    { 1 },	/*  5 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 1 },	/* 10 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 2 },	/* 15 */
    { 2 },
    { 2 },
    { 3 },	/* 18 */
    { 3 },
    { 3 },	/* 20 */
    { 3 },
    { 4 },
    { 4 },
    { 4 },
    { 5 },	/* 25 */
    { 5 },
    { 5 },
    { 6 },
    { 6 },
    { 6 }	/* 30 */
};



const	struct	dex_app_type	dex_app		[31]		=
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -120 },    /* 25 */
    { -125 },
    { -130 },
    { -135 },
    { -140 },
    { -145 }
};


const	struct	con_app_type	con_app		[31]		=
{
    { -4, 20 },   /*  0 */
    { -3, 25 },   /*  1 */
    { -2, 30 },
    { -2, 35 },	  /*  3 */
    { -1, 40 },
    { -1, 45 },   /*  5 */
    { -1, 50 },
    {  0, 55 },
    {  0, 60 },
    {  0, 65 },
    {  0, 70 },   /* 10 */
    {  0, 75 },
    {  0, 80 },
    {  0, 85 },
    {  0, 88 },
    {  1, 90 },   /* 15 */
    {  2, 95 },
    {  2, 97 },
    {  3, 99 },   /* 18 */
    {  3, 99 },
    {  4, 99 },   /* 20 */
    {  4, 99 },
    {  5, 99 },
    {  6, 99 },
    {  7, 99 },
    {  8, 99 },    /* 25 */
    {  9, 99 },
    {  9, 99 },
    {  9, 99 },
    {  10, 99 },
    {  10, 99 }    /* 30 */
};



/*
 * Liquid properties.
 * Used in world.obj.
 */
const	struct	liq_type	liq_table	[]	=
{
    { "water",			"clear",	{  0, 1, 10 }	},  /*  0 */
    { "beer",			"amber",	{  3, 2,  5 }	},
    { "wine",			"rose",		{  5, 2,  5 }	},
    { "ale",			"brown",	{  2, 2,  5 }	},
    { "dark ale",		"dark",		{  1, 2,  5 }	},

    { "whisky",			"golden",	{  6, 1,  4 }	},  /*  5 */
    { "lemonade",		"pink",		{  0, 1,  8 }	},
    { "firebreather",		"boiling",	{ 10, 0,  0 }	},
    { "local specialty",	"everclear",	{  3, 3,  3 }	},
    { "slime mold juice",	"green",	{  0, 4, -8 }	},

    { "milk",			"white",	{  0, 3,  6 }	},  /* 10 */
    { "tea",			"tan",		{  0, 1,  6 }	},
    { "coffee",			"black",	{  0, 1,  6 }	},
    { "blood",			"red",		{  0, 2, -1 }	},
    { "salt water",		"clear",	{  0, 1, -2 }	},

    { "cola",			"cherry",	{  0, 1,  5 }	},  /*15*/
    { NULL,			NULL,		{  0, 0,  0 }   }
};



/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n

const	struct	skill_type	skill_table	[MAX_SKILL]	=
{

/*
 * Magic spells.
 */

    {
	"reserved",{ 99,99,99, 99,99,99, 99,99,99, 99,99,99 },
	{ 0,0,0, 0,0,0, 0,0,0, 0,0,0},
	0,			TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT( 0),	 0,	 0,
	"",			""
    },

    {
	"acid blast",{ 28,25,22, 95,95,95, 95,95,95, 95,95,95 },
	{1,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_acid_blast,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(70),	20,	12,
	"acid blast",		"!Acid Blast!"
    },

    {
	"armor",{ 7,7,7, 2,2,2, 20,15,10, 25,15,10 },
	{ 1,1,1, 1,1,1, 2,2,2, 2,2,2},
	spell_armor,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT( 1),	 5,	12,
	"",			"You feel less protected."
    },

    {
	"bless",{ 95,95,95, 15,10,7, 95,95,95, 95,95,95 },
	{ 0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_bless,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT( 3),	 5,	12,
	"",			"You feel less righteous."
    },

    {
	"blindness",{ 15,13,11, 13,12,11, 95,95,95, 95,95,95 },
	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_blindness,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_blindness,		SLOT( 4),	 5,	12,
	"",			"You can see again."
    },
    {
	"heat metal",{ 95,95,35, 95,95,95, 95,95,95, 95,95,95 },
	{ 0,0,2, 0,0,0, 0,0,0, 0,0,0},
	spell_heat_metal,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(125),	 150,	36,
	"",			"!Heat Metal!"
    },

    {
	"burning hands",{ 7,6,5, 95,95,95, 95,95,95, 95,95,95 },
	{ 1,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_burning_hands,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT( 5),	15,	12,
	"burning hands",	"!Burning Hands!"
    },

    {
	"call lightning",{ 26,25,24, 18,17,16, 95,95,95, 95,95,95 },
	{1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_call_lightning,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT( 6),	15,	12,
	"lightning bolt",	"!Call Lightning!"
    },

    {   "calm",{ 95,95,95, 16,15,14, 95,95,95, 95,95,95 },
	{ 0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_calm,		TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(509),	30,	12,
	"",			"You have lost your peace of mind."
    },

    {
	"cancellation",	{ 18,17,16, 26,25,24, 34,33,32, 39,37,35 },
	{ 1,1,1, 1,1,1, 2,2,2, 2,2,2},
	spell_cancellation,	TAR_CHAR_SELF,	POS_FIGHTING,
	NULL,			SLOT(507),	20,	12,
	""			"!cancellation!",
    },

    {
	"cause critical",{ 95,95,95, 13,12,11, 95,95,95, 95,95,95 },
	{ 0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_cause_critical,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(63),	20,	12,
	"spell",		"!Cause Critical!"
    },

    {
	"cause light",{ 95,95,95, 1,1,1, 95,95,95, 95,95,95 },
	{ 0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_cause_light,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(62),	15,	12,
	"spell",		"!Cause Light!"
    },

    {
	"cause serious",{ 95,95,95, 7,6,5, 95,95,95, 95,95,95 },
	{ 0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_cause_serious,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(64),	17,	12,
	"spell",		"!Cause Serious!"
    },

    {   
	"chain lightning",{ 32,31,30, 95,95,95, 95,95,95, 95,95,95 },
	{ 1,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_chain_lightning,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(500),	25,	12,
	"lightning",		"!Chain Lightning!"
    }, 

    {
	"change sex",{70,70,70, 70,70,70, 95,95,95, 95,95,95,},
	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_change_sex,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(82),	15,	12,
	"",			"Your body feels familiar again."
    },

    {
	"charm person",	{ 20,19,18, 95,95,95, 45,35,25, 95,95,95},
	{ 1,1,1, 0,0,0, 2,2,2, 0,0,0},
	spell_charm_person,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_charm_person,	SLOT( 7),	 5,	12,
	"",			"You feel more self-confident."
    },

    {
	"chill touch",{ 4,3,2, 95,95,95, 95,95,95, 95,95,95 },
	{ 1,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_chill_touch,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT( 8),	15,	12,
	"chilling touch",	"You feel less cold."
    },

    {
	"colour spray",{ 16,15,14, 95,95,95, 95,95,95, 95,95,95 },
	{ 1,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_colour_spray,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(10),	15,	12,
	"colour spray",		"!Colour Spray!"
    },

    {
	"continual light",{ 6,5,4, 4,3,2, 95,95,95, 95,95,95 },
	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_continual_light,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(57),	 7,	12,
	"",			"!Continual Light!"
    },
    {
	"charisma",{ 40,35,30, 95,95,95, 95,95,95, 95,95,95 },
	{ 1,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_charisma,		TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(99),	 25,	12,
	"",			"!Charisma!"
    },

    {
	"control weather",{ 15,14,13, 19,18,17, 95,95,95, 95,95,95 },
	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_control_weather,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(11),	25,	12,
	"",			"!Control Weather!"
    },

    {
	"create food",{ 10,9,8, 5,4,3, 95,95,95, 95,95,95 },
	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_create_food,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(12),	 5,	12,
	"",			"!Create Food!"
    },

    {
        "create sign",{ 20,19,18, 95,95,95, 95,95,95, 95,95,95 },
        { 1,1,1, 0,0,0, 0,0,0, 0,0,0},
        spell_create_sign,      TAR_IGNORE,             POS_STANDING,
        NULL,                   SLOT(85),        25,     12,
        "",                     "!Create Sign!"
    },

    {
	"create spring",{ 14,13,12, 17,16,15, 95,95,95, 95,95,95 },
	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_create_spring,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(80),	20,	12,
	"",			"!Create Spring!"
    },
    {
	"eternal light",{ 3,2,1, 3,2,1, 95,95,95, 95,95,95 },
	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_eternal_light,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(97),	20,	12,
	"",			"!Eternal Light!"
    },
    {
	"preserve",{ 15,13,12, 15,13,12, 95,95,95, 95,95,95 },
	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_preserve, 	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(98),	20,	12,
	"",			"!Preserve!"
    },

    {
	"create water",{ 8,7,6, 3,2,1, 95,95,95, 95,95,95 },
	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_create_water,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(13),	 5,	12,
	"",			"!Create Water!"
    },

    {
	"cure blindness",{ 95,95,95, 6,5,4, 95,95,95, 95,95,95 },
	{ 0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_cure_blindness,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(14),	 5,	12,
	"",			"!Cure Blindness!"
    },

    {
	"cure critical",{ 95,95,95, 13,12,11, 95,95,95, 95,95,95 },
     	{0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_cure_critical,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(15),	20,	12,
	"",			"!Cure Critical!"
    },

    {
	"cure disease",{ 95,95,95, 13,12,11, 95,95,95, 95,95,95 },
     	{0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_cure_disease,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(501),	20,	12,
	"",			"!Cure Disease!"
    },

    {
	"cure light",{ 95,95,95, 1,1,1, 95,95,95, 95,95,95},
	{0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_cure_light,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(16),	10,	12,
	"",			"!Cure Light!"
    },

    {
	"cure poison",{ 95,95,95, 14,13,12, 95,95,95, 95,95,95 },
     	{ 0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_cure_poison,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(43),	 5,	12,
	"",			"!Cure Poison!"
    },

    {
	"cure serious",{ 95,95,95, 7,6,5, 95,95,95, 95,95,95 },
     	{ 0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_cure_serious,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(61),	15,	12,
	"",			"!Cure Serious!"
    },

    {
	"curse",{ 18,17,16, 18,17,16, 95,95,95, 95,95,95 },
	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_curse,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_curse,		SLOT(17),	20,	12,
	"curse",		"The curse wears off."
    },

    {
	"curse object",{ 30,28,26, 95,95,95, 95,95,95, 95,95,95 },
	{ 1,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_curse_object,     TAR_OBJ_INV,    POS_STANDING,
	NULL,		        SLOT(94),	30,	12,
	"",			"!curse object!"
    },

    {
	"demonfire",{ 95,95,95, 34,33,32, 95,95,95, 95,95,95 },
     	{ 0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_demonfire,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(505),	20,	12,
	"torments",		"!Demonfire!"
    },	

    {
	"detect evil",{ 12,11,10, 4,3,2, 22,20,15, 95,95,95, },
     	{ 1,1,1, 1,1,1, 2,2,2, 0,0,0},
	spell_detect_evil,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(18),	 5,	12,
	"",			"The red in your vision disappears."
    },

    {
	"detect traps",{ 15,13,11, 15,13,11, 6,5,4, 95,95,95, },
     	{ 1,1,1, 1,1,1, 2,2,2, 0,0,0},
	spell_detect_traps,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(20),	 5,	12,
	"",			"You can no longer sense where traps are."
    },

    {
	"detect hidden",{ 15,14,13, 11,10,9, 22,20,18, 95,95,95 },
	{ 1,1,1, 1,1,1, 2,2,2, 0,0,0},
	spell_detect_hidden,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(44),	 5,	12,
	"",			"You feel less aware of your suroundings."
    },

    {
	"detect invis",{ 3,2,1, 8,7,6, 26,20,16, 95,95,95, },
     	{ 1,1,1, 1,1,1, 2,2,2, 0,0,0},
	spell_detect_invis,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(19),	 5,	12,
	"",			"You no longer see invisible objects."
    },

    {
	"remove invis",{ 95,30,25, 95,95,95, 95,95,95, 95,95,95, },
     	{ 0,2,2, 0,0,0, 0,0,0, 0,0,0},
	spell_remove_invis,	TAR_OBJ_INV,     POS_STANDING,
	NULL,			SLOT(87),	 20,	20,
	"",			""
    },
    {
	"remove rot",   { 95,65,55, 95,95,95, 95,95,95, 95,95,95, },
     	{ 0,2,2, 0,0,0, 0,0,0, 0,0,0},
	spell_remove_rot,	TAR_OBJ_INV,     POS_STANDING,
	NULL,			SLOT(96),	 50,	20,
	"",			""
    },
    {
	"rot object",   { 95,55,45, 95,95,95, 95,95,95, 95,95,95, },
     	{ 0,2,2, 0,0,0, 0,0,0, 0,0,0},
	spell_rot_object,	TAR_OBJ_INV,     POS_STANDING,
	NULL,			SLOT(95),	 50,	20,
	"",			""
    },

    {
	"detect poison",{ 15,13,11, 7,6,5, 19,17,16, 95,95,95 },
     	{ 1,1,1, 1,1,1, 2,2,2, 0,0,0},
	spell_detect_poison,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(21),	 5,	12,
	"",			"!Detect Poison!"
    },

    {
	"dispel evil",{ 95,95,95, 15,13,12, 95,95,95, 95,95,95 },
     	{ 0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_dispel_evil,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(22),	15,	12,
	"dispel evil",		"!Dispel Evil!"
    },

    {
	"dispel magic",{ 16,15,14, 24,22,20, 40,37,34, 50,48,46 },
     	{ 1,1,1, 1,1,1, 2,2,2, 2,2,2},
	spell_dispel_magic,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(59),	15,	12,
	"",			"!Dispel Magic!"
    },

    {
	"earthquake",{ 95,95,95, 10,9,8, 95,95,95, 95,95,95 },
     	{ 0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_earthquake,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(23),	15,	12,
	"earthquake",		"!Earthquake!"
    },

    {
	"enchant armor",{ 20,18,16, 95,95,95, 95,95,95, 95,95,95 },
	{1,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_enchant_armor,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(510),	100,	24,
	"",			"!Enchant Armor!"
    },

    {
	"enchant weapon",{ 17,16,15, 95,95,95, 95,95,95, 95,95,95 },
     	{ 1,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_enchant_weapon,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(24),	100,	24,
	"",			"!Enchant Weapon!"
    },
    {
	"shrink",{ 95,50,30, 95,95,95, 95,95,95, 95,95,95 },
     	{ 0,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_shrink,   	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(89),	 100,	 36,
	"",			"!Shrink!"
    },

    {
	"energy drain",{ 19,18,17, 22,21,20, 95,95,95, 95,95,95 },
     	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_energy_drain,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(25),	35,	12,
	"energy drain",		"!Energy Drain!"
    },

    {
	"faerie fire",{ 6,5,4, 3,2,1, 95,95,95, 95,95,95 },
     	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_faerie_fire,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(72),	 5,	12,
	"faerie fire",		"The pink aura around you fades away."
    },

    {
	"faerie fog",{ 14,13,12, 21,20,19, 95,95,95, 95,95,95 },
     	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_faerie_fog,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(73),	12,	12,
	"faerie fog",		"!Faerie Fog!"
    },

    { 
	"fear",{ 50,48,45, 45,43,42, 95,95,95, 95,95,95 },
	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_fear,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(79),	15,	18,
	"fear",			"!Fear!"
    },

    {
	"fireball",{ 22,21,20, 95,95,95, 95,95,95, 95,95,95 },
     	{ 1,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_fireball,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(26),	15,	12,
	"fireball",		"!Fireball!"
    },

    {
	"flamestrike",{ 95,95,95, 20,19,18, 95,95,95, 95,95,95 },
     	{ 0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_flamestrike,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(65),	20,	12,
	"flamestrike",		"!Flamestrike!"
    },
    {
	"flamewall",{ 95,95,30, 95,95,95, 95,95,95, 95,95,95 },
     	{ 0,0,3, 0,0,0, 0,0,0, 0,0,0},
	spell_flamewall,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(92),	75,	20,
	"flamewall",		"!Flamewall!"
    },
    {
	"flame shield",{ 95,95,50, 95,95,95, 95,95,95, 95,95,95 },
     	{ 0,0,5, 0,0,0, 0,0,0, 0,0,0},
	spell_flameshield,	TAR_CHAR_SELF,	POS_STANDING,
	NULL,			SLOT(84),	75,	20,
	"flame shield", "The flame shield wears off."
    },

    {
	"fly",{ 10,9,8, 18,17,16, 95,95,95, 95,95,95 },
     	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_fly,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(56),	10,	18,
	"",			"You slowly float to the ground."
    },

    {
        "frenzy",{ 95,95,95, 23,22,21, 95,95,95, 95,95,95 },
     	{ 0,0,0, 1,1,1, 0,0,0, 0,0,0},
        spell_frenzy,           TAR_CHAR_DEFENSIVE,     POS_STANDING,
        NULL,                   SLOT(504),      30,     24,
        "",                     "Your rage ebbs."
    },

    {
	"gate",{ 18,16,15, 15,14,13, 35,33,31, 48,45,40 },
     	{ 1,1,1, 1,1,1, 2,2,2, 2,2,2},
	spell_gate,		TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(83),	50,	2,
	"",			"!Gate!"
    },

    {
	"portal",{ 27,25,23, 25,23,21, 95,70,65, 95,70,65 },     
	{1,1,1, 1,1,1, 0,3,2, 0,3,2},
	spell_portal,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(100),	65,	2,
	"",			"!Portal!"
    },

    {
	"nexus",{ 38,36,35, 30,29,27, 95,95,75, 95,95,75 },
     	{ 1,1,1, 1,1,1, 0,0,3, 0,0,3},
	spell_nexus,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(101),	80,	20,
	"",			"!Nexus!"
    },

    {
	"giant strength",{ 11,10,9, 95,95,95, 95,95,95, 95,95,95 },
     	{ 1,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_giant_strength,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(39),	20,	12,
	"",			"You feel weaker."
    },

    {
	"harm",{ 95,95,95, 23,22,21, 95,95,95, 95,95,95 },
     	{ 0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_harm,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(27),	35,	12,
	"harm spell",		"!Harm!"
    },
  
    {
	"haste",{ 21,20,20, 95,95,95, 95,95,95, 95,95,95 },
     	{ 1,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_haste,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(502),	30,	12,
	"",			"You feel yourself slow down."
    },

    {
	"heal",{ 95,95,95, 21,20,19, 95,95,95, 95,95,95 },
     	{ 0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_heal,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(28),	50,	12,
	"",			"!Heal!"
    },

    {
	"divine healing",{ 95,95,95, 95,90,80, 95,95,95, 95,95,95 },
     	{ 0,0,0, 0,3,3, 0,0,0, 0,0,0},
	spell_divine_healing,	TAR_CHAR_SELF,      POS_STANDING,
	NULL,			SLOT(90),	75,	12,
	"",			"!Divine Healing!"
    },

    {
	"holy word",{ 95,95,95, 36,35,34, 95,95,95, 95,95,95 },
	{ 0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_holy_word,	TAR_CHAR_SELF,	POS_STANDING,
	NULL,			SLOT(506), 	35,	12,
	"",			"!Holy Word!"
    },

    {
	"identify",{ 15,14,13, 16,15,14, 28,26,24, 95,95,95 },
     	{ 1,1,1, 1,1,1, 2,2,2, 0,0,0},
	spell_identify,		TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(53),	12,	24,
	"",			"!Identify!"
    },

    {
	"infravision",{ 9,8,7, 13,12,11, 30,27,25, 95,95,95 },
     	{ 1,1,1, 1,1,1, 2,2,2, 0,0,0},
	spell_infravision,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(77),	 5,	18,
	"",			"You no longer see in the dark."
    },

    {
	"invis",{ 5,4,3, 95,95,95, 19,17,15, 95,95,95 },
     	{ 1,1,1, 0,0,0, 2,2,2, 0,0,0},
	spell_invis,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_invis,		SLOT(29),	 5,	12,
	"",			"You are no longer invisible."
    },

    {
	"invis object",{ 95,25,20, 95,95,95, 95,95,95, 95,95,95, },
     	{ 0,2,2, 0,0,0, 0,0,0, 0,0,0},
	spell_invis_object,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(86),	 20,	20,
	"",			""
    },

    {
	"know alignment",{ 12,11,10, 9,8,7, 20,19,18, 95,95,95 },
     	{ 1,1,1, 1,1,1, 2,2,2, 0,0,0},
	spell_know_alignment,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(58),	 9,	12,
	"",			"!Know Alignment!"
    },

    {	"light feet",{ 15,14,13, 20,19,18, 95,95,95, 95,95,95 },
     	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},        
	spell_light_feet,       TAR_CHAR_SELF,          POS_STANDING,        
	NULL,                   SLOT(183),        10,     12,        
	"",                     "Your movement becomes loud again."    
    },

    {
	"lightning bolt",{ 13,12,11, 95,95,95, 95,95,95, 95,95,95 },
	{ 1,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_lightning_bolt,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(30),	15,	12,
	"lightning bolt",	"!Lightning Bolt!"
    },

    {
	"locate object",{ 9,8,7, 15,14,13, 35,33,31, 95,95,95 },
     	{ 1,1,1, 1,1,1, 2,2,2, 0,0,0},
	spell_locate_object,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(31),	20,	18,
	"",			"!Locate Object!"
    },

      {
	"locate person",{ 20,19,18, 35,34,32, 65,62,60, 95,95,95 },
     	{ 1,1,1, 1,1,1, 2,2,2, 0,0,0},	
        spell_locate_person,	TAR_IGNORE,		POS_STANDING,	
        NULL,			SLOT(669),	20,	18,	
	"",			"!Locate Person!"    
     },

    {
	"magic missile",{ 1,1,1, 95,95,95, 95,95,95, 95,95,95 },
     	{ 1,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_magic_missile,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(32),	15,	12,
	"magic missile",	"!Magic Missile!"
    },

    {
	"make bag",{ 50,47,45, 50,46,43, 95,95,95, 95,95,95 },
   	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},	
	spell_make_bag,		TAR_OBJ_INV,		POS_STANDING,	
	NULL,			SLOT(50),	100,	24,	
	"",			"!Make Bag!"
    },

    {
	"mass healing",{ 95,95,95, 45,43,41, 95,95,95, 95,95,95 },
	{0,0,0, 2,2,2, 0,0,0, 0,0,0},
	spell_mass_healing,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(508),	100,	36,
	"",			"!Mass Healing!"
    },

    {
	"mass invis",{ 22,21,20, 95,95,95, 31,30,28, 95,95,95 },
     	{ 1,1,1, 0,0,0, 2,2,2, 0,0,0},
	spell_mass_invis,	TAR_IGNORE,		POS_STANDING,
	&gsn_mass_invis,	SLOT(69),	20,	24,
	"",			"!Mass Invis!"
    },

    {
        "nuclear blast",{ 69,67,65, 95,95,95, 95,95,95, 95,95,95 },
     	{ 1,1,1, 0,0,0, 0,0,0, 0,0,0},
        spell_nuclear_blast,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(76),       15,     12,
        "nuclear blast",             "!Nuclear Blast!"
    },

    {
	"pass door",{ 24,22,21, 32,30,29, 95,95,95, 95,95,95 },
     	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_pass_door,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(74),	20,	12,
	"",			"You feel solid again."
    },

    {
	"plague",{ 23,22,21, 17,16,15, 95,95,95, 95,95,95 },
     	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_plague,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_plague,		SLOT(503),	20,	12,
	"sickness",		"Your sores vanish."
    },

    {
	"poison",{ 17,16,15, 12,11,10, 95,95,95, 95,95,95 },
     	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_poison,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_poison,		SLOT(33),	10,	12,
	"poison",		"You feel less sick."
    },

    {
	"protection evil",{ 12,11,10, 9,8,7, 27,25,23, 31,29,27 },
     	{ 1,1,1, 1,1,1, 2,2,2, 2,2,2},
	spell_protection,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(34),	 5,	12,
	"",			"You feel less protected against evil."
    },
    {
	"protection good",{ 12,11,10, 9,8,7, 27,25,23, 31,29,27 },
     	{ 1,1,1, 1,1,1, 2,2,2, 2,2,2},
	spell_protection_good,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(88),	 5,	12,
	"",			"You feel less protected against good."
    },

    {
        "quench",{ 95,35,25, 95,40,30, 95,95,95, 95,95,95},
	{ 0,1,1, 0,1,1, 0,0,0, 0,0,0},
        spell_quench,		TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(77),	20,     20,
	"",			"!Quench!"
    },
    {
        "quiet",{ 95,95,40, 95,95,50, 95,95,95, 95,95,95},
	{ 0,0,2, 0,0,2, 0,0,0, 0,0,0},
        spell_quiet,		TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(91),	50,     20,
	"",			"Others can now hear your spells."
    },

    {
	"refresh",{ 8,7,6, 5,4,3, 95,95,95, 95,95,95 },
     	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_refresh,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(81),	12,	18,
	"refresh",		"!Refresh!"
    },

    {
	"remove curse",{ 95,95,95, 18,17,16, 95,95,95, 52,50,48 },
     	{ 0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_remove_curse,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(35),	 5,	12,
	"",			"!Remove Curse!"
    },

    {
	"sanctuary",{ 36,35,34, 20,19,18, 52,50,48, 60,58,56 },
     	{ 1,1,1, 1,1,1, 2,2,2, 2,2,2},
	spell_sanctuary,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(36),	75,	12,
	"",			"The white aura around your body fades."
    },

    {
	"sate",{95,50,40, 95,55,45, 95,95,95, 95,95,95},
      	{ 0,1,1, 0,1,1, 0,0,0, 0,0,0},
	spell_sate,		TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(78),	20,	30,
	"",			"!Sate!"
    },

    {
	"shield",{ 20,19,18, 35,34,33, 55,45,35, 60,50,40 },
     	{ 1,1,1, 1,1,1, 2,2,2, 2,2,2},
	spell_shield,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(67),	12,	18,
	"",			"Your force shield shimmers then fades away."
    },

    {
	"blink",{ 10,9,8, 95,95,95, 95,95,95, 95,95,95 },
     	{ 1,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_blink,		TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(71),	30,	18,
	"",			"You stop flickering out of reality."
    },

    {
	"remove alignment",{ 75,70,65, 95,95,95, 95,95,95, 95,95,95 },
	{ 1,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_remove_alignment,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(0),	75,	40,
	"",			"!Remove Alignment!"
    },

    {
	"fireproof",{ 50,40,30, 95,95,95, 95,95,95, 95,95,95 },
	{ 1,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_fireproof,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(0),	50,	24,
	"",			"!Fireproof!"
    },

    {
	"shocking grasp",{ 10,9,8, 95,95,95, 95,95,95, 95,95,95},
     	{ 1,1,1, 0,0,0, 0,0,0, 0,0,0},
	spell_shocking_grasp,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(53),	15,	12,
	"shocking grasp",	"!Shocking Grasp!"
    },

    {
	"sleep",{ 10,9,8, 95,95,95, 20,17,15, 95,95,95 },
     	{ 1,1,1, 0,0,0, 2,2,2, 0,0,0},
	spell_sleep,		TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_sleep,		SLOT(38),	15,	12,
	"",			"You feel less tired."
    },

    {
	"stone skin",{ 25,24,23, 40,38,36, 50,47,44, 60,55,50 },
     	{ 1,1,1, 1,1,1, 2,2,2, 2,2,2},
	spell_stone_skin,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(66),	12,	18,
	"",			"Your skin feels soft again."
    },

    {
	"summon",{ 24,23,22, 12,11,10, 49,47,46, 52,50,48 },
    	{ 1,1,1, 1,1,1, 2,2,2, 2,2,2},
	spell_summon,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(40),	50,	12,
	"",			"!Summon!"
    },

    {
	"teleport",{ 13,12,11, 22,21,20, 55,53,51, 66,64,63 },
     	{ 1,1,1, 1,1,1, 2,2,2, 2,2,2},
	spell_teleport,		TAR_CHAR_SELF,		POS_FIGHTING,
	NULL,	 		SLOT( 2),	35,	12,
	"",			"!Teleport!"
    },

    {
	"ventriloquate",{ 1,1,1, 95,95,95, 2,2,2, 95,95,95 },
     	{ 1,1,1, 1,1,1, 2,2,2, 2,2,2},
	spell_ventriloquate,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(41),	 5,	12,
	"",			"!Ventriloquate!"
    },

    {
	"weaken",{ 11,10,9, 14,13,11, 95,95,95, 95,95,95 },
     	{ 1,1,1, 1,1,1, 0,0,0, 0,0,0},
	spell_weaken,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(68),	20,	12,
	"spell",		"You feel stronger."
    },

    {
	"web",{ 95,30,30, 95,95,45, 95,95,95, 95,95,95 },
     	{ 0,1,1, 0,0,1, 0,0,0, 0,0,0},
	spell_web,		TAR_IGNORE,	POS_STANDING,
	NULL,			SLOT(102),	15,	12,
	"web",			"The webs around you break up and dissolve."
    },

    {
	"wrath of god",{ 95,95,95, 69,67,65, 95,95,95, 95,95,95 },
     	{ 0,0,0, 1,1,1, 0,0,0, 0,0,0},
	spell_wrath_of_god,	TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,			SLOT(95),	20,	12,
	"prayer",      		"!Wrath!"
    },

    {
	"word of recall",{ 32,31,30, 28,27,26, 50,47,45, 60,57,55 },
     	{ 1,1,1, 1,1,1, 2,2,2, 2,2,2},
	spell_word_of_recall,	TAR_CHAR_SELF,		POS_RESTING,
	NULL,			SLOT(42),	 5,	12,
	"",			"!Word of Recall!"
    },

/*
 * Dragon breath
 */
    {
	"acid breath",{1,1,1, 1,1,1, 1,1,1, 1,1,1},
	{ 1,1,1, 1,1,1, 1,1,1, 1,1,1},
	spell_acid_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(200),	 5,	 12,
	"blast of acid",	"!Acid Breath!"
    },

    {
	"fire breath",{ 1,1,1, 1,1,1, 1,1,1, 1,1,1},
	{ 1,1,1, 1,1,1, 1,1,1, 1,1,1},
	spell_fire_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(201),	 5,	 12,
	"blast of flame",	"!Fire Breath!"
    },

    {
	"frost breath",{ 1,1,1, 1,1,1, 1,1,1, 1,1,1},
     	{ 1,1,1, 1,1,1, 1,1,1, 1,1,1},
	spell_frost_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(202),	 5,	 12,
	"blast of frost",	"!Frost Breath!"
    },

    {
	"gas breath",{ 1,1,1, 1,1,1, 1,1,1, 1,1,1},
        { 1,1,1, 1,1,1, 1,1,1, 1,1,1},
	spell_gas_breath,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(203),	 5,	 12,
	"blast of gas",		"!Gas Breath!"
    },

    {
	"lightning breath",{ 1,1,1, 1,1,1, 1,1,1, 1,1,1 },
     	{ 1,1,1, 1,1,1, 1,1,1, 1,1,1},
	spell_lightning_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(204),	 5,	 12,
	"blast of lightning",	"!Lightning Breath!"
    },

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
    {
        "general purpose",{ 99,99,99, 99,99,99, 99,99,99, 99,99,99 },
	{ 0,0,0, 0,0,0, 0,0,0, 0,0,0 },
        spell_general_purpose,  TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(401),      0,      12,
        "general purpose ammo", "!General Purpose Ammo!"
    },
 
    {
        "high explosive",{ 99,99,99, 99,99,99, 99,99,99, 99,99,99 },
	{ 0,0,0, 0,0,0, 0,0,0, 0,0,0 },
        spell_high_explosive,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(402),      0,      12,
        "high explosive ammo",  "!High Explosive Ammo!"
    },


/* combat and weapons skills */


    {
	"axe",{ 95,95,95, 95,95,95, 95,95,95, 1,1,1 },
	{ 0,0,0, 0,0,0, 0,0,0, 1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_axe,            	SLOT( 0),       0,      0,
        "",                     "!Axe!"
    },

    {
        "dagger",{ 1,1,1, 95,95,95, 1,1,1, 1,1,1 },
        { 2,2,2, 0,0,0, 2,2,2, 1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_dagger,            SLOT( 0),       0,      0,
        "",                     "!Dagger!"
    },
 
    {
	"flail", { 95,95,95, 1,1,1, 95,95,95, 1,1,1 },
	{ 0,0,0, 2,2,2, 0,0,0, 1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_flail,            	SLOT( 0),       0,      0,
        "",                     "!Flail!"
    },

    {
	"mace",{ 95,95,95, 1,1,1, 95,95,95, 1,1,1 },
	{ 0,0,0, 2,2,2, 0,0,0, 1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_mace,            	SLOT( 0),       0,      0,
        "",                     "!Mace!"
    },

    {
	"polearm",{ 95,95,95, 95,95,95, 95,95,95, 1,1,1 },
	{ 0,0,0, 0,0,0, 0,0,0, 1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_polearm,           SLOT( 0),       0,      0,
        "",                     "!Polearm!"
    },
    
    {
	"shield block",{ 95,95,95, 1,1,1, 95,95,95, 1,1,1 },
	{ 0,0,0, 5,5,5, 0,0,0, 1,1,1},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_shield_block,	SLOT(0),	0,	0,
	"",			"!Shield!"
    },

    {
	"settrap",{ 95,95,95, 95,95,95, 95,90,85, 95,95,95 },
	{ 0,0,0, 0,0,0, 0,5,5, 0,0,0},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_set_trap,		SLOT(0),	0,	60,
	"trap",			"!Trap!"
    },
 
    {
	"spear",{ 95,95,95, 95,95,95, 95,95,95, 1,1,1 },
	{ 0,0,0, 0,0,0, 0,0,0, 1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_spear,            	SLOT( 0),       0,      0,
        "",                     "!Spear!"
    },

    {
	"sword",{ 95,95,95, 95,30,5, 1,1,1, 1,1,1},
	{ 0,0,0, 0,3,3, 2,2,2, 2,2,2},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_sword,            	SLOT( 0),       0,      0,
        "",                     "!sword!"
    },

    {
	"grip",{ 95,95,95, 95,50,25, 95,95,95, 25,23,21},
	{ 0,0,0, 0,2,2, 0,0,0, 1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_grip,            	SLOT( 0),       0,      0,
        "",                     "!grip!"
    },

    {
	"whip",{ 95,95,95, 95,95,95, 95,95,95, 1,1,1},
	{ 0,0,0, 0,0,0, 0,0,0, 1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_whip,            	SLOT( 0),       0,      0,
        "",                     "!Whip!"
    },

    {
        "backstab",{95,95,95, 95,95,95, 15,12,10, 95,95,95},
	{0,0,0, 0,0,0, 6,5,4, 0,0,0},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_backstab,          SLOT( 0),        0,     36,
        "backstab",             "!Backstab!"
    },
    {
        "circle",{95,95,95, 95,95,95, 45,40,35, 95,95,95},
	{0,0,0, 0,0,0, 5,5,5, 0,0,0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_circle,          SLOT( 0),        0,     24,
        "circle",             "!Circle!"
    },

    {
	"bash",{95,95,95, 95,20,15, 95,95,95, 20,15,10 },
	{0,0,0, 0,5,4, 0,0,0, 4,4,4 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_bash,            	SLOT( 0),       0,      24,
        "bash",                 "!Bash!"
    },
    {
	"thrash",{95,95,95, 95,20,35, 95,95,95, 30,35,30 },
	{0,0,0, 0,5,5, 0,0,0, 5,5,5 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_crush,            	SLOT( 0),       0,      2,
        "thrashing",                 "!Crush!"
    },

    {
	"evade",{95,95,95, 95,95,95, 95,25,20, 20,15,10 },
	{0,0,0, 0,0,0, 0,4,4, 3,3,3 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_evade,            	SLOT( 0),       0,      0,
        "",                     "!evade!"
    },

    {
	"berserk",{95,95,95, 95,95,95, 95,95,95, 18,15,13},
	{0,0,0, 0,0,0, 0,0,0, 4,4,4},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_berserk,        	SLOT( 0),       0,      24,
        "",                     "You feel your pulse slow down."
    },

    {
	"dirt kicking",{95,95,95, 95,95,95, 5,4,3, 3,3,3},
	{0,0,0, 0,0,0, 2,2,2, 2,2,2}, 
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_dirt,		SLOT( 0),	0,	24,
	"kicked dirt",		"You rub the dirt out of your eyes."
    },

    {
        "disarm",{95,95,95, 95,20,15, 15,13,12, 11,11,11},
	{0,0,0, 0,4,4, 3,3,3, 3,3,3},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_disarm,            SLOT( 0),        0,     24,
        "",                     "!Disarm!"
    },
 
    {
        "dodge",{20,15,10, 22,17,12, 1,1,1, 13,11,9},
	{9,9,9, 6,6,6, 4,4,4, 4,4,4},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_dodge,             SLOT( 0),        0,      0,
        "",                     "!Dodge!"
    },
    {
        "duck",{95,95,95, 95,95,95, 95,24,22, 95,95,95},
	{0,0,0, 0,0,0, 0,4,4, 0,0,0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_duck,              SLOT( 0),        0,      0,
        "",                     "!Duck!"
    },
    {
        "roll",{95,95,95, 95,95,95, 15,14,13, 95,95,95},
	{0,0,0, 0,0,0, 3,3,3, 0,0,0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_roll,              SLOT( 0),        0,      0,
        "",                     "!Roll!"
    },
 
    {
        "enhanced damage",{95,95,95, 30,25,15, 25,23,20, 7,4,1},
	{0,0,0, 6,6,6, 5,5,5, 3,3,3},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_enhanced_damage,   SLOT( 0),        0,      0,
        "",                     "!Enhanced Damage!"
    },

    {
	"hand to hand",{95,95,95, 10,10,10, 95,95,95, 6,6,1},
	{0,0,0, 3,3,3, 0,0,0, 4,4,4},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_hand_to_hand,	SLOT( 0),	0,	0,
	"",			"!Hand to Hand!"
    },

    {
        "kick",{ 95,95,95, 12,12,12, 95,95,95, 8,8,8 },
        { 0,0,0, 2,2,2, 0,0,0, 3,3,3},
        spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        &gsn_kick,              SLOT( 0),        0,      12,
        "kick",                 "!Kick!"
    },

    {
        "parry",{ 22,21,20, 5,5,5, 13,12,11, 1,1,1 },
	{ 9,9,9, 6,6,6, 4,4,4, 4,4,4},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_parry,             SLOT( 0),        0,      0,
        "",                     "!Parry!"
    },

    {
        "rescue",{ 95,95,95, 95,95,95, 95,95,95, 25,23,21 },
     	{ 0,0,0, 0,0,0, 0,0,0, 2,2,2},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_rescue,            SLOT( 0),        0,     12,
        "",                     "!Rescue!"
    },

    {
	"trip",{ 95,95,95, 95,95,95, 1,1,1, 95,95,95 },
	{ 0,0,0, 0,0,0, 4,4,4, 0,0,0},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_trip,		SLOT( 0),	0,	24,
	"trip",			"!Trip!"
    },

    {
	"jump",{ 95,95,95, 95,95,95, 20,17,15, 20,17,15 },
	{ 0,0,0, 0,0,0, 2,2,2, 3,3,3},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_jump,		SLOT( 0),	0,	0,
	"",			"!Jump!"
    },

    {
        "second attack",{ 95,30,28, 10,9,8, 12,11,10, 5,4,3 },
     	{ 0,8,8, 6,6,6, 4,4,4, 3,3,3},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_second_attack,     SLOT( 0),        0,      0,
        "",                     "!Second Attack!"
    },

    {
        "third attack",{ 95,95,95, 95,25,15, 24,23,22, 12,11,10 },
     	{0,0,0, 0,6,6, 5,5,5, 4,4,4},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_third_attack,      SLOT( 0),        0,      0,
        "",                     "!Third Attack!"
    },
    {
        "fourth attack",{ 95,95,95, 95,95,95, 95,95,95, 40,38,36 },
     	{0,0,0, 0,0,0, 0,0,0, 5,5,5},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_fourth_attack,      SLOT( 0),        0,      0,
        "",                     "!Fourth Attack!"
    },
    {
        "fifth attack",{ 95,95,95, 95,95,95, 95,95,95, 95,50,45 },
     	{0,0,0, 0,0,0, 0,0,0, 0,6,6},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_fifth_attack,      SLOT( 0),        0,      0,
        "",                     "!Fifth Attack!"
    },

    {
	"dual wield",{ 95,95,95, 95,95,95, 20,18,17, 10,9,8 },
       	{0,0,0, 0,0,0, 6,6,6, 4,4,4},
	spell_null,		TAR_IGNORE,             POS_FIGHTING,
	&gsn_dual_wield,	SLOT( 0),        0,     0,
	"",			"!Dual Wield!"
    },

/* non-combat skills */

    { 
	"fast healing",{ 25,20,15, 9,8,7, 16,15,14, 6,6,6 },
	{ 7,7,7, 5,5,5, 4,4,4, 4,4,4},
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_fast_healing,	SLOT( 0),	0,	0,
	"",			"!Fast Healing!"
    },

    {
	"haggle",{ 7,6,5, 18,17,16, 1,1,1, 14,13,12 },
	{ 2,2,2, 2,2,2, 1,1,1, 2,2,2},
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_haggle,		SLOT( 0),	0,	0,
	"",			"!Haggle!"
    },

    {
	"hide",{ 95,95,95, 95,95,95, 1,1,1, 95,95,95 },
	{ 0,0,0, 0,0,0, 2,2,2, 0,0,0},
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_hide,		SLOT( 0),	 0,	12,
	"",			"!Hide!"
    },

    {
	"meditation",{ 6,6,6, 6,6,6, 35,35,35, 35,35,35 },
	{ 5,5,5, 6,6,6, 6,6,6, 6,6,6},
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_meditation,	SLOT( 0),	0,	0,
	"",			"Meditation"
    },

    {
	"peek",	{ 95,95,95, 95,95,95, 1,1,1, 95,95,95 },
	{ 0,0,0, 0,0,0, 3,3,3, 0,0,0},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_peek,		SLOT( 0),	 0,	 0,
	"",			"!Peek!"
    },

    {
	"pick lock",{ 95,95,95, 95,95,95, 7,6,5, 95,95,95 },
	{ 0,0,0, 0,0,0, 2,2,2, 0,0,0},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_pick_lock,		SLOT( 0),	 0,	12,
	"",			"!Pick!"
    },

    {
	"smash",{ 95,95,95, 95,95,95, 95,95,95, 15,14,13 },
	{ 0,0,0, 0,0,0, 0,0,0, 2,2,2},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_smash,		SLOT( 0),	 0,	12,
	"",			"!Smash!"
    },

    {
	"sneak",{ 95,95,95, 95,95,95, 14,13,12, 20,18,16 },
	{ 0,0,0, 0,0,0, 4,4,4, 4,4,4},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_sneak,		SLOT( 0),	 0,	12,
	"",			"You no longer feel stealthy."
    },
    {
	"sharpen",{ 95,95,95, 95,95,95, 95,40,37, 95,30,27 },
	{ 0,0,0, 0,0,0, 0,4,4, 0,3,3},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_sharpen,		SLOT( 0),	 0,	12,
	"",			"!Sharpen!"
    },

    {
	"steal",{ 95,95,95, 95,95,95, 5,5,5, 95,95,95 },
	{ 0,0,0, 0,0,0, 4,4,4, 0,0,0},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_steal,		SLOT( 0),	 0,	24,
	"",			"!Steal!"
    },

    {
	"disguise",{ 95,95,95, 95,95,95, 95,95,20, 95,95,95 },
	{ 0,0,0, 0,0,0, 0,0,4, 0,0,0},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_disguise,		SLOT( 0),	 0,	24,
	"",			"You are unable to keep yourself disguised any longer."
    },

    {
	"scrolls",{ 1,1,1, 1,1,1, 1,1,1, 1,1,1 },
	{ 2,2,2, 3,3,3, 4,4,4, 8,8,8},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_scrolls,		SLOT( 0),	0,	24,
	"",			"!Scrolls!"
    },

    {
	"staves",{ 1,1,1, 1,1,1, 1,1,1, 1,1,1 },
	{ 2,2,2, 3,3,3, 5,5,5, 8,8,8},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_staves,		SLOT( 0),	0,	12,
	"",			"!Staves!"
    },

    {
	"wands",{ 1,1,1, 1,1,1, 1,1,1, 1,1,1 },
	{ 2,2,2, 3,3,3, 5,5,5, 8,8,8},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_wands,		SLOT( 0),	0,	12,
	"",			"!Wands!"
    },

    {
	"recall",{ 1,1,1, 1,1,1, 1,1,1, 1,1,1 },
	{ 2,2,2, 2,2,2, 2,2,2, 2,2,2},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_recall,		SLOT( 0),	0,	12,
	"",			"!Recall!"
    }
};

const   struct  group_type      group_table     [MAX_GROUP]     =
{

    {
	"rom basics",		{ 0,0,0, 0,0,0, 0,0,0, 0,0,0 },
	{ "scrolls", "staves", "wands", "recall", "home"}
    },

    {
	"magician basics",	{ 0,0,0, -1,-1,-1, -1,-1,-1, -1,-1,-1 },
	{ "dagger" }
    },

    {
	"acolyte basics",	{ -1,-1,-1, 0,0,0, -1,-1,-1, -1,-1,-1 },
	{ "mace" }
    },
   
    {
	"pickpocket basics",		{ -1,-1,-1, -1,-1,-1, 0,0,0, -1,-1,-1 },
	{ "dagger", "steal", "peek" }
    },

    {
	"swordpupil basics",	{ -1,-1,-1, -1,-1,-1, -1,-1,-1, 0,0,0 },
	{ "sword", "second attack" }
    },

    {
	"attack",{-1,-1,-1, 8,8,8, -1,-1,-1, -1,-1,-1},
	{ "demonfire", "dispel evil", "earthquake", "flamestrike",
	"wrath of god" }
    },

    {
	"beguiling",		{ 7,7,7, -1,-1,-1, 4,4,4, -1,-1,-1},
	{ "charm person", "sleep", "quiet", "charisma" }
    },

    {
	"benedictions",		{-1,-1,-1, 6,6,6, -1,-1,-1, -1,-1,-1},
	{ "bless", "calm", "frenzy", "holy word", "remove curse", "quiet" }
    },

    {
	"combat",{ 9,9,9, -1,-1,-1, -1,-1,-1, -1,-1,-1},
	{ "acid blast", "burning hands", "chain lightning", "chill touch",
	  "colour spray", "fireball", "lightning bolt", "magic missile",
	  "shocking grasp", "nuclear blast" }
    },

    {
	"creation",{4,4,4, 3,3,3, -1,-1,-1, -1,-1,-1},
	{ "continual light", "create food", "create spring", "create water",
           "make bag", "quench", "sate", "eternal light", "preserve" }
    },

    {
	"curative",{ -1,-1,-1, 3,3,3, -1,-1,-1, -1,-1,-1 },
	{ "cure blindness", "cure disease", "cure poison" }
    }, 

    {
	"detection",{6,6,6, 3,3,3, 4,4,4, -1,-1,-1},
 	{ "detect evil", "detect hidden", "detect invis", 
	  "detect poison", "identify", "know alignment", "locate object",
	  "locate person", "detect traps" }

    },

    /*Disable the draconian group. Only draconians can use these*/
    {
	"draconian",{ -1,-1,-1, -1,-1,-1, -1,-1,-1, -1,-1,-1 },
	{ "acid breath", "fire breath", "frost breath", "gas breath",
	  "lightning breath"  }
    },

    {
	"enchantment",		{ 6,6,6, -1,-1,-1, -1,-1,-1, -1,-1,-1 },
	{ "enchant armor", "enchant weapon", "remove invis", 
	  "invis object", "curse object" }
    },

    {
	"modification",		{ 4,4,4, -1,-1,-1, -1,-1,-1, -1,-1,-1 },
	{ "shrink", "rot object", "remove rot", "fireproof",
	  "remove alignment"}
    },

    { 
	"enhancement",		{ 5,5,5, -1,-1,-1, -1,-1,-1, -1,-1,-1 },
	{ "giant strength", "haste", "infravision", "refresh" }
    },

    {
	"harmful",		{ -1,-1,-1, 3,3,3, -1,-1,-1, -1,-1,-1 },
	{ "cause critical", "cause light", "cause serious", "harm" }
    },

    {   
	"healing",		{ -1,-1,-1, 3,3,3, -1,-1,-1, -1,-1,-1 },
 	{ "cure critical", "cure light", "cure serious", "heal", 
	  "mass healing", "refresh", "divine healing" }
    },

    {
	"illusion",		{ 6,6,6, -1,-1,-1, 3,3,3, -1,-1,-1 },
	{ "invis", "mass invis", "ventriloquate", "flamewall", "web" }
    },
  
    {
	"maladictions",		{ 6,6,6, 6,6,6, -1,-1,-1, -1,-1,-1 },
	{ "blindness", "change sex", "curse", "energy drain", "plague", 
	  "poison", "weaken", "fear", "heat metal" }
    },

    { 
	"protective",		{ 6,6,6, 6,6,6, 7,7,7, 9,9,9 },
	{ "armor", "cancellation", "dispel magic", "protection evil",
 	"protection good", "sanctuary", "shield", "stone skin", "blink",
	"flame shield" }
    },

    {
	"transportation",	{ 6,6,6, 5,5,5, 8,8,8, 9,9,9 },
	{ "fly", "gate", "pass door", "summon", "teleport", 
	"word of recall", "portal", "nexus", "light feet" }
    },
   
    {
	"weather",{ 4,4,4, 3,3,3 -1,-1,-1, -1,-1,-1 },
	{ "call lightning", "control weather", "faerie fire", "faerie fog",
	  "lightning bolt" }
    }
	
   

};
